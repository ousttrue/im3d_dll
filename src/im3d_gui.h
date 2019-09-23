#pragma once
#include "camera_state.h"
#include "mouse_state.h"

bool Im3dGui_Initialize();
void Im3dGui_Finalize();

void Im3dGui_NewFrame(const camera::CameraState *camera, const MouseState *mouse, float deltaTime);
void Im3dGui_Manipulate(float world[16]);
void Im3dGui_EndFrame();

void Im3dGui_Draw(const float *viewProjection, int w, int h);
