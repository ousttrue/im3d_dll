#include "win32_window.h"
#include <imgui/imgui.h>
#include <string>

static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam);

static HWND InitWindow(int &_width_, int &_height_, const char *_title)
{
    static ATOM wndclassex = 0;
    if (wndclassex == 0)
    {
        WNDCLASSEX wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.style = CS_OWNDC; // | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(0);
        wc.lpszClassName = "Im3dTestApp";
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wndclassex = RegisterClassEx(&wc);
    }

    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    if (_width_ == -1 || _height_ == -1)
    {
        // auto size; get the dimensions of the primary screen area and subtract the non-client area
        RECT r;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
        _width_ = r.right - r.left;
        _height_ = r.bottom - r.top;

        RECT wr = {};
        AdjustWindowRectEx(&wr, dwStyle, FALSE, dwExStyle);
        _width_ -= wr.right - wr.left;
        _height_ -= wr.bottom - wr.top;
    }

    RECT r;
    r.top = 0;
    r.left = 0;
    r.bottom = _height_;
    r.right = _width_;
    AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle);
    auto hwnd = CreateWindowEx(
        dwExStyle,
        MAKEINTATOM(wndclassex),
        _title,
        dwStyle,
        0, 0,
        r.right - r.left, r.bottom - r.top,
        nullptr,
        nullptr,
        GetModuleHandle(0),
        nullptr);
    assert(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}

class Impl
{
    HWND m_hwnd = NULL;
    int m_width = 0;
    int m_height = 0;

public:
    Impl()
    {
    }
    ~Impl()
    {
        if (m_hwnd)
        {
            DestroyWindow(m_hwnd);
        }
    }
    HWND GetHandle() const { return m_hwnd; }
    void Resize(int w, int h)
    {
        m_width = w;
        m_height = h;
    }
    HWND Create(int w, int h, const char *title)
    {
        m_width = w;
        m_height = h;
        m_hwnd = InitWindow(m_width, m_height, title);
        return m_hwnd;
    }
    std::tuple<int, int> GetSize() const
    {
        return std::make_tuple(m_width, m_height);
    }
};
Impl *g_impl = nullptr;

static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
{
    ImGuiIO &imgui = ImGui::GetIO();
    // Example *im3d = g_Example;

    switch (_umsg)
    {
    case WM_SIZE:
    {
        auto w = (int)LOWORD(_lparam);
        auto h = (int)HIWORD(_lparam);
        g_impl->Resize(w, h);
        break;
    }
    /*
    case WM_SIZING:
    {
        RECT *r = (RECT *)_lparam;
        int w = (int)(r->right - r->left);
        int h = (int)(r->bottom - r->top);
        if (im3d->m_width != w || im3d->m_height != h)
        {
            im3d->m_width = w;
            im3d->m_height = h;
        }
        break;
    }
    */
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        imgui.MouseDown[0] = _umsg == WM_LBUTTONDOWN;
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        imgui.MouseDown[2] = _umsg == WM_MBUTTONDOWN;
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        imgui.MouseDown[1] = _umsg == WM_RBUTTONDOWN;
        break;
    case WM_MOUSEWHEEL:
        imgui.MouseWheel = (float)(GET_WHEEL_DELTA_WPARAM(_wparam)) / (float)(WHEEL_DELTA);
        break;
    case WM_MOUSEMOVE:
        imgui.MousePos.x = LOWORD(_lparam);
        imgui.MousePos.y = HIWORD(_lparam);
        break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        WPARAM vk = _wparam;
        UINT sc = (_lparam & 0x00ff0000) >> 16;
        bool e0 = (_lparam & 0x01000000) != 0;
        if (vk == VK_SHIFT)
        {
            vk = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);
        }
        switch (vk)
        {
        case VK_CONTROL:
            imgui.KeyCtrl = _umsg == WM_KEYDOWN;
            break;
        case VK_MENU:
            imgui.KeyAlt = _umsg == WM_KEYDOWN;
            break;
        case VK_LSHIFT:
        case VK_RSHIFT:
            imgui.KeyShift = _umsg == WM_KEYDOWN;
            break;
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        default:
            if (vk < 512)
            {
                imgui.KeysDown[vk] = _umsg == WM_KEYDOWN;
            }
            break;
        };
        return 0;
    }
    case WM_CHAR:
        if (_wparam > 0 && _wparam < 0x10000)
        {
            imgui.AddInputCharacter((unsigned short)_wparam);
        }
        return 0;
    case WM_PAINT:
        //IM3D_ASSERT(false); // should be suppressed by calling ValidateRect()
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0; // prevent DefWindowProc from destroying the window
    default:
        break;
    };
    return DefWindowProc(_hwnd, _umsg, _wparam, _lparam);
}

Win32Window::Win32Window()
: m_impl(new Impl)
{
    g_impl = m_impl;
}

Win32Window::~Win32Window()
{
}

HWND Win32Window::Create(int w, int h, const char *title)
{
    return m_impl->Create(w, h, title);
}

HWND Win32Window::GetHandle() const
{
    return m_impl->GetHandle();
}

std::tuple<int, int> Win32Window::GetSize() const
{
    return m_impl->GetSize();
}

bool Win32Window::HasFocus() const
{
    return m_impl->GetHandle() == GetFocus();
}

std::tuple<int, int> Win32Window::GetCursorPosition() const
{
    POINT p = {};
    GetCursorPos(&p);
    ScreenToClient(m_impl->GetHandle(), &p);
    return std::make_tuple(p.x, p.y);
}
