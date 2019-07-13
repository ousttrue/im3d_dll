#pragma once

bool Im3d_Init();
void Im3d_Shutdown();
void Im3d_NewFrame(int x, int y, int w, int h, class Scene *);
void Im3d_EndFrame(int w, int h, const float *viewProj);
