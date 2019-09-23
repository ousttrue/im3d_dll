#include "im3d_newframe.h"
#include "im3d.h"
#include "im3d_math.h"

static_assert(sizeof(Im3d::VertexData) % 16 == 0);


namespace Im3d
{
static void AppData_setCullFrustum(AppData *ad, const Mat4 &_viewProj, bool _ndcZNegativeOneToOne)
{
    ad->m_cullFrustum[FrustumPlane_Top].x = _viewProj(3, 0) - _viewProj(1, 0);
    ad->m_cullFrustum[FrustumPlane_Top].y = _viewProj(3, 1) - _viewProj(1, 1);
    ad->m_cullFrustum[FrustumPlane_Top].z = _viewProj(3, 2) - _viewProj(1, 2);
    ad->m_cullFrustum[FrustumPlane_Top].w = -(_viewProj(3, 3) - _viewProj(1, 3));

    ad->m_cullFrustum[FrustumPlane_Bottom].x = _viewProj(3, 0) + _viewProj(1, 0);
    ad->m_cullFrustum[FrustumPlane_Bottom].y = _viewProj(3, 1) + _viewProj(1, 1);
    ad->m_cullFrustum[FrustumPlane_Bottom].z = _viewProj(3, 2) + _viewProj(1, 2);
    ad->m_cullFrustum[FrustumPlane_Bottom].w = -(_viewProj(3, 3) + _viewProj(1, 3));

    ad->m_cullFrustum[FrustumPlane_Right].x = _viewProj(3, 0) - _viewProj(0, 0);
    ad->m_cullFrustum[FrustumPlane_Right].y = _viewProj(3, 1) - _viewProj(0, 1);
    ad->m_cullFrustum[FrustumPlane_Right].z = _viewProj(3, 2) - _viewProj(0, 2);
    ad->m_cullFrustum[FrustumPlane_Right].w = -(_viewProj(3, 3) - _viewProj(0, 3));

    ad->m_cullFrustum[FrustumPlane_Left].x = _viewProj(3, 0) + _viewProj(0, 0);
    ad->m_cullFrustum[FrustumPlane_Left].y = _viewProj(3, 1) + _viewProj(0, 1);
    ad->m_cullFrustum[FrustumPlane_Left].z = _viewProj(3, 2) + _viewProj(0, 2);
    ad->m_cullFrustum[FrustumPlane_Left].w = -(_viewProj(3, 3) + _viewProj(0, 3));

    ad->m_cullFrustum[FrustumPlane_Far].x = _viewProj(3, 0) - _viewProj(2, 0);
    ad->m_cullFrustum[FrustumPlane_Far].y = _viewProj(3, 1) - _viewProj(2, 1);
    ad->m_cullFrustum[FrustumPlane_Far].z = _viewProj(3, 2) - _viewProj(2, 2);
    ad->m_cullFrustum[FrustumPlane_Far].w = -(_viewProj(3, 3) - _viewProj(2, 3));

    if (_ndcZNegativeOneToOne)
    {
        ad->m_cullFrustum[FrustumPlane_Near].x = _viewProj(3, 0) + _viewProj(2, 0);
        ad->m_cullFrustum[FrustumPlane_Near].y = _viewProj(3, 1) + _viewProj(2, 1);
        ad->m_cullFrustum[FrustumPlane_Near].z = _viewProj(3, 2) + _viewProj(2, 2);
        ad->m_cullFrustum[FrustumPlane_Near].w = -(_viewProj(3, 3) + _viewProj(2, 3));
    }
    else
    {
        ad->m_cullFrustum[FrustumPlane_Near].x = _viewProj(2, 0);
        ad->m_cullFrustum[FrustumPlane_Near].y = _viewProj(2, 1);
        ad->m_cullFrustum[FrustumPlane_Near].z = _viewProj(2, 2);
        ad->m_cullFrustum[FrustumPlane_Near].w = -(_viewProj(2, 3));
    }

    // normalize
    for (int i = 0; i < FrustumPlane_Count; ++i)
    {
        float d = 1.0f / Length(Vec3(ad->m_cullFrustum[i]));
        ad->m_cullFrustum[i] = ad->m_cullFrustum[i] * d;
    }
}
} // namespace Im3d

void Im3dGui_NewFrame(const camera::CameraState *c, const MouseState *mouse, float deltaTime)
{
    auto &ad = Im3d::GetAppData();

    ad.m_deltaTime = deltaTime;
    ad.m_viewportSize = Im3d::Vec2(c->viewportWidth, c->viewportHeight);

    auto &inv = c->viewInverse;
    ad.m_viewOrigin = Im3d::Vec3(inv[12], inv[13], inv[14]); // for VR use the head position
    ad.m_viewDirection = Im3d::Vec3(-inv[8], -inv[9], -inv[10]);
    ad.m_worldUp = Im3d::Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
    ad.m_projOrtho = false;

    // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
    ad.m_projScaleY = tanf(c->fovYRadians * 0.5f) * 2.0f;

    // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
    Im3d::Vec2 cursorPos((float)mouse->X, (float)mouse->Y);
    cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
    cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
    Im3d::Vec3 rayOrigin, rayDirection;
    {
        rayOrigin = ad.m_viewOrigin;
        rayDirection.x = cursorPos.x / c->projection[0];
        rayDirection.y = cursorPos.y / c->projection[5];
        rayDirection.z = -1.0f;
        Im3d::Mat4 camWorld(
            c->viewInverse[0], c->viewInverse[4], c->viewInverse[8], c->viewInverse[12],
            c->viewInverse[1], c->viewInverse[5], c->viewInverse[9], c->viewInverse[13],
            c->viewInverse[2], c->viewInverse[6], c->viewInverse[10], c->viewInverse[14],
            c->viewInverse[3], c->viewInverse[7], c->viewInverse[11], c->viewInverse[15]);
        rayDirection = camWorld * Im3d::Vec4(Im3d::Normalize(rayDirection), 0.0f);
    }
    ad.m_cursorRayOrigin = rayOrigin;
    ad.m_cursorRayDirection = rayDirection;

    // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via im3d_config.h, or if any of the IsVisible() functions are called.
    Im3d::Mat4 viewProj(
        c->viewProjection[0], c->viewProjection[1], c->viewProjection[2], c->viewProjection[3],
        c->viewProjection[4], c->viewProjection[5], c->viewProjection[6], c->viewProjection[7],
        c->viewProjection[8], c->viewProjection[9], c->viewProjection[10], c->viewProjection[11],
        c->viewProjection[12], c->viewProjection[13], c->viewProjection[14], c->viewProjection[15]);
    ad.setCullFrustum(viewProj, true);
    // Im3d::AppData_setCullFrustum(&ad, viewProj, true);

    // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
    // All key states have an equivalent (and more descriptive) 'Action_' enum.
    //ad.m_keyDown[Im3d::Mouse_Left /*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Mouse_Left /*Im3d::Action_Select*/] = mouse->IsDown(ButtonFlags::Left);

#if 0
    // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl key as an additional predicate.
    bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_L /*Action_GizmoLocal*/] = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_T /*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_R /*Action_GizmoRotation*/] = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_S /*Action_GizmoScale*/] = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

    // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
    ad.m_snapTranslation = 0.0f;
    ad.m_snapRotation = 0.0f;
    ad.m_snapScale = 0.0f;
#endif

    Im3d::NewFrame();
}