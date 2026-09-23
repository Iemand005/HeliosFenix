cmake_minimum_required(VERSION 3.15)
project(Helios)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KISSFFT_TEST OFF CACHE BOOL "Disable KissFFT tests" FORCE)

set(KISSFFT_STATIC ON CACHE BOOL "Build static library" FORCE)

set(FE_INCLUDE_OPENVR OFF CACHE BOOL "Include OpenVR" FORCE)

set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../FenixEngine")

set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "Use dynamic MSVC runtime to match rest of project" FORCE)

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
endif()


if(NOT TARGET Engine)
    message(STATUS "Engine target not found. Looking for engine source...")

    if(EXISTS "${ENGINE_DIR}/CMakeLists.txt")
        add_subdirectory("${ENGINE_DIR}" FenixEngine)
    else()
        message(FATAL_ERROR "FenixEngine not found!")
    endif()
endif()

set(IMGUI_DIR ${ENGINE_DIR}/external/imgui)

file(GLOB_RECURSE IMGUI_SOURCE_FILES
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdlgpu3.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
	${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
)

if(EMSCRIPTEN)
    add_executable(Helios
        ${IMGUI_SOURCE_FILES}
        main.cpp
    )
else()
    add_executable(Helios
        WIN32
        ${IMGUI_SOURCE_FILES}
        main.cpp
    )
endif()

target_include_directories(Helios PRIVATE
	include
	${ENGINE_DIR}
	${ENGINE_DIR}/include
	${ENGINE_DIR}/external
	${ENGINE_DIR}/external/imgui
	${ENGINE_DIR}/external/kissfft
	${ENGINE_DIR}/external/SDL/include
	${ENGINE_DIR}/src/Engine
)

target_link_libraries(Helios PRIVATE
	Engine
)

target_precompile_headers(Helios PRIVATE
	"$<$<COMPILE_LANGUAGE:CXX>:${ENGINE_DIR}/src/Engine/pch.hpp>"
)

if(NOT WIN32 AND NOT EMSCRIPTEN)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
target_include_directories(Helios PRIVATE ${PIPEWIRE_INCLUDE_DIRS})
target_link_libraries(Helios PRIVATE ${PIPEWIRE_LIBRARIES})
target_compile_options(Helios PRIVATE ${PIPEWIRE_CFLAGS_OTHER})

find_package(X11 REQUIRED)

target_link_libraries(Helios PRIVATE X11::X11)
endif()

set_property(TARGET Helios PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
	target_compile_options(Helios PRIVATE $<$<CONFIG:Debug>:/Zi>)

	target_link_options(Helios PRIVATE $<$<CONFIG:Debug>:/DEBUG>)

	add_compile_options(/RTC1)
endif()


include(${CMAKE_SOURCE_DIR}/EmscriptenConfig.cmake)
if(EMSCRIPTEN)
	setup_emscripten_target(Helios)
endif()