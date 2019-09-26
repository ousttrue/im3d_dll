#include <im3d.h>
#include <im3d_newframe.h>
#include "../win32_window.h"
#include "../orbit_camera.h"
#include <dx11_renderer.h>
#include "dx11_context.h"

int main(int, char **)
{
    Win32Window window;
    if (!window.Create(640, 480, L"sample_dx11"))
    {
        return 1;
    }

    OrbitCamera camera;

    DX11Context dx11;
    if(!dx11.Create(window.GetHandle()))
    {
        return 3;
    }

    if (!DX11_Initialize())
    {
        return 1;
    }
    if (!Im3d_DX11_Initialize())
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

        Im3dGui_NewFrame(&camera.state, &mouse, 0, -1);
        #if 1
        Im3d::Gizmo("GizmoUnified", transform.data());
        #else
        Im3d::GizmoTranslation("GizmoUnified", transform.data()+12);
        #endif
        Im3d::EndFrame();

        auto context = dx11.NewFrame(w, h);
        {
            DX11_DrawTeapot(context, camera.state.viewProjection.data(), transform.data());

            Im3d_DX11_Draw(context, camera.state.viewProjection.data(), w, h, Im3d::GetDrawLists(), Im3d::GetDrawListCount());
        }
        dx11.Present();
    }

    Im3d_DX11_Finalize();
    DX11_Finalize();

    return 0;
}
