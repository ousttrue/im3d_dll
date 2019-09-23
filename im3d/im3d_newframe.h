#pragma once
#include "camera_state.h"
#include "mouse_state.h"
#include "im3d_config.h"

IM3D_EXPORT void Im3dGui_NewFrame(const camera::CameraState *camera, const MouseState *mouse, float deltaTime);
