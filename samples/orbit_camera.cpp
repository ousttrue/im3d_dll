#include "orbit_camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

void OrbitCamera::CalcView()
{
    auto yaw = DirectX::XMMatrixRotationY(yawRadians);
    auto pitch = DirectX::XMMatrixRotationX(pitchRadians);
    auto t = DirectX::XMMatrixTranslation(-shiftX, -shiftY, -shiftZ);
    auto yawPitch = yaw * pitch;
    DirectX::XMStoreFloat4x4(&state.view, yawPitch * t);

    auto inv_t = DirectX::XMMatrixTranslation(shiftX, shiftY, shiftZ);
    auto transposed = DirectX::XMMatrixTranspose(yawPitch);
    DirectX::XMStoreFloat4x4(&state.viewInverse, inv_t * transposed);
}

void OrbitCamera::CalcPerspective()
{
    auto projection = DirectX::XMMatrixPerspectiveFovRH(state.fovYRadians, aspectRatio, zNear, zFar);
    DirectX::XMStoreFloat4x4(&state.projection, projection);
}

void OrbitCamera::SetScreenSize(float w, float h)
{
    if (w == state.viewportWidth && h == state.viewportHeight)
    {
        return;
    }
    if (h == 0 || w == 0)
    {
        aspectRatio = 1.0f;
    }
    else
    {
        aspectRatio = w / (float)h;
    }
    state.viewportWidth = w;
    state.viewportHeight = h;
    CalcPerspective();
}

void OrbitCamera::MouseInput(const MouseState &mouse)
{
    if (prevMouseX != -1 && prevMouseY != -1)
    {
        auto deltaX = mouse.X - prevMouseX;
        auto deltaY = mouse.Y - prevMouseY;

        if (mouse.IsDown(ButtonFlags::Right))
        {
            const auto FACTOR = 1.0f / 180.0f * 1.7f;
            yawRadians += deltaX * FACTOR;
            pitchRadians += deltaY * FACTOR;
        }
        if (mouse.IsDown(ButtonFlags::Middle))
        {
            shiftX -= deltaX / (float)state.viewportHeight * shiftZ;
            shiftY += deltaY / (float)state.viewportHeight * shiftZ;
        }
        if (mouse.Wheel > 0)
        {
            shiftZ *= 0.9f;
        }
        else if (mouse.Wheel < 0)
        {
            shiftZ *= 1.1f;
        }
    }
    prevMouseX = mouse.X;
    prevMouseY = mouse.Y;
    CalcView();
}
