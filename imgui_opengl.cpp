#include "imgui_opengl.h"
#include "glutil.h"
#include "im3d.h"
// #include <gl/glew.h>

static GLuint g_ImGuiVertexArray;
static GLuint g_ImGuiVertexBuffer;
static GLuint g_ImGuiIndexBuffer;
static GLuint g_ImGuiShader;
static GLuint g_ImGuiFontTexture;

bool ImGui_Init()
{
    GLuint vs = LoadCompileShader(GL_VERTEX_SHADER, "imgui.glsl", "VERTEX_SHADER\0");
    GLuint fs = LoadCompileShader(GL_FRAGMENT_SHADER, "imgui.glsl", "FRAGMENT_SHADER\0");
    if (vs && fs)
    {
        g_ImGuiShader = glCreateProgram();
        glAttachShader(g_ImGuiShader, vs);
        glAttachShader(g_ImGuiShader, fs);
        bool ret = LinkShaderProgram(g_ImGuiShader);
        glDeleteShader(vs);
        glDeleteShader(fs);
        if (!ret)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    glUseProgram(g_ImGuiShader);
    glUniform1i(glGetUniformLocation(g_ImGuiShader, "txTexture"), 0);
    glUseProgram(0);

    glGenBuffers(1, &g_ImGuiVertexBuffer);
    glGenBuffers(1, &g_ImGuiIndexBuffer);
    glGenVertexArrays(1, &g_ImGuiVertexArray);
    glBindVertexArray(g_ImGuiVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, g_ImGuiVertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid *)offsetof(ImDrawVert, col));
    glBindVertexArray(0);

    unsigned char *txbuf;
    int txX, txY;
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->GetTexDataAsAlpha8(&txbuf, &txX, &txY);
    glGenTextures(1, &g_ImGuiFontTexture);
    glBindTexture(GL_TEXTURE_2D, g_ImGuiFontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, txX, txY, 0, GL_RED, GL_UNSIGNED_BYTE, (const GLvoid *)txbuf);
    io.Fonts->TexID = (void *)g_ImGuiFontTexture;

    io.RenderDrawListsFn = &ImGui_Draw;

    ImGui::StyleColorsDark();

    return true;
}

void ImGui_Shutdown()
{
    glDeleteVertexArrays(1, &g_ImGuiVertexArray);
    glDeleteBuffers(1, &g_ImGuiVertexBuffer);
    glDeleteBuffers(1, &g_ImGuiIndexBuffer);
    glDeleteProgram(g_ImGuiShader);
    glDeleteTextures(1, &g_ImGuiFontTexture);
}

void ImGui_Draw(ImDrawData *_drawData)
{
    ImGuiIO &io = ImGui::GetIO();

    int fbX, fbY;
    fbX = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    fbY = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fbX == 0 || fbY == 0)
    {
        return;
    }
    _drawData->ScaleClipRects(io.DisplayFramebufferScale);

    glViewport(0, 0, (GLsizei)fbX, (GLsizei)fbY);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    auto ortho = Im3d::Mat4(
        2.0f / io.DisplaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f);
    glUseProgram(g_ImGuiShader);

    bool transpose = false;
#ifdef IM3D_MATRIX_ROW_MAJOR
    transpose = true;
#endif
    glUniformMatrix4fv(glGetUniformLocation(g_ImGuiShader, "uProjMatrix"), 1, transpose, (const GLfloat *)ortho);
    glBindVertexArray(g_ImGuiVertexArray);

    for (int i = 0; i < _drawData->CmdListsCount; ++i)
    {
        const ImDrawList *drawList = _drawData->CmdLists[i];
        const ImDrawIdx *indexOffset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_ImGuiVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, drawList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid *)&drawList->VtxBuffer.front(), GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ImGuiIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawList->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid *)drawList->IdxBuffer.Data, GL_STREAM_DRAW);

        for (const ImDrawCmd *pcmd = drawList->CmdBuffer.begin(); pcmd != drawList->CmdBuffer.end(); ++pcmd)
        {
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(drawList, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fbY - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, (GLvoid *)indexOffset);
            }
            indexOffset += pcmd->ElemCount;
        }
    }

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glUseProgram(0);
}