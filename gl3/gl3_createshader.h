#pragma once
#include <GL/glew.h>

inline GLuint CompileShader(GLenum stage, const std::string &src)
{
    auto shader = glCreateShader(stage);
    auto data = static_cast<const GLchar *>(src.data());
    auto size = static_cast<GLint>(src.size());
    glShaderSource(shader, 1, &data, &size);

    glCompileShader(shader);
    auto compileStatus = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        // fprintf(stderr, "Error compiling '%s':\n\n", _path);
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char *log = new GLchar[len];
        glGetShaderInfoLog(shader, len, 0, log);
        fprintf(stderr, log);
        delete[] log;

        //fprintf(stderr, "\n\n%s", src.data());
        fprintf(stderr, "\n");
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

inline bool LinkShaderProgram(GLuint _handle)
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

inline unsigned int GL3_CreateShader(const std::string &vsSrc, const std::string &fsSrc)
{
    auto vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
    if (!vs)
    {
        return 0;
    }
    auto fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!fs)
    {
        return 0;
    }

    auto shTeapot = glCreateProgram();
    glAttachShader(shTeapot, vs);
    glAttachShader(shTeapot, fs);
    bool ret = LinkShaderProgram(shTeapot);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ret)
    {
        return 0;
    }
    return shTeapot;
}
