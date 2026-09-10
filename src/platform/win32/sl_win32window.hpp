#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "core/sl_window.hpp"

namespace starlight
{

class Win32Window : public Window {
public:
    Win32Window();
    ~Win32Window() override;

    // starlight::Window Interface
    bool Initialize(const slWindowInstanceDesc& desc) override;
    void PollEvents() override;
    bool ShouldClose() const override { return m_shouldClose; }
    void RequestClose() override { m_shouldClose = true; }

    void* GetNativeHandle() const override { return static_cast<void*>(m_hwnd); }
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }

    HINSTANCE GetHInstance() const { return m_hinstance; }

private:
    // Static Win32 message procedure router
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd{NULL};
    HINSTANCE m_hinstance{NULL};
    const char* m_className{"StarlightWindowClass"};
    bool m_classRegistered{false};
};

} // namespace starlight