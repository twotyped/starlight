#pragma once

#include <starlight/starlight.h>
#include <string>

namespace starlight
{

class Window {
public:
    virtual ~Window() = default;

    virtual bool Initialize(const slWindowInstanceDesc& desc) = 0;
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() const = 0;
    virtual void RequestClose() = 0;

    virtual void* GetNativeHandle() const = 0; // HWND on Win32, NSWindow on Cocoa, etc.
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

protected:
    std::string m_title;
    uint32_t m_width{0};
    uint32_t m_height{0};
    bool m_shouldClose{false};
};

} // namespace starlight