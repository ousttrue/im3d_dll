#pragma once
#include "camera_state.h"
#include "mouse_state.h"
#include "im3d_config.h"

IM3D_EXPORT bool Im3dGui_Initialize();
IM3D_EXPORT void Im3dGui_Finalize();

IM3D_EXPORT void Im3dGui_NewFrame(const camera::CameraState *camera, const MouseState *mouse, float deltaTime);
IM3D_EXPORT void Im3dGui_Manipulate(float world[16]);
IM3D_EXPORT void Im3dGui_EndFrame();

IM3D_EXPORT void Im3dGui_Draw(const float *viewProjection, int w, int h);
