# ✨ Starlight
### A cross-platform Hardware Abstraction Layer that unifies Vulkan, DirectX, Metal, and native windowing into a single explicit, low-level API.

**Starlight** bridges the painful gap between graphics API and rendering engine development. It eliminates thousands of lines of fragile platform-dependent window creation and abstracting graphics APIs such as *Vulkan, DirectX,* and *Metal* into one coherent framework without forcing a rendering engine's structure or hiding explicit control.

---

### 🛑 What Starlight Is (And Is Not)
* **Starlight IS**: A lean **Hardware Abstraction Layer** (HAL) and Graphics API. It manages platform-dependent window creation, event loops, surface initialization, swapchain initialization while unifying the extensive frameworks of Vulkan, DirectX, OpenGL, and Metal into one unified framework.
* **Starlight IS NOT**: A rendering engine, scene graph, or material framework. It does **not** handle high-level input mapping, audio, physics, or lighting pipelines. It exists purely to give engine developers total control over low-level graphics execution without the cross-platform setup tax.

---

### ⚡ Key Features

* **Vulkan-Inspired Descriptor Design:** Explicit `sType`/`pNext` style configuration structures give developers precise control over backend behavior and forward binary compatibility.
* **Deferred Asset & Surface Creation:** Descriptors build configuration offline. Native GPU resources (`VkDevice`, `ID3D12Device`, swapchains) are only allocated when bound to an active window surface—preventing validation cascades and race conditions.
* **Unified Native Event Pump:** Abstracted, thread-safe Win32, Cocoa, and X11/Wayland event loops that require explicit acknowledgment, ensuring clean GPU flushes before native window teardown.
* **Zero-Overhead Explicit Control:** Automatic defaults and fallbacks can be explicitly configured or toggled off completely for zero-overhead production builds.
* **C99 Public ABI with C++ Implementation:** Pure C header interface (`extern "C"`) with opaque handles for effortless FFI bindings (Rust, Zig, C#), backed by a modern C++ internal codebase.
* **Built-In Graphics Debugging Hooks:** Native integration for validation layers, resource leak tracking, and GPU diagnostics across all backends.

---

### 🛠️ Building & Requirements
Starlight is built using pure C++, CMake, and vcpkg. To compile the code (as the raw binaries are not yet widely available):
```bash
# Clone the repository
git clone https://github.com/twotyped/starlight.git
cd starlight

# Install vcpkg, then set VCPKG_ROOT to its directory. For example, in
# PowerShell, if vcpkg is at C:\src\vcpkg:
$env:VCPKG_ROOT = 'C:\src\vcpkg'

# CMake will now load vcpkg.json through the vcpkg toolchain and install its
# declared dependencies automatically.
cmake --preset vcpkg

# Build the Debug configuration
cmake --build --preset vcpkg
```

For a Release build, select the configuration supported by your generator:

```powershell
$env:VCPKG_ROOT = 'C:\src\vcpkg'
cmake --preset vcpkg
cmake --build build-vcpkg --config Release
```

On POSIX shells, set the equivalent variable before configuring:

```sh
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset vcpkg
```

The `CMakePresets.json` file contains the default configuration used by the
commands above. Dependencies should be added to `vcpkg.json`; do not run
`vcpkg install` separately for this project.

---

### 📄 License
Starlight is free, open-source software licensed entirely under the MIT License (read LICENSE).