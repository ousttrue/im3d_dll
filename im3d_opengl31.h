#pragma once

namespace camera
{
    struct CameraState;
}

bool Im3d_Init();
void Im3d_Shutdown();
void Im3d_NewFrame(int x, int y, const camera::CameraState *c);
void Im3d_EndFrame(int w, int h, const float *viewProj);
