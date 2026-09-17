#pragma once

#include <starlight/starlight.h>
#include <string>

namespace starlight
{

class Window;
using NativeEventCallback = bool (*)(
    Window* window,
    uint32_t message,
    uintptr_t wParam,
    intptr_t lParam
);

class Window {
public:
    virtual ~Window() = default;

    virtual bool Initialize(const slWindowInstanceDesc& pDesc, slWindowSurface surface) = 0;
    virtual void PollEvents(NativeEventCallback callback) = 0;
    virtual bool ShouldClose() const = 0;
    virtual void RequestClose() = 0;

    void SetPublicHandle(slWindow handle) { m_publicHandle = handle; }
    slWindow GetPublicHandle() const { return m_publicHandle; }

    virtual void* GetNativeHandle() const = 0; // HWND on Win32, NSWindow on Cocoa, etc.
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual slWindowSurface GetSurfaceBuffer() { return m_surface; }

protected:
    std::string m_title;
    uint32_t m_width{0};
    uint32_t m_height{0};
    bool m_shouldClose{false};
    slWindow m_publicHandle{nullptr};
    slWindowSurface m_surface{nullptr};
};

} // namespace starlight