#include "frame.h"
#include "imgui_opengl.h"
#include "win32_window.h"
#include "glcontext.h"
#include "im3d_opengl31.h"
#include "scene.h"
#include "glutil.h"

int main(int, char **)
{
    Win32Window window;
    ImGui::SetCurrentContext(ImGui::CreateContext()); // can't call this in ImGui_Init() because creating the window ends up calling ImGui::GetIO()
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

    if (!ImGui_Init())
    {
        return 3;
    }

    if (!Im3d_Init())
    {
        return 4;
    }

    Scene scene;

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
        std::tie(w, h) = window.GetSize();
        window.UpdateImGui();
        scene.Update(x, y, w, h);

        ImGui::NewFrame();

        Im3d_NewFrame(x, y, w, h, &scene);

        // ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        // ImGui::Begin(
        //     "Frame Info", 0,
        //     ImGuiWindowFlags_NoTitleBar |
        //         ImGuiWindowFlags_NoResize |
        //         ImGuiWindowFlags_NoMove |
        //         ImGuiWindowFlags_NoSavedSettings |
        //         ImGuiWindowFlags_AlwaysAutoResize);
        // ImGui::Text("%.2f fps", 1.0f / m_deltaTime);
        // ImGui::Text("Layers:    %u ", Im3d::GetContext().getLayerCount());
        // ImGui::Text("Triangles: %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Triangles));
        // ImGui::Text("Lines:     %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Lines));
        // ImGui::Text("Points:    %u ", Im3d::GetContext().getPrimitiveCount(Im3d::DrawPrimitive_Points));
        // ImGui::End();

        // frame begin

        ImGui::Begin("Im3d Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        Frame(scene);

        // frame end

        ImGui::End();

        // draw
        Im3d_EndFrame(w, h, scene.m_camViewProj);

        ImGui::Render();

        ValidateRect(window.GetHandle(), 0); // suppress WM_PAINT

        glcontext.Present();

        // reset state & clear backbuffer for next frame
        GLClearState(w, h);
    }
    Im3d_Shutdown();
    ImGui::EndFrame(); // prevent assert due to locked font atlas in DestroyContext() call below
    ImGui::DestroyContext();
    ImGui_Shutdown();

    return 0;
}
