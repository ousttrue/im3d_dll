#pragma once

#ifdef EXPORT_IM3D_GL3
#define GL3_EXPORT __declspec(dllexport)
#else
#define GL3_EXPORT __declspec(dllimport)
#endif

namespace Im3d {
    struct DrawList;
}

GL3_EXPORT void Im3d_GL3_Draw(const float *viewProjection, int w, int h, const Im3d::DrawList *drawList, int count);
GL3_EXPORT bool Im3d_GL3_Initialize();
GL3_EXPORT void Im3d_GL3_Finalize();
