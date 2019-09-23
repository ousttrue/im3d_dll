#include "im3d_gui.h"
#include "im3d.h"
#include "im3d_math.h"
#include <GL/glew.h>
#include "gl3_createshader.h"

static_assert(sizeof(Im3d::VertexData) % 16 == 0);

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

namespace Im3d
{
static void AppData_setCullFrustum(AppData *ad, const Mat4 &_viewProj, bool _ndcZNegativeOneToOne)
{
    ad->m_cullFrustum[FrustumPlane_Top].x = _viewProj(3, 0) - _viewProj(1, 0);
    ad->m_cullFrustum[FrustumPlane_Top].y = _viewProj(3, 1) - _viewProj(1, 1);
    ad->m_cullFrustum[FrustumPlane_Top].z = _viewProj(3, 2) - _viewProj(1, 2);
    ad->m_cullFrustum[FrustumPlane_Top].w = -(_viewProj(3, 3) - _viewProj(1, 3));

    ad->m_cullFrustum[FrustumPlane_Bottom].x = _viewProj(3, 0) + _viewProj(1, 0);
    ad->m_cullFrustum[FrustumPlane_Bottom].y = _viewProj(3, 1) + _viewProj(1, 1);
    ad->m_cullFrustum[FrustumPlane_Bottom].z = _viewProj(3, 2) + _viewProj(1, 2);
    ad->m_cullFrustum[FrustumPlane_Bottom].w = -(_viewProj(3, 3) + _viewProj(1, 3));

    ad->m_cullFrustum[FrustumPlane_Right].x = _viewProj(3, 0) - _viewProj(0, 0);
    ad->m_cullFrustum[FrustumPlane_Right].y = _viewProj(3, 1) - _viewProj(0, 1);
    ad->m_cullFrustum[FrustumPlane_Right].z = _viewProj(3, 2) - _viewProj(0, 2);
    ad->m_cullFrustum[FrustumPlane_Right].w = -(_viewProj(3, 3) - _viewProj(0, 3));

    ad->m_cullFrustum[FrustumPlane_Left].x = _viewProj(3, 0) + _viewProj(0, 0);
    ad->m_cullFrustum[FrustumPlane_Left].y = _viewProj(3, 1) + _viewProj(0, 1);
    ad->m_cullFrustum[FrustumPlane_Left].z = _viewProj(3, 2) + _viewProj(0, 2);
    ad->m_cullFrustum[FrustumPlane_Left].w = -(_viewProj(3, 3) + _viewProj(0, 3));

    ad->m_cullFrustum[FrustumPlane_Far].x = _viewProj(3, 0) - _viewProj(2, 0);
    ad->m_cullFrustum[FrustumPlane_Far].y = _viewProj(3, 1) - _viewProj(2, 1);
    ad->m_cullFrustum[FrustumPlane_Far].z = _viewProj(3, 2) - _viewProj(2, 2);
    ad->m_cullFrustum[FrustumPlane_Far].w = -(_viewProj(3, 3) - _viewProj(2, 3));

    if (_ndcZNegativeOneToOne)
    {
        ad->m_cullFrustum[FrustumPlane_Near].x = _viewProj(3, 0) + _viewProj(2, 0);
        ad->m_cullFrustum[FrustumPlane_Near].y = _viewProj(3, 1) + _viewProj(2, 1);
        ad->m_cullFrustum[FrustumPlane_Near].z = _viewProj(3, 2) + _viewProj(2, 2);
        ad->m_cullFrustum[FrustumPlane_Near].w = -(_viewProj(3, 3) + _viewProj(2, 3));
    }
    else
    {
        ad->m_cullFrustum[FrustumPlane_Near].x = _viewProj(2, 0);
        ad->m_cullFrustum[FrustumPlane_Near].y = _viewProj(2, 1);
        ad->m_cullFrustum[FrustumPlane_Near].z = _viewProj(2, 2);
        ad->m_cullFrustum[FrustumPlane_Near].w = -(_viewProj(2, 3));
    }

    // normalize
    for (int i = 0; i < FrustumPlane_Count; ++i)
    {
        float d = 1.0f / Length(Vec3(ad->m_cullFrustum[i]));
        ad->m_cullFrustum[i] = ad->m_cullFrustum[i] * d;
    }
}
} // namespace Im3d

void Im3dGui_NewFrame(const camera::CameraState *c, const MouseState *mouse, float deltaTime)
{
    auto &ad = Im3d::GetAppData();

    ad.m_deltaTime = deltaTime;
    ad.m_viewportSize = Im3d::Vec2(c->viewportWidth, c->viewportHeight);

    auto &inv = c->viewInverse;
    ad.m_viewOrigin = Im3d::Vec3(inv[12], inv[13], inv[14]); // for VR use the head position
    ad.m_viewDirection = Im3d::Vec3(-inv[8], -inv[9], -inv[10]);
    ad.m_worldUp = Im3d::Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
    ad.m_projOrtho = false;

    // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
    ad.m_projScaleY = tanf(c->fovYRadians * 0.5f) * 2.0f;

    // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
    Im3d::Vec2 cursorPos((float)mouse->X, (float)mouse->Y);
    cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
    cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
    Im3d::Vec3 rayOrigin, rayDirection;
    {
        rayOrigin = ad.m_viewOrigin;
        rayDirection.x = cursorPos.x / c->projection[0];
        rayDirection.y = cursorPos.y / c->projection[5];
        rayDirection.z = -1.0f;
        Im3d::Mat4 camWorld(
            c->viewInverse[0], c->viewInverse[4], c->viewInverse[8], c->viewInverse[12],
            c->viewInverse[1], c->viewInverse[5], c->viewInverse[9], c->viewInverse[13],
            c->viewInverse[2], c->viewInverse[6], c->viewInverse[10], c->viewInverse[14],
            c->viewInverse[3], c->viewInverse[7], c->viewInverse[11], c->viewInverse[15]);
        rayDirection = camWorld * Im3d::Vec4(Im3d::Normalize(rayDirection), 0.0f);
    }
    ad.m_cursorRayOrigin = rayOrigin;
    ad.m_cursorRayDirection = rayDirection;

    // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via im3d_config.h, or if any of the IsVisible() functions are called.
    Im3d::Mat4 viewProj(
        c->viewProjection[0], c->viewProjection[1], c->viewProjection[2], c->viewProjection[3],
        c->viewProjection[4], c->viewProjection[5], c->viewProjection[6], c->viewProjection[7],
        c->viewProjection[8], c->viewProjection[9], c->viewProjection[10], c->viewProjection[11],
        c->viewProjection[12], c->viewProjection[13], c->viewProjection[14], c->viewProjection[15]);
    ad.setCullFrustum(viewProj, true);
    // Im3d::AppData_setCullFrustum(&ad, viewProj, true);

    // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
    // All key states have an equivalent (and more descriptive) 'Action_' enum.
    //ad.m_keyDown[Im3d::Mouse_Left /*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Mouse_Left /*Im3d::Action_Select*/] = mouse->IsDown(ButtonFlags::Left);

#if 0
    // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl key as an additional predicate.
    bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_L /*Action_GizmoLocal*/] = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_T /*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_R /*Action_GizmoRotation*/] = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_S /*Action_GizmoScale*/] = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

    // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
    ad.m_snapTranslation = 0.0f;
    ad.m_snapRotation = 0.0f;
    ad.m_snapScale = 0.0f;
#endif

    Im3d::NewFrame();
}

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
