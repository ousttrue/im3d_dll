#include "win32_window.h"
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

    OrbitCamera camera;

    if (!GL3_Initialize(window.GetHandle()))
    {
        return 1;
    }
    if (!Im3dGui_Initialize())
    {
        return 2;
    }

    // Unified gizmo operates directly on a 4x4 matrix using the context-global gizmo modes.
    //static Im3d::Mat4 transform(1.0f);
    static std::array<float, 16> transform = {
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
    };

    while (window.IsRunning())
    {
        int w, h;
        auto mouse = window.GetMouseState();
        std::tie(w, h) = window.GetSize();
        camera.SetScreenSize((float)w, (float)h);
        camera.MouseInput(mouse);
        camera.state.CalcViewProjection();

        Im3dGui_NewFrame(&camera.state, &mouse, 0);
        Im3dGui_Manipulate(transform.data());
        Im3dGui_EndFrame();

        GL3_NewFrame(w, h);
        {
            GL3_DrawTeapot(camera.state.viewProjection.data(), transform.data());
            Im3dGui_Draw(camera.state.viewProjection.data(), w, h);
        }
        GL3_EndFrame();
    }

    return 0;
}
