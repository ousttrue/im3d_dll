#pragma once
#include "im3d_impl_gl3.h"
#include <string>

GL3_EXPORT bool GL3_Initialize(void *hwnd);
GL3_EXPORT void GL3_Finalize();
GL3_EXPORT void GL3_NewFrame(int screenWidth, int screenHeight);
GL3_EXPORT void GL3_DrawTeapot(const float *viewProjection, const float *world);
GL3_EXPORT void GL3_EndFrame();
