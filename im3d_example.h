#pragma once
#include "im3d.h"

#define IM3D_COMPILER_MSVC
#define IM3D_PLATFORM_WIN

#include <Windows.h>

#define winAssert(e) IM3D_VERIFY_MSG(e, Im3d::GetPlatformErrorString(GetLastError()))

class Win32Window;

namespace Im3d
{
const char *GetPlatformErrorString(DWORD _err);
}

// Graphics API
#if defined(IM3D_OPENGL)
// OpenGL
//#define IM3D_OPENGL_VMAJ    3
//#define IM3D_OPENGL_VMIN    3
//#define IM3D_OPENGL_VSHADER "#version 150"

#include "GL/glew.h"
#define glAssert(call)                                                           \
    do                                                                           \
    {                                                                            \
        (call);                                                                  \
        GLenum err = glGetError();                                               \
        if (err != GL_NO_ERROR)                                                  \
        {                                                                        \
            Im3d::Assert(#call, __FILE__, __LINE__, Im3d::GetGlEnumString(err)); \
            IM3D_BREAK();                                                        \
        }                                                                        \
    } while (0)

namespace Im3d
{
// Return 0 on failure (prints log info to stderr). _defines is a list of null-separated strings e.g. "DEFINE1 1\0DEFINE2 1\0"
GLuint LoadCompileShader(GLenum _stage, const char *_path, const char *_defines = 0);
// Return false on failure (prints log info to stderr).
bool LinkShaderProgram(GLuint _handle);

const char *GetGlEnumString(GLenum _enum);
const char *GlGetString(GLenum _name);
} // namespace Im3d

#endif

#define IM3D_UNUSED(x)   \
    do                   \
    {                    \
        (void)sizeof(x); \
    } while (0)
#ifdef IM3D_COMPILER_MSVC
#define IM3D_BREAK() __debugbreak()
#else
#include <cstdlib>
#define IM3D_BREAK() abort()
#endif

#define IM3D_ASSERT_MSG(e, msg, ...)                                  \
    do                                                                \
    {                                                                 \
        if (!(e))                                                     \
        {                                                             \
            Im3d::Assert(#e, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
            IM3D_BREAK();                                             \
        }                                                             \
    } while (0)

#undef IM3D_ASSERT
#define IM3D_ASSERT(e) IM3D_ASSERT_MSG(e, 0, 0)
#define IM3D_VERIFY_MSG(e, msg, ...) IM3D_ASSERT_MSG(e, msg, ##__VA_ARGS__)
#define IM3D_VERIFY(e) IM3D_VERIFY_MSG(e, 0, 0)

#ifndef __COUNTER__
#define __COUNTER__ __LINE__
#endif
#define IM3D_TOKEN_CONCATENATE_(_t0, _t1) _t0##_t1
#define IM3D_TOKEN_CONCATENATE(_t0, _t1) IM3D_TOKEN_CONCATENATE_(_t0, _t1)
#define IM3D_UNIQUE_NAME(_base) IM3D_TOKEN_CONCATENATE(_base, __COUNTER__)
#define IM3D_STRINGIFY_(_t) #_t
#define IM3D_STRINGIFY(_t) IM3D_STRINGIFY_(_t)

#include "im3d_math.h"

#include "imgui/imgui.h"

namespace Im3d
{

void Assert(const char *_e, const char *_file, int _line, const char *_msg, ...);

void RandSeed(int _seed);
int RandInt(int _min, int _max);
float RandFloat(float _min, float _max);
Vec3 RandVec3(float _min, float _max);
Mat3 RandRotation();
Color RandColor(float _min, float _max);

void DrawNdcQuad();
void DrawTeapot(const Mat4 &_world, const Mat4 &_viewProj);

struct Example
{
    Win32Window *m_window = nullptr;
    Example();
    ~Example();

    bool init(int _width, int _height, const char *_title);

    bool update();
    void draw();

    // window
    Vec2 m_prevCursorPos;

    bool hasFocus() const;
    Vec2 getWindowRelativeCursor() const;

    // 3d camera
    bool m_camOrtho = false;
    Vec3 m_camPos = {0};
    Vec3 m_camDir = {0};
    float m_camFovDeg = 0;
    float m_camFovRad = 0;
    Mat4 m_camWorld = {0};
    Mat4 m_camView = {0};
    Mat4 m_camProj = {0};
    Mat4 m_camViewProj = {0};

    float m_deltaTime = 0;

    // platform/graphics specifics
    LARGE_INTEGER m_currTime = {0};
    LARGE_INTEGER m_prevTime = {0};

    HDC m_hdc = nullptr;
    HGLRC m_hglrc = nullptr;

}; // struct Example

extern Example *g_Example;

} // namespace Im3d
