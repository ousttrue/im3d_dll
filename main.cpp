#include "win32_window.h"
#include "wgl_context.h"
#include "im3d_gui.h"
#include "orbit_camera.h"
#include "gl3_renderer.h"


int main(int, char **)
{
    Win32Window window;
    if (!window.Create(640, 480, L"Im3d Example"))
    {
        return 1;
    }

    WGLContext glcontext;
    if (!glcontext.Create(window.GetHandle(), 3, 0))
    {
        return 2;
    }

    Im3dGui gui;

    OrbitCamera camera;

    // Unified gizmo operates directly on a 4x4 matrix using the context-global gizmo modes.
    //static Im3d::Mat4 transform(1.0f);
    static std::array<float, 16> transform = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    GL3Renderer renderer;

    while (window.IsRunning())
    {
        int w, h;
        auto mouse = window.GetMouseState();
        std::tie(w, h) = window.GetSize();
        camera.SetScreenSize((float)w, (float)h);
        camera.MouseInput(mouse);
        camera.state.CalcViewProjection();

        renderer.NewFrame(w, h);

        gui.NewFrame(&camera.state, &mouse, 0);

        gui.Manipulate(transform.data());

        renderer.DrawTeapot(camera.state.viewProjection.data(), transform.data());

        // draw
        gui.Draw(camera.state.viewProjection.data(), w, h);

        glcontext.Present();
    }

    return 0;
}
