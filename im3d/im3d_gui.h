#pragma once
#include "camera_state.h"
#include "mouse_state.h"
#include "im3d_config.h"

IM3D_EXPORT bool Im3dGui_Initialize();
IM3D_EXPORT void Im3dGui_Finalize();

IM3D_EXPORT void Im3dGui_NewFrame(const camera::CameraState *camera, const MouseState *mouse, float deltaTime);

namespace Im3d {
    struct DrawList;
}

IM3D_EXPORT void Im3dGui_Impl_GL3_Draw(const float *viewProjection, int w, int h, const Im3d::DrawList *drawList, int count);
