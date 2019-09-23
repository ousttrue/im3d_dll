#pragma once
#include <string>

#ifdef gl3_EXPORTS
#define GL3_EXPORT __declspec(dllexport)
#else
#define GL3_EXPORT __declspec(dllimport)
#endif

GL3_EXPORT bool GL3_Initialize(void *hwnd);
GL3_EXPORT void GL3_Finalize();
GL3_EXPORT void GL3_NewFrame(int screenWidth, int screenHeight);
GL3_EXPORT void GL3_DrawTeapot(const float *viewProjection, const float *world);
GL3_EXPORT void GL3_EndFrame();
