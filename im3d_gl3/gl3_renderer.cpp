#include "gl3_renderer.h"
#include "gl3_createshader.h"
#include "wgl_context.h"
#include <array>
#include <string>
#include <GL/glew.h>
#include "../teapot.h"

const std::string g_vs =
#include "model.vs"
    ;
const std::string g_fs =
#include "model.fs"
    ;

class GL3RendererImpl;
class GL3Renderer
{
    GL3RendererImpl *m_impl = nullptr;

public:
    GL3Renderer();
    ~GL3Renderer();
    void NewFrame(int screenWidth, int screenHeight);
    void DrawTeapot(const float *viewProjection, const float *world);
};

GL3Renderer::GL3Renderer()
{
}

GL3Renderer::~GL3Renderer()
{
}

void GL3Renderer::NewFrame(int screenWidth, int screenHeight)
{
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screenWidth, screenHeight);

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
}

void GL3Renderer::DrawTeapot(const float *viewProjection, const float *world)
{
    static GLuint shTeapot;
    static GLuint vbTeapot;
    static GLuint ibTeapot;
    static GLuint vaTeapot;
    if (shTeapot == 0)
    {
        shTeapot = GL3_CreateShader(g_vs, g_fs);
        glGenBuffers(1, &vbTeapot);
        glGenBuffers(1, &ibTeapot);
        glGenVertexArrays(1, &vaTeapot);
        glBindVertexArray(vaTeapot);
        glBindBuffer(GL_ARRAY_BUFFER, vbTeapot);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3 * 2, (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)(sizeof(float) * 3));
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_teapotVertices), (GLvoid *)s_teapotVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibTeapot);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_teapotIndices), (GLvoid *)s_teapotIndices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
    glUseProgram(shTeapot);
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uWorldMatrix"), 1, false, world);
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, viewProjection);
    glBindVertexArray(vaTeapot);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid *)0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    glUseProgram(0);
}

WGLContext g_glcontext;
GL3Renderer *g_renderer = nullptr;

bool GL3_Initialize(void *hwnd)
{
    if (!g_glcontext.Create(hwnd, 3, 0))
    {
        return false;
    }
    g_renderer = new GL3Renderer();
    return true;
}

void GL3_Finalize()
{
    if (g_renderer)
    {
        delete g_renderer;
        g_renderer = nullptr;
    }
}

void GL3_NewFrame(int screenWidth, int screenHeight)
{
    g_renderer->NewFrame(screenWidth, screenHeight);
}

void GL3_DrawTeapot(const float *viewProjection, const float *world)
{
    g_renderer->DrawTeapot(viewProjection, world);
}

void GL3_EndFrame()
{
    g_glcontext.Present();
}

// unsigned int GL3_CreateShader(const std::string &vsSrc, const std::string &fsSrc)
