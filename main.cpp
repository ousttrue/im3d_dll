#include "win32_window.h"
#include "glcontext.h"
#include "im3d_gui.h"
#include "scene.h"
#include "glutil.h"
#include "orbit_camera.h"
#include <im3d.h>
#include <im3d_math.h>

int main(int, char **)
{
    Win32Window window;
    if (!window.Create(640, 480, L"Im3d Example"))
    {
        return 1;
    }

    GLContext glcontext;
    if (!glcontext.Initialize((HWND)window.GetHandle(), 3, 0))
    {
        return 2;
    }

    Im3dGui gui;

    Scene scene;

    OrbitCamera camera;

    // Unified gizmo operates directly on a 4x4 matrix using the context-global gizmo modes.
    static Im3d::Mat4 transform(1.0f);

    while (window.IsRunning())
    {
        int w, h;
        auto mouse = window.GetMouseState();
        std::tie(w, h) = window.GetSize();
        camera.SetScreenSize((float)w, (float)h);
        // window.UpdateImGui();
        camera.MouseInput(mouse);
        camera.state.CalcViewProjection();

        // reset state & clear backbuffer for next frame
        GLClearState(w, h);

        gui.NewFrame(&camera.state, &mouse, 0);

        // The ID passed to Gizmo() should be unique during a frame - to create gizmos in a loop use PushId()/PopId().
        Im3d::Gizmo("GizmoUnified", transform);

        // Using the transform for drawing *after* the call to Gizmo() causes a 1 frame lag between the gizmo position and the output
        // matrix - this can only be avoided if it's possible to issue the draw call *before* calling Gizmo().
        scene.DrawTeapot(camera.state.viewProjection.data(), transform);

        // draw
        gui.Draw(camera.state.viewProjection.data(), w, h);

        ValidateRect((HWND)window.GetHandle(), 0); // suppress WM_PAINT

        glcontext.Present();
    }

    return 0;
}
