#pragma once
#include <string>

bool GL3_Initialize(void *hwnd);
void GL3_Finalize();
void GL3_NewFrame(int screenWidth, int screenHeight);
void GL3_DrawTeapot(const float *viewProjection, const float *world);
void GL3_EndFrame();
unsigned int GL3_CreateShader(const std::string &vsSrc, const std::string &fsSrc);
