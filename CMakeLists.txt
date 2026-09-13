cmake_minimum_required(VERSION 3.20)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/vcpkg")
    message(STATUS "Bootstrapping vcpkg...")
    if(WIN32)
        execute_process(COMMAND bootstrap-vcpkg.bat WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg")
    else()
        execute_process(COMMAND ./bootstrap-vcpkg.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg")
    endif()
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")

project(starlight VERSION 0.1 LANGUAGES CXX)

include_directories(include)

add_subdirectory(src)
add_subdirectory(examples)