#pragma once
#include <Windows.h>
#include <tuple>

class Win32Window
{
    class Impl *m_impl = nullptr;

public:
    Win32Window();
    ~Win32Window();
    HWND Create(int w, int h, const char *title);
    HWND GetHandle() const;
    std::tuple<int, int> GetSize() const;
    bool HasFocus() const;
    std::tuple<int, int> GetCursorPosition() const;
};
