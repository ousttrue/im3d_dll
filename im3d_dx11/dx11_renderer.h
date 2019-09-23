#pragma once
#include "im3d_impl_dx11.h"

DX11_EXPORT bool DX11_Initialize(void *hwnd);
DX11_EXPORT void DX11_Finalize();
DX11_EXPORT void DX11_DrawTeapot(void *deviceContext, const float *viewProjection, const float *world);
