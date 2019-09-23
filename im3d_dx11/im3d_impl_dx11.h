#pragma once

#ifdef im3d_dx11_EXPORTS
#define DX11_EXPORT __declspec(dllexport)
#else
#define DX11_EXPORT __declspec(dllimport)
#endif

namespace Im3d
{
struct DrawList;
}

DX11_EXPORT void Im3d_DX11_Draw(void *deviceContext, const float *viewProjection, int w, int h, const Im3d::DrawList *drawList, int count);
DX11_EXPORT bool Im3d_DX11_Initialize();
DX11_EXPORT void Im3d_DX11_Finalize();
