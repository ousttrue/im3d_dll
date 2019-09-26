#pragma once
// #include <array>
#include <DirectXMath.h>

namespace camera
{
inline float dot(const float *row, const float *col)
{
    auto a = row[0] * col[0];
    auto b = row[1] * col[4];
    auto c = row[2] * col[8];
    auto d = row[3] * col[12];
    auto value = a + b + c + d;
    return value;
}

inline DirectX::XMFLOAT4X4 Mult(const DirectX::XMFLOAT4X4 &_l, const DirectX::XMFLOAT4X4 &_r)
{
    auto l = DirectX::XMLoadFloat4x4(&_l);
    auto r = DirectX::XMLoadFloat4x4(&_r);
    auto result = l * r;
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, result);
    return m;
}

struct CameraState
{
    // projection
    float fovYRadians = 30.0f / 180.0f * 3.14f;
    DirectX::XMFLOAT4X4 projection;

    // view
    float viewportWidth = 1.0f;
    float viewportHeight = 1.0f;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 viewInverse;

    // view projection
    DirectX::XMFLOAT4X4 viewProjection;

    void CalcViewProjection()
    {
        auto l = DirectX::XMLoadFloat4x4(&view);
        auto r = DirectX::XMLoadFloat4x4(&projection);
        auto result = DirectX::XMMatrixMultiply(l, r);
        DirectX::XMStoreFloat4x4(&viewProjection, result);
    }
};
} // namespace camera