#include "im3d_example.h"
#include "im3d_opengl31.h"
#include "teapot.h"
#include "win32_window.h"
#include "glcontext.h"
#include "im3d_context.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Im3d;

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

/******************************************************************************/
// force Nvidia/AMD drivers to use the discrete GPU
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

static LARGE_INTEGER g_SysTimerFreq;

const char *Im3d::GetPlatformErrorString(DWORD _err)
{
    const int kErrMsgMax = 1024;
    static char buf[kErrMsgMax];
    buf[0] = '\0';
    IM3D_VERIFY(
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            _err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)buf,
            kErrMsgMax,
            nullptr) != 0);
    return buf;
}

// #include "GL/wglew.h"
// static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat = 0;
// static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = 0;

/******************************************************************************/
static void Append(const char *_str, Vector<char> &_out_)
{
    while (*_str)
    {
        _out_.push_back(*_str);
        ++_str;
    }
}
static void AppendLine(const char *_str, Vector<char> &_out_)
{
    Append(_str, _out_);
    _out_.push_back('\n');
}
static bool LoadShader(const char *_path, const char *_defines, Vector<char> &_out_)
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
    IM3D_VERIFY(fseek(fin, 0, SEEK_END) == 0); // not portable but should work almost everywhere
    long fsize = ftell(fin);
    IM3D_VERIFY(fseek(fin, 0, SEEK_SET) == 0);

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

#if defined(IM3D_OPENGL)
GLuint Im3d::LoadCompileShader(GLenum _stage, const char *_path, const char *_defines)
{
    Vector<char> src;
    AppendLine("#version " IM3D_STRINGIFY(IM3D_OPENGL_VSHADER), src);
    if (!LoadShader(_path, _defines, src))
    {
        return 0;
    }

    GLuint ret = 0;
    glAssert(ret = glCreateShader(_stage));
    const GLchar *pd = src.data();
    GLint ps = src.size();
    glAssert(glShaderSource(ret, 1, &pd, &ps));

    glAssert(glCompileShader(ret));
    GLint compileStatus = GL_FALSE;
    glAssert(glGetShaderiv(ret, GL_COMPILE_STATUS, &compileStatus));
    if (compileStatus == GL_FALSE)
    {
        fprintf(stderr, "Error compiling '%s':\n\n", _path);
        GLint len;
        glAssert(glGetShaderiv(ret, GL_INFO_LOG_LENGTH, &len));
        char *log = new GLchar[len];
        glAssert(glGetShaderInfoLog(ret, len, 0, log));
        fprintf(stderr, log);
        delete[] log;

        //fprintf(stderr, "\n\n%s", src.data());
        fprintf(stderr, "\n");
        glAssert(glDeleteShader(ret));
        return 0;
    }
    return ret;
}

bool Im3d::LinkShaderProgram(GLuint _handle)
{
    IM3D_ASSERT(_handle != 0);

    glAssert(glLinkProgram(_handle));
    GLint linkStatus = GL_FALSE;
    glAssert(glGetProgramiv(_handle, GL_LINK_STATUS, &linkStatus));
    if (linkStatus == GL_FALSE)
    {
        fprintf(stderr, "Error linking program:\n\n");
        GLint len;
        glAssert(glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len));
        GLchar *log = new GLchar[len];
        glAssert(glGetProgramInfoLog(_handle, len, 0, log));
        fprintf(stderr, log);
        fprintf(stderr, "\n");
        delete[] log;

        return false;
    }
    return true;
}

void Im3d::DrawNdcQuad()
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
        glAssert(glGenBuffers(1, &vbQuad));
        glAssert(glGenVertexArrays(1, &vaQuad));
        glAssert(glBindVertexArray(vaQuad));
        glAssert(glBindBuffer(GL_ARRAY_BUFFER, vbQuad));
        glAssert(glEnableVertexAttribArray(0));
        glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *)0));
        glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(quadv), (GLvoid *)quadv, GL_STATIC_DRAW));
        glAssert(glBindVertexArray(0));
    }
    glAssert(glBindVertexArray(vaQuad));
    glAssert(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    glAssert(glBindVertexArray(0));
}

void Im3d::DrawTeapot(const Mat4 &_world, const Mat4 &_viewProj)
{
    static GLuint shTeapot;
    static GLuint vbTeapot;
    static GLuint ibTeapot;
    static GLuint vaTeapot;
    if (shTeapot == 0)
    {
        GLuint vs = LoadCompileShader(GL_VERTEX_SHADER, "model.glsl", "VERTEX_SHADER\0");
        GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "model.glsl", "FRAGMENT_SHADER\0");
        if (vs && fs)
        {
            glAssert(shTeapot = glCreateProgram());
            glAssert(glAttachShader(shTeapot, vs));
            glAssert(glAttachShader(shTeapot, fs));
            bool ret = LinkShaderProgram(shTeapot);
            glAssert(glDeleteShader(vs));
            glAssert(glDeleteShader(fs));
            if (!ret)
            {
                return;
            }
        }
        else
        {
            return;
        }
        glAssert(glGenBuffers(1, &vbTeapot));
        glAssert(glGenBuffers(1, &ibTeapot));
        glAssert(glGenVertexArrays(1, &vaTeapot));
        glAssert(glBindVertexArray(vaTeapot));
        glAssert(glBindBuffer(GL_ARRAY_BUFFER, vbTeapot));
        glAssert(glEnableVertexAttribArray(0));
        glAssert(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3) * 2, (GLvoid *)0));
        glAssert(glEnableVertexAttribArray(1));
        glAssert(glEnableVertexAttribArray(0));
        glAssert(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)0));
        glAssert(glEnableVertexAttribArray(1));
        glAssert(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)(sizeof(float) * 3)));
        glAssert(glBufferData(GL_ARRAY_BUFFER, sizeof(s_teapotVertices), (GLvoid *)s_teapotVertices, GL_STATIC_DRAW));
        glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibTeapot));
        glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_teapotIndices), (GLvoid *)s_teapotIndices, GL_STATIC_DRAW));
        glAssert(glBindVertexArray(0));
    }
    glAssert(glUseProgram(shTeapot));
    glAssert(glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uWorldMatrix"), 1, false, _world.m));
    glAssert(glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, _viewProj.m));
    glAssert(glBindVertexArray(vaTeapot));
    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glEnable(GL_CULL_FACE));
    glAssert(glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid *)0));
    glAssert(glDisable(GL_DEPTH_TEST));
    glAssert(glDisable(GL_CULL_FACE));
    glAssert(glBindVertexArray(0));
    glAssert(glUseProgram(0));
}

const char *Im3d::GetGlEnumString(GLenum _enum)
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

#endif // graphics

/******************************************************************************/
void Im3d::Assert(const char *_e, const char *_file, int _line, const char *_msg, ...)
{
    const int kAssertMsgMax = 1024;

    char buf[kAssertMsgMax];
    if (_msg != nullptr)
    {
        va_list args;
        va_start(args, _msg);
        vsnprintf(buf, kAssertMsgMax, _msg, args);
        va_end(args);
    }
    else
    {
        buf[0] = '\0';
    }
    fprintf(stderr, "Assert (%s, line %d)\n\t'%s' %s", StripPath(_file), _line, _e ? _e : "", buf);
}

void Im3d::RandSeed(int _seed)
{
    srand(_seed);
}
int Im3d::RandInt(int _min, int _max)
{
    return _min + (int)rand() % (_max - _min);
}
float Im3d::RandFloat(float _min, float _max)
{
    return _min + (float)rand() / (float)RAND_MAX * (_max - _min);
}
Mat3 Im3d::RandRotation()
{
    return Rotation(Normalize(RandVec3(-1.0f, 1.0f)), RandFloat(-Pi, Pi));
}
Vec3 Im3d::RandVec3(float _min, float _max)
{
    return Im3d::Vec3(
        RandFloat(_min, _max),
        RandFloat(_min, _max),
        RandFloat(_min, _max));
}
Color Im3d::RandColor(float _min, float _max)
{
    Vec3 v = RandVec3(_min, _max);
    return Color(v.x, v.y, v.z);
}

/******************************************************************************/
#if defined(IM3D_OPENGL)
static GLuint g_ImGuiVertexArray;
static GLuint g_ImGuiVertexBuffer;
static GLuint g_ImGuiIndexBuffer;
static GLuint g_ImGuiShader;
static GLuint g_ImGuiFontTexture;

static void ImGui_Draw(ImDrawData *_drawData)
{
    ImGuiIO &io = ImGui::GetIO();

    int fbX, fbY;
    fbX = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    fbY = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fbX == 0 || fbY == 0)
    {
        return;
    }
    _drawData->ScaleClipRects(io.DisplayFramebufferScale);

    glAssert(glViewport(0, 0, (GLsizei)fbX, (GLsizei)fbY));
    glAssert(glEnable(GL_BLEND));
    glAssert(glBlendEquation(GL_FUNC_ADD));
    glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glAssert(glDisable(GL_CULL_FACE));
    glAssert(glDisable(GL_DEPTH_TEST));
    glAssert(glEnable(GL_SCISSOR_TEST));
    glAssert(glActiveTexture(GL_TEXTURE0));

    Mat4 ortho = Mat4(
        2.0f / io.DisplaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f);
    glAssert(glUseProgram(g_ImGuiShader));

    bool transpose = false;
#ifdef IM3D_MATRIX_ROW_MAJOR
    transpose = true;
#endif
    glAssert(glUniformMatrix4fv(glGetUniformLocation(g_ImGuiShader, "uProjMatrix"), 1, transpose, (const GLfloat *)ortho));
    glAssert(glBindVertexArray(g_ImGuiVertexArray));

    for (int i = 0; i < _drawData->CmdListsCount; ++i)
    {
        const ImDrawList *drawList = _drawData->CmdLists[i];
        const ImDrawIdx *indexOffset = 0;

        glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_ImGuiVertexBuffer));
        glAssert(glBufferData(GL_ARRAY_BUFFER, drawList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid *)&drawList->VtxBuffer.front(), GL_STREAM_DRAW));
        glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ImGuiIndexBuffer));
        glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawList->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid *)drawList->IdxBuffer.Data, GL_STREAM_DRAW));

        for (const ImDrawCmd *pcmd = drawList->CmdBuffer.begin(); pcmd != drawList->CmdBuffer.end(); ++pcmd)
        {
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(drawList, pcmd);
            }
            else
            {
                glAssert(glBindTexture(GL_TEXTURE_2D, (GLuint)pcmd->TextureId));
                glAssert(glScissor((int)pcmd->ClipRect.x, (int)(fbY - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
                glAssert(glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, (GLvoid *)indexOffset));
            }
            indexOffset += pcmd->ElemCount;
        }
    }

    glAssert(glDisable(GL_SCISSOR_TEST));
    glAssert(glDisable(GL_BLEND));
    glAssert(glUseProgram(0));
}

static bool ImGui_Init()
{
    GLuint vs = LoadCompileShader(GL_VERTEX_SHADER, "imgui.glsl", "VERTEX_SHADER\0");
    GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "imgui.glsl", "FRAGMENT_SHADER\0");
    if (vs && fs)
    {
        glAssert(g_ImGuiShader = glCreateProgram());
        glAssert(glAttachShader(g_ImGuiShader, vs));
        glAssert(glAttachShader(g_ImGuiShader, fs));
        bool ret = LinkShaderProgram(g_ImGuiShader);
        glAssert(glDeleteShader(vs));
        glAssert(glDeleteShader(fs));
        if (!ret)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    glAssert(glUseProgram(g_ImGuiShader));
    glAssert(glUniform1i(glGetUniformLocation(g_ImGuiShader, "txTexture"), 0));
    glAssert(glUseProgram(0));

    glAssert(glGenBuffers(1, &g_ImGuiVertexBuffer));
    glAssert(glGenBuffers(1, &g_ImGuiIndexBuffer));
    glAssert(glGenVertexArrays(1, &g_ImGuiVertexArray));
    glAssert(glBindVertexArray(g_ImGuiVertexArray));
    glAssert(glBindBuffer(GL_ARRAY_BUFFER, g_ImGuiVertexBuffer));
    glAssert(glEnableVertexAttribArray(0));
    glAssert(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, pos)));
    glAssert(glEnableVertexAttribArray(1));
    glAssert(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, uv)));
    glAssert(glEnableVertexAttribArray(2));
    glAssert(glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, col)));
    glAssert(glBindVertexArray(0));

    unsigned char *txbuf;
    int txX, txY;
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
    glAssert(glGenTextures(1, &g_ImGuiFontTexture));
    glAssert(glBindTexture(GL_TEXTURE_2D, g_ImGuiFontTexture));
    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glAssert(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glAssert(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, txX, txY, 0, GL_RED, GL_UNSIGNED_BYTE, (const GLvoid *)txbuf));
    io.Fonts->TexID = (void *)g_ImGuiFontTexture;

    io.RenderDrawListsFn = &ImGui_Draw;

    ImGui::StyleColorsDark();

    return true;
}

static void ImGui_Shutdown()
{
    glAssert(glDeleteVertexArrays(1, &g_ImGuiVertexArray));
    glAssert(glDeleteBuffers(1, &g_ImGuiVertexBuffer));
    glAssert(glDeleteBuffers(1, &g_ImGuiIndexBuffer));
    glAssert(glDeleteProgram(g_ImGuiShader));
    glAssert(glDeleteTextures(1, &g_ImGuiFontTexture));
}

#elif defined(IM3D_DX11)
static ID3D11InputLayout *g_ImGuiInputLayout;
static ID3DBlob *g_ImGuiVertexShaderBlob;
static ID3D11VertexShader *g_ImGuiVertexShader;
static ID3DBlob *g_ImGuiPixelShaderBlob;
static ID3D11PixelShader *g_ImGuiPixelShader;
static ID3D11RasterizerState *g_ImGuiRasterizerState;
static ID3D11BlendState *g_ImGuiBlendState;
static ID3D11DepthStencilState *g_ImGuiDepthStencilState;
static ID3D11ShaderResourceView *g_ImGuiFontResourceView;
static ID3D11SamplerState *g_ImGuiFontSampler;
static ID3D11Buffer *g_ImGuiConstantBuffer;
static ID3D11Buffer *g_ImGuiVertexBuffer;
static ID3D11Buffer *g_ImGuiIndexBuffer;

static void ImGui_Draw(ImDrawData *_drawData)
{
    ImGuiIO &io = ImGui::GetIO();
    ID3D11Device *d3d = g_Example->m_d3dDevice;
    ID3D11DeviceContext *ctx = g_Example->m_d3dDeviceCtx;

    // (re)alloc vertex/index buffers
    static int s_vertexBufferSize = 0;
    if (!g_ImGuiVertexBuffer || s_vertexBufferSize < _drawData->TotalVtxCount)
    {
        if (g_ImGuiVertexBuffer)
        {
            g_ImGuiVertexBuffer->Release();
            g_ImGuiVertexBuffer = nullptr;
        }
        s_vertexBufferSize = _drawData->TotalVtxCount;
        g_ImGuiVertexBuffer = CreateVertexBuffer(s_vertexBufferSize * sizeof(ImDrawVert), D3D11_USAGE_DYNAMIC);
    }
    static int s_indexBufferSize = 0;
    if (!g_ImGuiIndexBuffer || s_indexBufferSize < _drawData->TotalIdxCount)
    {
        if (g_ImGuiIndexBuffer)
        {
            g_ImGuiIndexBuffer->Release();
            g_ImGuiIndexBuffer = nullptr;
        }
        s_indexBufferSize = _drawData->TotalIdxCount;
        g_ImGuiIndexBuffer = CreateIndexBuffer(s_indexBufferSize * sizeof(ImDrawIdx), D3D11_USAGE_DYNAMIC);
    }

    // copy and convert all vertices into a single contiguous buffer
    ImDrawVert *vtxDst = (ImDrawVert *)MapBuffer(g_ImGuiVertexBuffer, D3D11_MAP_WRITE_DISCARD);
    ImDrawIdx *idxDst = (ImDrawIdx *)MapBuffer(g_ImGuiIndexBuffer, D3D11_MAP_WRITE_DISCARD);
    for (int i = 0; i < _drawData->CmdListsCount; ++i)
    {
        const ImDrawList *cmdList = _drawData->CmdLists[i];
        memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += cmdList->VtxBuffer.Size;
        idxDst += cmdList->IdxBuffer.Size;
    }
    UnmapBuffer(g_ImGuiVertexBuffer);
    UnmapBuffer(g_ImGuiIndexBuffer);

    // update constant buffer
    *(Mat4 *)MapBuffer(g_ImGuiConstantBuffer, D3D11_MAP_WRITE_DISCARD) = Mat4(
        2.0f / io.DisplaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f);
    UnmapBuffer(g_ImGuiConstantBuffer);

    // set state
    D3D11_VIEWPORT viewport = {};
    viewport.Width = ImGui::GetIO().DisplaySize.x;
    viewport.Height = ImGui::GetIO().DisplaySize.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = viewport.TopLeftY = 0.0f;
    ctx->RSSetViewports(1, &viewport);

    unsigned int stride = sizeof(ImDrawVert);
    unsigned int offset = 0;
    ctx->IASetInputLayout(g_ImGuiInputLayout);
    ctx->IASetVertexBuffers(0, 1, &g_ImGuiVertexBuffer, &stride, &offset);
    ctx->IASetIndexBuffer(g_ImGuiIndexBuffer, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(g_ImGuiVertexShader, nullptr, 0);
    ctx->VSSetConstantBuffers(0, 1, &g_ImGuiConstantBuffer);
    ctx->PSSetShader(g_ImGuiPixelShader, nullptr, 0);
    ctx->PSSetSamplers(0, 1, &g_ImGuiFontSampler);

    ctx->OMSetBlendState(g_ImGuiBlendState, nullptr, 0xffffffff);
    ctx->OMSetDepthStencilState(g_ImGuiDepthStencilState, 0);
    ctx->RSSetState(g_ImGuiRasterizerState);

    int vtxOffset = 0;
    int idxOffset = 0;
    for (int i = 0; i < _drawData->CmdListsCount; ++i)
    {
        const ImDrawList *cmdList = _drawData->CmdLists[i];
        for (const ImDrawCmd *pcmd = cmdList->CmdBuffer.begin(); pcmd != cmdList->CmdBuffer.end(); ++pcmd)
        {
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmdList, pcmd);
            }
            else
            {
                const D3D11_RECT r = {(LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w};
                ctx->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView **)&pcmd->TextureId);
                ctx->RSSetScissorRects(1, &r);
                ctx->DrawIndexed(pcmd->ElemCount, idxOffset, vtxOffset);
            }
            idxOffset += pcmd->ElemCount;
        }
        vtxOffset += cmdList->VtxBuffer.Size;
    }
}

static bool ImGui_Init()
{
    ImGuiIO &io = ImGui::GetIO();
    ID3D11Device *d3d = g_Example->m_d3dDevice;

    {
        g_ImGuiVertexShaderBlob = LoadCompileShader("vs_4_0", "imgui.hlsl", "VERTEX_SHADER\0");
        if (!g_ImGuiVertexShaderBlob)
        {
            return false;
        }
        dxAssert(d3d->CreateVertexShader((DWORD *)g_ImGuiVertexShaderBlob->GetBufferPointer(), g_ImGuiVertexShaderBlob->GetBufferSize(), nullptr, &g_ImGuiVertexShader));
    }
    {
        g_ImGuiPixelShaderBlob = LoadCompileShader("ps_4_0", "imgui.hlsl", "PIXEL_SHADER\0");
        if (!g_ImGuiPixelShaderBlob)
        {
            return false;
        }
        dxAssert(d3d->CreatePixelShader((DWORD *)g_ImGuiPixelShaderBlob->GetBufferPointer(), g_ImGuiPixelShaderBlob->GetBufferSize(), nullptr, &g_ImGuiPixelShader));
    }
    {
        D3D11_INPUT_ELEMENT_DESC desc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(ImDrawVert, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(ImDrawVert, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(ImDrawVert, col), D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        dxAssert(d3d->CreateInputLayout(desc, 3, g_ImGuiVertexShaderBlob->GetBufferPointer(), g_ImGuiVertexShaderBlob->GetBufferSize(), &g_ImGuiInputLayout));
    }
    g_ImGuiConstantBuffer = CreateConstantBuffer(sizeof(Mat4), D3D11_USAGE_DYNAMIC);

    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = true;
        desc.DepthClipEnable = true;
        dxAssert(d3d->CreateRasterizerState(&desc, &g_ImGuiRasterizerState));
    }
    {
        D3D11_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.StencilEnable = false;
        dxAssert(d3d->CreateDepthStencilState(&desc, &g_ImGuiDepthStencilState));
    }
    {
        D3D11_BLEND_DESC desc = {};
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        dxAssert(d3d->CreateBlendState(&desc, &g_ImGuiBlendState));
    }

    unsigned char *txbuf;
    int txX, txY;
    io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
    CreateTexture2D(txX, txY, DXGI_FORMAT_R8_UNORM, &g_ImGuiFontResourceView, txbuf)->Release();

    io.Fonts->TexID = (void *)g_ImGuiFontResourceView;
    io.RenderDrawListsFn = &ImGui_Draw;

    return true;
}

static void ImGui_Shutdown()
{
    if (g_ImGuiConstantBuffer)
        g_ImGuiConstantBuffer->Release();
    if (g_ImGuiVertexBuffer)
        g_ImGuiVertexBuffer->Release();
    if (g_ImGuiIndexBuffer)
        g_ImGuiIndexBuffer->Release();
    if (g_ImGuiFontResourceView)
        g_ImGuiFontResourceView->Release();
    if (g_ImGuiBlendState)
        g_ImGuiBlendState->Release();
    if (g_ImGuiDepthStencilState)
        g_ImGuiDepthStencilState->Release();
    if (g_ImGuiRasterizerState)
        g_ImGuiRasterizerState->Release();
    if (g_ImGuiInputLayout)
        g_ImGuiInputLayout->Release();
    if (g_ImGuiVertexShader)
        g_ImGuiVertexShader->Release();
    if (g_ImGuiVertexShaderBlob)
        g_ImGuiVertexShaderBlob->Release();
    if (g_ImGuiPixelShader)
        g_ImGuiPixelShader->Release();
    if (g_ImGuiPixelShaderBlob)
        g_ImGuiPixelShaderBlob->Release();
}

#endif

#if defined(IM3D_PLATFORM_WIN)
static void ImGui_Update(HWND hwnd, int width, int height)
{
    ImGuiIO &io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 0x41;
    io.KeyMap[ImGuiKey_C] = 0x43;
    io.KeyMap[ImGuiKey_V] = 0x56;
    io.KeyMap[ImGuiKey_X] = 0x58;
    io.KeyMap[ImGuiKey_Y] = 0x59;
    io.KeyMap[ImGuiKey_Z] = 0x5A;

    io.ImeWindowHandle = hwnd;
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.DeltaTime = g_Example->m_deltaTime;

    ImGui::NewFrame();
}
#endif

/******************************************************************************/
Example *Im3d::g_Example = nullptr;

Example::Example()
    : m_window(new Win32Window), m_glcontext(new GLContext)
{
}

Example::~Example()
{
    ImGui_Shutdown();
    Im3d_Shutdown();

    ImGui::EndFrame(); // prevent assert due to locked font atlas in DestroyContext() call below
    ImGui::DestroyContext();

    delete m_glcontext;
    delete m_window;
}

bool Example::init(int _width, int _height, const char *_title)
{
    g_Example = this;

    //  // force the current working directory to the exe location
    // 	TCHAR buf[MAX_PATH] = {};
    // 	DWORD buflen;
    // 	winAssert(buflen = GetModuleFileName(0, buf, MAX_PATH));
    // 	char* pathend = strrchr(buf, (int)'\\');
    // 	*(++pathend) = '\0';
    // 	winAssert(SetCurrentDirectory(buf));
    // 	fprintf(stdout, "Set current directory: '%s'\n", buf);

    winAssert(QueryPerformanceFrequency(&g_SysTimerFreq));
    winAssert(QueryPerformanceCounter(&m_currTime));

    ImGui::SetCurrentContext(ImGui::CreateContext()); // can't call this in ImGui_Init() because creating the window ends up calling ImGui::GetIO()

    auto hwnd = m_window->Create(_width, _height, _title);
    if (!hwnd)
    {
        return false;
    }

    if (!m_glcontext->Initialize(hwnd, IM3D_OPENGL_VMAJ, IM3D_OPENGL_VMIN))
    {
        return false;
    }

    if (!ImGui_Init())
    {
        return false;
    }
    if (!Im3d_Init())
    {
        return false;
    }

    m_camOrtho = false;
    m_camPos = Vec3(0.0f, 2.0f, 3.0f);
    m_camDir = Normalize(Vec3(0.0f, -0.5f, -1.0f));
    m_camFovDeg = 50.0f;

    return true;
}

bool Example::update()
{
    bool ret = true;
#if defined(IM3D_PLATFORM_WIN)
    g_Example->m_prevTime = g_Example->m_currTime;
    winAssert(QueryPerformanceCounter(&m_currTime));
    double microseconds = (double)((g_Example->m_currTime.QuadPart - g_Example->m_prevTime.QuadPart) * 1000000ll / g_SysTimerFreq.QuadPart);
    m_deltaTime = (float)(microseconds / 1000000.0);

    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ret = msg.message != WM_QUIT;
#endif

    int w, h;
    std::tie(w, h) = m_window->GetSize();
    ImGui_Update(m_window->GetHandle(), w, h);

    float kCamSpeed = 2.0f;
    float kCamSpeedMul = 10.0f;
    float kCamRotationMul = 10.0f;
    m_camWorld = LookAt(m_camPos, m_camPos - m_camDir);
    m_camView = Inverse(m_camWorld);
#if defined(IM3D_PLATFORM_WIN)
    Vec2 cursorPos = getWindowRelativeCursor();
    if (hasFocus())
    {
        if (!ImGui::GetIO().WantCaptureKeyboard)
        {
            if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
            {
                kCamSpeed *= 10.0f;
            }
            if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0)
            { // ctrl not pressed
                if (GetAsyncKeyState(0x57) & 0x8000)
                { // W (forward)
                    m_camPos = m_camPos - m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
                }
                if (GetAsyncKeyState(0x41) & 0x8000)
                { // A (left)
                    m_camPos = m_camPos - m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
                }
                if (GetAsyncKeyState(0x53) & 0x8000)
                { // S (backward)
                    m_camPos = m_camPos + m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
                }
                if (GetAsyncKeyState(0x44) & 0x8000)
                { // D (right)
                    m_camPos = m_camPos + m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
                }
                if (GetAsyncKeyState(0x51) & 0x8000)
                { // Q (down)
                    m_camPos = m_camPos - m_camWorld.getCol(1) * (m_deltaTime * kCamSpeed);
                }
                if (GetAsyncKeyState(0x45) & 0x8000)
                { // D (up)
                    m_camPos = m_camPos + m_camWorld.getCol(1) * (m_deltaTime * kCamSpeed);
                }
            }
        }
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
            {
                Vec2 cursorDelta = ((cursorPos - m_prevCursorPos) / Vec2((float)w, (float)h)) * kCamRotationMul;
                m_camDir = Rotation(Vec3(0.0f, 1.0f, 0.0f), -cursorDelta.x) * m_camDir;
                m_camDir = Rotation(m_camWorld.getCol(0), -cursorDelta.y) * m_camDir;
            }
        }
    }
    m_prevCursorPos = cursorPos;
#endif

    m_camFovRad = Im3d::Radians(m_camFovDeg);
    float n = 0.1f;
    float f = 500.0f;
    float a = (float)w / (float)h;
    float scale = tanf(m_camFovRad * 0.5f) * n;
    float viewZ = -1.0f;

    if (m_camOrtho)
    {
        // ortho proj
        scale = 5.0f;
        float r = scale * a;
        float l = -scale * a;
        float t = scale;
        float b = -scale;
        m_camProj = Mat4(
#if defined(IM3D_OPENGL)
            2.0f / (r - l), 0.0f, 0.0f, (r + l) / (l - r),
            0.0f, 2.0f / (t - b), 0.0f, (t + b) / (b - t),
            0.0f, 0.0f, 2.0f / (n - f), (n + f) / (n - f),
            0.0f, 0.0f, 0.0f, 1.0f
#elif defined(IM3D_DX11)
            2.0f / (r - l), 0.0f, 0.0f, (r + l) / (l - r),
            0.0f, 2.0f / (t - b), 0.0f, (t + b) / (b - t),
            0.0f, 0.0f, 1.0f / (n - f), n / (n - f),
            0.0f, 0.0f, 0.0f, 1.0f
#endif
        );
    }
    else
    {
        // infinite perspective proj
        float r = a * scale;
        float l = -r;
        float t = scale;
        float b = -t;

        m_camProj = Mat4(
#if defined(IM3D_OPENGL)
            2.0f * n / (r - l), 0.0f, -viewZ * (r + l) / (r - l), 0.0f,
            0.0f, 2.0f * n / (t - b), -viewZ * (t + b) / (t - b), 0.0f,
            0.0f, 0.0f, viewZ, -2.0f * n,
            0.0f, 0.0f, viewZ, 0.0f
#elif defined(IM3D_DX11)
            2.0f * n / (r - l), 0.0f, -viewZ * (r + l) / (r - l), 0.0f,
            0.0f, 2.0f * n / (t - b), -viewZ * (t + b) / (t - b), 0.0f,
            0.0f, 0.0f, viewZ, -n,
            0.0f, 0.0f, viewZ, 0.0f
#endif
        );
    }

    m_camWorld = LookAt(m_camPos, m_camPos + m_camDir * viewZ);
    m_camView = Inverse(m_camWorld);
    m_camViewProj = m_camProj * m_camView;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin(
        "Frame Info", 0,
        ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%.2f fps", 1.0f / m_deltaTime);
    ImGui::Text("Layers:    %u ", Im3d::GetContext().getLayerCount());
    ImGui::Text("Triangles: %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Triangles));
    ImGui::Text("Lines:     %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Lines));
    ImGui::Text("Points:    %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Points));
    ImGui::End();

    Im3d_NewFrame(w, h);

    return ret;
}

void Example::draw()
{
    static const Vec4 kClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    int w, h;
    std::tie(w, h) = m_window->GetSize();

    Im3d_EndFrame(w, h);

    ImGui::Render();

    winAssert(ValidateRect(m_window->GetHandle(), 0)); // suppress WM_PAINT

    m_glcontext->Present();

    // reset state & clear backbuffer for next frame
    glAssert(glBindVertexArray(0));
    glAssert(glUseProgram(0));
    glAssert(glViewport(0, 0, w, h));
    glAssert(glClearColor(kClearColor.x, kClearColor.y, kClearColor.z, kClearColor.w));
    glAssert(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

bool Example::hasFocus() const
{
    return m_window->HasFocus();
}

Vec2 Example::getWindowRelativeCursor() const
{
    int x, y;
    std::tie(x, y) = m_window->GetCursorPosition();
    return Vec2(static_cast<float>(x), static_cast<float>(y));
}
