#pragma once
#include <Windows.h>

class GLContextImpl;
class GLContext
{
    GLContextImpl *m_impl = nullptr;

public:
    GLContext();
    ~GLContext();
    bool Initialize(HWND hwnd, int major, int minor);
    void Present();
};
