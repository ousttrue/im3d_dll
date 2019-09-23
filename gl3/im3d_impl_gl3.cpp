#include "im3d_impl_gl3.h"
#include "gl3_createshader.h"
#include <GL/glew.h>
#include <memory>
#include <string>
#include <im3d.h>


const std::string g_points_vs =
#include "im3d_points.vs"
    ;
const std::string g_points_fs =
#include "im3d_points.fs"
    ;

const std::string g_lines_vs =
#include "im3d_lines.vs"
    ;
const std::string g_lines_fs =
#include "im3d_lines.fs"
    ;

const std::string g_triangles_vs =
#include "im3d_triangles.vs"
    ;
const std::string g_triangles_fs =
#include "im3d_triangles.fs"
    ;

class GL3Shader
{
    GLuint m_shader;
    GLuint m_uniform;

public:
    GL3Shader(GLuint shader)
        : m_shader(shader)
    {
        glCreateBuffers(1, &m_uniform);
    }

    ~GL3Shader()
    {
        glDeleteBuffers(1, &m_uniform);
        glDeleteProgram(m_shader);
    }

    static std::shared_ptr<GL3Shader> Create(const std::string &vs, const std::string &fs)
    {
        auto shader = GL3_CreateShader(vs, fs);
        if (!shader)
        {
            return nullptr;
        }

        auto blockIndex = glGetUniformBlockIndex(shader, "VertexDataBlock");
        glUniformBlockBinding(shader, blockIndex, 0);

        return std::make_shared<GL3Shader>(shader);
    }

    void Use()
    {
        glUseProgram(m_shader);
    }

    void SetUniformFloat2(const char *name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(m_shader, name), x, y);
    }

    void SetUniformMatrix(const char *name, const float *m)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_shader, name), 1, false, m);
    }

    void _SetUniformData(int passVertexCount, const Im3d::VertexData *vertexData)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniform);
        glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)passVertexCount * sizeof(Im3d::VertexData), (GLvoid *)vertexData, GL_DYNAMIC_DRAW);
    }

    int DrawPoints(int passVertexCount, const Im3d::VertexData *vertexData)
    {
        _SetUniformData(passVertexCount, vertexData);

        // instanced draw call, 1 instance per prim
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniform);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, passVertexCount); // for triangles just use the first 3 verts of the strip

        return passVertexCount;
    }

    void DrawLines(int passVertexCount, const Im3d::VertexData *vertexData)
    {
        _SetUniformData(passVertexCount, vertexData);

        // instanced draw call, 1 instance per prim
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniform);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, passVertexCount / 2); // for triangles just use the first 3 verts of the strip
    }

    void DrawTriangles(int passVertexCount, const Im3d::VertexData *vertexData)
    {
        _SetUniformData(passVertexCount, vertexData);

        // instanced draw call, 1 instance per prim
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniform);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, passVertexCount / 3); // for triangles just use the first 3 verts of the strip
    }
};

class GL3Mesh
{
    GLuint m_buffer;
    GLuint m_array;

public:
    GL3Mesh()
    {
        glCreateBuffers(1, &m_buffer);
        glCreateVertexArrays(1, &m_array);
    }

    ~GL3Mesh()
    {
        glDeleteVertexArrays(1, &m_array);
        glDeleteBuffers(1, &m_buffer);
    }

    static std::shared_ptr<GL3Mesh> Create(const Im3d::Vec4 *vertexData, int count)
    {
        auto mesh = std::make_shared<GL3Mesh>();

        // store data
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Im3d::Vec4) * count, (GLvoid *)vertexData, GL_STATIC_DRAW);

        // setup array
        glBindVertexArray(mesh->m_array);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec4), (GLvoid *)0);
        glBindVertexArray(0);

        return mesh;
    }

    void Bind()
    {
        glBindVertexArray(m_array);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    }
};

std::shared_ptr<GL3Shader> g_Im3dShaderPoints;
std::shared_ptr<GL3Shader> g_Im3dShaderLines;
std::shared_ptr<GL3Shader> g_Im3dShaderTriangles;
std::shared_ptr<GL3Mesh> g_Im3dVertexArray;


bool Im3d_GL3_Initialize()
{
    return true;
}

void Im3d_GL3_Finalize()
{
    g_Im3dShaderPoints.reset();
    g_Im3dShaderLines.reset();
    g_Im3dShaderTriangles.reset();
    g_Im3dVertexArray.reset();
}

void Im3d_GL3_Draw(const float *viewProjection, int w, int h, const struct Im3d::DrawList *drawList, int count)
{
    // m_impl->Draw(viewProjection, w, h, drawList, count);
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < count; ++i, ++drawList)
    {
        if (drawList->m_layerId == Im3d::MakeId("NamedLayer"))
        {
            // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
        }

        std::shared_ptr<GL3Shader> sh;
        switch (drawList->m_primType)
        {
        case Im3d::DrawPrimitive_Points:
            if (!g_Im3dShaderPoints)
            {
                g_Im3dShaderPoints = GL3Shader::Create(g_points_vs, g_points_fs);
            }
            sh = g_Im3dShaderPoints;
            glDisable(GL_CULL_FACE); // points are view-aligned
            break;

        case Im3d::DrawPrimitive_Lines:
            if (!g_Im3dShaderLines)
            {
                g_Im3dShaderLines = GL3Shader::Create(g_lines_vs, g_lines_fs);
            }
            sh = g_Im3dShaderLines;
            glDisable(GL_CULL_FACE); // lines are view-aligned
            break;

        case Im3d::DrawPrimitive_Triangles:
            if (!g_Im3dShaderTriangles)
            {
                g_Im3dShaderTriangles = GL3Shader::Create(g_triangles_vs, g_triangles_fs);
            }
            sh = g_Im3dShaderTriangles;
            glEnable(GL_CULL_FACE); // culling valid for triangles, but optional
            break;

        default:
            IM3D_ASSERT(false);
            return;
        };
        sh->Use();

        if (!g_Im3dVertexArray)
        {
            // in this example we're using a static buffer as the vertex source with a uniform buffer to provide
            // the shader with the Im3d vertex data
            Im3d::Vec4 vertexData[] = {
                Im3d::Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
                Im3d::Vec4(1.0f, -1.0f, 0.0f, 1.0f),
                Im3d::Vec4(-1.0f, 1.0f, 0.0f, 1.0f),
                Im3d::Vec4(1.0f, 1.0f, 0.0f, 1.0f)};
            g_Im3dVertexArray = GL3Mesh::Create(vertexData, 4);
        }
        g_Im3dVertexArray->Bind();

        auto &ad = Im3d::GetAppData();
        sh->SetUniformFloat2("uViewport", ad.m_viewportSize.x, ad.m_viewportSize.y);
        sh->SetUniformMatrix("uViewProjMatrix", viewProjection);

        // Uniform buffers have a size limit; split the vertex data into several passes.
        const int kMaxBufferSize = 64 * 1024; // assuming 64kb here but the application should check the implementation limit
        const int kVertexPerPass = kMaxBufferSize / (sizeof(Im3d::VertexData));

        const Im3d::VertexData *vertexData = drawList->m_vertexData;
        auto remainingVertexCount = drawList->m_vertexCount;
        while (remainingVertexCount > 0)
        {
            int passVertexCount = remainingVertexCount < kVertexPerPass ? remainingVertexCount : kVertexPerPass;
            switch (drawList->m_primType)
            {
            case Im3d::DrawPrimitive_Points:
                sh->DrawPoints(passVertexCount, vertexData);
                break;
            case Im3d::DrawPrimitive_Lines:
                sh->DrawLines(passVertexCount, vertexData);
                break;
            case Im3d::DrawPrimitive_Triangles:
                sh->DrawTriangles(passVertexCount, vertexData);
                break;
            }
            vertexData += passVertexCount;
            remainingVertexCount -= passVertexCount;
        }
    }
    glDisable(GL_BLEND);
}
