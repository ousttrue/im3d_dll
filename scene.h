#pragma once
#include "im3d.h"
#include "im3d_math.h"

/// Camera management
class Scene
{
public:
    // 3d camera
    bool m_camOrtho = false;
    Im3d::Vec3 m_camPos = {0.0f, 2.0f, 3.0f};
    Im3d::Vec3 m_camDir = Im3d::Normalize(Im3d::Vec3(0.0f, -0.5f, -1.0f));
    float m_camFovDeg = 50.0f;
    float m_camFovRad = 0;
    Im3d::Mat4 m_camWorld = {0};
    Im3d::Mat4 m_camView = {0};
    Im3d::Mat4 m_camProj = {0};
    Im3d::Mat4 m_camViewProj = {0};

    int m_lastX = 0;
    int m_lastY = 0;

public:
    Scene();
    void Update(int mouseX, int mouseY, int windowW, int windowH, float deltaTime, const Im3d::Vec2 cursorPosDelta);
    void DrawTeapot(const float *world);
};
