#include "win32_window.h"
#include "glcontext.h"
#include "im3d_opengl31.h"
#include "scene.h"
#include "glutil.h"

int main(int, char **)
{
    Win32Window window;
    auto hwnd = window.Create(-1, -1, "Im3d Example");
    if (!hwnd)
    {
        return 1;
    }

    GLContext glcontext;
    if (!glcontext.Initialize(hwnd, 3, 0))
    {
        return 2;
    }

    if (!Im3d_Init())
    {
        return 4;
    }

    Scene scene;

    int last_x = -1;
    int last_y = -1;
    while (true)
    {
        // update
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
        {
            break;
        }

        int x, y, w, h;
        std::tie(x, y) = window.GetCursorPosition();
        Im3d::Vec2 cursorDelta(0, 0);
        if (last_x != -1 && last_y != -1)
        {
            cursorDelta = Im3d::Vec2(x - last_x, y - last_y);
        }
        last_x = x;
        last_y = y;
        std::tie(w, h) = window.GetSize();
        // window.UpdateImGui();
        scene.Update(x, y, w, h, window.GetDeltaTime(), cursorDelta);

        // Unified gizmo operates directly on a 4x4 matrix using the context-global gizmo modes.
        static Im3d::Mat4 transform(1.0f);

        // reset state & clear backbuffer for next frame
        GLClearState(w, h);

        Im3d_NewFrame(x, y, w, h, &scene);

        // frame begin

        // The ID passed to Gizmo() should be unique during a frame - to create gizmos in a loop use PushId()/PopId().
        if (Im3d::Gizmo("GizmoUnified", transform))
        {
            // if Gizmo() returns true, the transform was modified
            switch (Im3d::GetContext().m_gizmoMode)
            {
            case Im3d::GizmoMode_Translation:
            {
                Im3d::Vec3 pos = transform.getTranslation();
                // ImGui::Text("Position: %.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
                break;
            }
            case Im3d::GizmoMode_Rotation:
            {
                Im3d::Vec3 euler = Im3d::ToEulerXYZ(transform.getRotation());
                // ImGui::Text("Rotation: %.3f, %.3f, %.3f", Im3d::Degrees(euler.x), Im3d::Degrees(euler.y), Im3d::Degrees(euler.z));
                break;
            }
            case Im3d::GizmoMode_Scale:
            {
                Im3d::Vec3 scale = transform.getScale();
                // ImGui::Text("Scale: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);
                break;
            }
            default:
                break;
            };
        }

        // Using the transform for drawing *after* the call to Gizmo() causes a 1 frame lag between the gizmo position and the output
        // matrix - this can only be avoided if it's possible to issue the draw call *before* calling Gizmo().
        scene.DrawTeapot(transform);

        // draw
        Im3d_EndFrame(w, h, scene.m_camViewProj);

        ValidateRect(window.GetHandle(), 0); // suppress WM_PAINT

        glcontext.Present();

    }
    Im3d_Shutdown();

    return 0;
}
