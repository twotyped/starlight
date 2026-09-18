add_library(starlight STATIC
    starlight.cpp
    core/sl_window.hpp
    core/sl_internal.hpp
    platform/win32/sl_win32window.hpp
    platform/win32/sl_win32window.cpp
    core/api/vulkan/vk_internal.hpp
)

find_package(Vulkan REQUIRED)

target_include_directories(starlight
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(starlight PUBLIC cxx_std_20)

target_link_libraries(starlight PRIVATE Vulkan::Vulkan)

if(WIN32)
    target_link_libraries(starlight PRIVATE user32 gdi32)
endif()