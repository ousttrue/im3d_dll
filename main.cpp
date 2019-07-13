#include "im3d_example.h"
#include "frame.h"

int main(int, char **)
{
    Im3d::Example example;
    if (!example.init(-1, -1, "Im3d Example"))
    {
        return 1;
    }

    while (example.update())
    {
        // frame begin
        Im3d::RandSeed(0);

        ImGui::Begin("Im3d Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        Frame(example);

        // frame end

        ImGui::End();
        example.draw();
    }

    return 0;
}
