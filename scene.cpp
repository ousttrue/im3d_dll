#include "scene.h"
#include "im3d_math.h"
#include <GL/glew.h>
#include "glutil.h"
#include "teapot.h"

Scene::Scene()
{
}

void Scene::Update(int mouseX, int mouseY, int windowW, int windowH)
{
    float kCamSpeed = 2.0f;
    float kCamSpeedMul = 10.0f;
    float kCamRotationMul = 10.0f;
    // m_camWorld = Im3d::LookAt(m_camPos, m_camPos - m_camDir);
    // m_camView = Im3d::Inverse(m_camWorld);

    // if (!ImGui::GetIO().WantCaptureKeyboard)
    // {
    //     if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
    //     {
    //         kCamSpeed *= 10.0f;
    //     }
    //     if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0)
    //     { // ctrl not pressed
    //         if (GetAsyncKeyState(0x57) & 0x8000)
    //         { // W (forward)
    //             m_camPos = m_camPos - m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
    //         }
    //         if (GetAsyncKeyState(0x41) & 0x8000)
    //         { // A (left)
    //             m_camPos = m_camPos - m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
    //         }
    //         if (GetAsyncKeyState(0x53) & 0x8000)
    //         { // S (backward)
    //             m_camPos = m_camPos + m_camWorld.getCol(2) * (m_deltaTime * kCamSpeed);
    //         }
    //         if (GetAsyncKeyState(0x44) & 0x8000)
    //         { // D (right)
    //             m_camPos = m_camPos + m_camWorld.getCol(0) * (m_deltaTime * kCamSpeed);
    //         }
    //         if (GetAsyncKeyState(0x51) & 0x8000)
    //         { // Q (down)
    //             m_camPos = m_camPos - m_camWorld.getCol(1) * (m_deltaTime * kCamSpeed);
    //         }
    //         if (GetAsyncKeyState(0x45) & 0x8000)
    //         { // D (up)
    //             m_camPos = m_camPos + m_camWorld.getCol(1) * (m_deltaTime * kCamSpeed);
    //         }
    //     }
    // }
    // if (!ImGui::GetIO().WantCaptureMouse)
    // {
    //     if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    //     {
    //         Vec2 cursorDelta = ((cursorPos - m_prevCursorPos) / Vec2((float)w, (float)h)) * kCamRotationMul;
    //         m_camDir = Rotation(Vec3(0.0f, 1.0f, 0.0f), -cursorDelta.x) * m_camDir;
    //         m_camDir = Rotation(m_camWorld.getCol(0), -cursorDelta.y) * m_camDir;
    //     }
    // }

    // m_prevCursorPos = cursorPos;
    m_camFovRad = Im3d::Radians(m_camFovDeg);
    float n = 0.1f;
    float f = 500.0f;
    float a = (float)windowW / (float)windowH;
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
        m_camProj = Im3d::Mat4(
            2.0f / (r - l), 0.0f, 0.0f, (r + l) / (l - r),
            0.0f, 2.0f / (t - b), 0.0f, (t + b) / (b - t),
            0.0f, 0.0f, 2.0f / (n - f), (n + f) / (n - f),
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }
    else
    {
        // infinite perspective proj
        float r = a * scale;
        float l = -r;
        float t = scale;
        float b = -t;

        m_camProj = Im3d::Mat4(
            2.0f * n / (r - l), 0.0f, -viewZ * (r + l) / (r - l), 0.0f,
            0.0f, 2.0f * n / (t - b), -viewZ * (t + b) / (t - b), 0.0f,
            0.0f, 0.0f, viewZ, -2.0f * n,
            0.0f, 0.0f, viewZ, 0.0f
        );
    }

    m_camWorld = Im3d::LookAt(m_camPos, m_camPos + m_camDir * viewZ);
    m_camView = Im3d::Inverse(m_camWorld);
    m_camViewProj = m_camProj * m_camView;
}

void Scene::DrawTeapot(const float *world)
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
            shTeapot = glCreateProgram();
            glAttachShader(shTeapot, vs);
            glAttachShader(shTeapot, fs);
            bool ret = LinkShaderProgram(shTeapot);
            glDeleteShader(vs);
            glDeleteShader(fs);
            if (!ret)
            {
                return;
            }
        }
        else
        {
            return;
        }
        glGenBuffers(1, &vbTeapot);
        glGenBuffers(1, &ibTeapot);
        glGenVertexArrays(1, &vaTeapot);
        glBindVertexArray(vaTeapot);
        glBindBuffer(GL_ARRAY_BUFFER, vbTeapot);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec3) * 2, (GLvoid *)0);
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
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, m_camViewProj.m);
    glBindVertexArray(vaTeapot);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid *)0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    glUseProgram(0);
}
