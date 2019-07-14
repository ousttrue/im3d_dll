#include "glutil.h"
#include "im3d.h"
#include <gl/glew.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static const char *StripPath(const char *_path)
{
    int i = 0, last = 0;
    while (_path[i] != '\0')
    {
        if (_path[i] == '\\' || _path[i] == '/')
        {
            last = i + 1;
        }
        ++i;
    }
    return &_path[last];
}

static void Append(const char *_str, Im3d::Vector<char> &_out_)
{
    while (*_str)
    {
        _out_.push_back(*_str);
        ++_str;
    }
}

static void AppendLine(const char *_str, Im3d::Vector<char> &_out_)
{
    Append(_str, _out_);
    _out_.push_back('\n');
}

const char *GetGlEnumString(GLenum _enum)
{
#define CASE_ENUM(e) \
    case e:          \
        return #e
    switch (_enum)
    {
        // errors
        CASE_ENUM(GL_NONE);
        CASE_ENUM(GL_INVALID_ENUM);
        CASE_ENUM(GL_INVALID_VALUE);
        CASE_ENUM(GL_INVALID_OPERATION);
        CASE_ENUM(GL_INVALID_FRAMEBUFFER_OPERATION);
        CASE_ENUM(GL_OUT_OF_MEMORY);

    default:
        return "Unknown GLenum";
    };
#undef CASE_ENUM
}

bool LoadShader(const char *_path, const char *_defines, Im3d::Vector<char> &_out_)
{
    fprintf(stdout, "Loading shader: '%s'", StripPath(_path));
    if (_defines)
    {
        fprintf(stdout, " ");
        while (*_defines != '\0')
        {
            fprintf(stdout, " %s,", _defines);
            Append("#define ", _out_);
            AppendLine(_defines, _out_);
            _defines = strchr(_defines, 0);
            IM3D_ASSERT(_defines);
            ++_defines;
        }
    }
    fprintf(stdout, "\n");

    FILE *fin = fopen(_path, "rb");
    if (!fin)
    {
        fprintf(stderr, "Error opening '%s'\n", _path);
        return false;
    }
    fseek(fin, 0, SEEK_END); // not portable but should work almost everywhere
    long fsize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    int srcbeg = _out_.size();
    _out_.resize(srcbeg + fsize, '\0');
    if (fread(_out_.data() + srcbeg, 1, fsize, fin) != fsize)
    {
        fclose(fin);
        fprintf(stderr, "Error reading '%s'\n", _path);
        return false;
    }
    fclose(fin);
    _out_.push_back('\0');

    return true;
}

GLuint LoadCompileShader(GLenum _stage, const char *_path, const char *_defines)
{
    Im3d::Vector<char> src;
    AppendLine("#version 140", src);
    if (!LoadShader(_path, _defines, src))
    {
        return 0;
    }

    GLuint ret = 0;
    ret = glCreateShader(_stage);
    const GLchar *pd = src.data();
    GLint ps = src.size();
    glShaderSource(ret, 1, &pd, &ps);

    glCompileShader(ret);
    GLint compileStatus = GL_FALSE;
    glGetShaderiv(ret, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        fprintf(stderr, "Error compiling '%s':\n\n", _path);
        GLint len;
        glGetShaderiv(ret, GL_INFO_LOG_LENGTH, &len);
        char *log = new GLchar[len];
        glGetShaderInfoLog(ret, len, 0, log);
        fprintf(stderr, log);
        delete[] log;

        //fprintf(stderr, "\n\n%s", src.data());
        fprintf(stderr, "\n");
        glDeleteShader(ret);
        return 0;
    }
    return ret;
}

bool LinkShaderProgram(GLuint _handle)
{
    glLinkProgram(_handle);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(_handle, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        fprintf(stderr, "Error linking program:\n\n");
        GLint len;
        glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len);
        GLchar *log = new GLchar[len];
        glGetProgramInfoLog(_handle, len, 0, log);
        fprintf(stderr, log);
        fprintf(stderr, "\n");
        delete[] log;

        return false;
    }
    return true;
}

void DrawNdcQuad()
{
    static GLuint vbQuad;
    static GLuint vaQuad;
    if (vbQuad == 0)
    {
        float quadv[8] = {
            -1.0f,
            -1.0f,
            1.0f,
            -1.0f,
            -1.0f,
            1.0f,
            1.0f,
            1.0f,
        };
        glGenBuffers(1, &vbQuad);
        glGenVertexArrays(1, &vaQuad);
        glBindVertexArray(vaQuad);
        glBindBuffer(GL_ARRAY_BUFFER, vbQuad);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec2), (GLvoid *)0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadv), (GLvoid *)quadv, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
    glBindVertexArray(vaQuad);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void GLClearState(int w, int h)
{
    Im3d::Vec4 kClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glBindVertexArray(0);
    glUseProgram(0);
    glViewport(0, 0, w, h);
    glClearColor(kClearColor.x, kClearColor.y, kClearColor.z, kClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);
}