#include "sl_win32window.hpp"

namespace starlight
{

Win32Window::Win32Window() = default;

Win32Window::~Win32Window() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    if (m_classRegistered && m_hinstance) {
        UnregisterClassA(m_className, m_hinstance);
        m_classRegistered = false;
    }
}

bool Win32Window::Initialize(const slWindowInstanceDesc& desc) {
    m_hinstance = GetModuleHandle(NULL);
    m_width = desc.Width;
    m_height = desc.Height;
    m_title = desc.ApplicationName ? desc.ApplicationName : "Starlight Window";

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = Win32Window::StaticWndProc;
    wc.hInstance = m_hinstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = m_className;

    if (!RegisterClassExA(&wc)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }

    m_classRegistered = true;

    RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&windowRect, dwStyle, FALSE);

    m_hwnd = CreateWindowExA(
        0,
        m_className,
        m_title.c_str(),
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        m_hinstance,
        this // Passed to WM_NCCREATE / WM_CREATE in StaticWndProc
    );

    if (!m_hwnd) {
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    return true;
}

void Win32Window::PollEvents(NativeEventCallback callback) {
    MSG msg = {};
    // Non-blocking message pump
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        const bool handled = callback && callback(
            this,
            static_cast<uint32_t>(msg.message),
            static_cast<uintptr_t>(msg.wParam),
            static_cast<intptr_t>(msg.lParam)
        );

        if (msg.message == WM_QUIT) {
            m_shouldClose = true;
            continue;
        }

        if (handled) {
            ApplyHandledMessage(msg.message, msg.wParam, msg.lParam);
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

LRESULT CALLBACK Win32Window::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32Window* window = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCTA* pCreate = reinterpret_cast<CREATESTRUCTA*>(lParam);
        window = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);
        
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->m_hwnd = hwnd;
    } else {
        window = reinterpret_cast<Win32Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->HandleMessage(msg, wParam, lParam);
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

LRESULT Win32Window::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    ApplyHandledMessage(msg, wParam, lParam);

    switch (msg) {
        default:
            break;
    }

    return DefWindowProcA(m_hwnd, msg, wParam, lParam);
}

void Win32Window::ApplyHandledMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            m_shouldClose = true;
            break;

        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            break;
    }
}

} // namespace starlight