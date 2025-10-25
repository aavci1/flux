cmake_minimum_required(VERSION 3.25)

# Force Clang compiler for better C++23 support
find_program(CLANG_EXECUTABLE NAMES clang)
find_program(CLANGXX_EXECUTABLE NAMES clang++)

if(NOT CMAKE_C_COMPILER AND CLANG_EXECUTABLE)
    set(CMAKE_C_COMPILER ${CLANG_EXECUTABLE})
endif()

if(NOT CMAKE_CXX_COMPILER AND CLANGXX_EXECUTABLE)
    set(CMAKE_CXX_COMPILER ${CLANGXX_EXECUTABLE})
endif()

project(flux VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-w)

# Verify we're on Linux
if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "Flux currently supports Linux/Wayland only. Cross-platform support is not a goal at this time.")
endif()

# Dependencies
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)

# Wayland dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
pkg_check_modules(WAYLAND_CURSOR REQUIRED wayland-cursor)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES2 REQUIRED glesv2)

# Wayland protocol generation
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

message(STATUS "Building Flux for Linux with Wayland + NanoVG + OpenGL ES 2")

# NanoVG for high-quality 2D graphics rendering
# Check if NanoVG submodule exists
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanovg/src/nanovg.c")
    message(STATUS "NanoVG submodule found - building with NanoVG backend support")
    set(FLUX_HAS_NANOVG ON)
    set(NANOVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanovg")
else()
    message(FATAL_ERROR "NanoVG submodule is required. Please run: ./scripts/setup-nanovg.sh")
endif()

# NanoSVG for SVG parsing
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src/nanosvg.h")
    message(STATUS "NanoSVG found - building with SVG support")
    set(FLUX_HAS_NANOSVG ON)
    set(NANOSVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src")
else()
    message(FATAL_ERROR "NanoSVG is required. Please ensure nanosvg submodule is initialized.")
endif()

# STB for single-file libraries (provides stb_truetype.h, stb_image.h, etc.)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/stb/stb_truetype.h")
    message(STATUS "STB found - building with STB support")
    set(FLUX_HAS_STB ON)
    set(STB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/stb")
else()
    message(FATAL_ERROR "STB is required. Please ensure stb submodule is initialized.")
endif()

# Generate Wayland protocol files
# XDG Shell protocol (window management)
set(XDG_SHELL_PROTOCOL "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
set(XDG_SHELL_CLIENT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h")
set(XDG_SHELL_PRIVATE_CODE "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c")

add_custom_command(
    OUTPUT ${XDG_SHELL_CLIENT_HEADER}
    COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_PROTOCOL} ${XDG_SHELL_CLIENT_HEADER}
    DEPENDS ${XDG_SHELL_PROTOCOL}
    VERBATIM
)

add_custom_command(
    OUTPUT ${XDG_SHELL_PRIVATE_CODE}
    COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_PROTOCOL} ${XDG_SHELL_PRIVATE_CODE}
    DEPENDS ${XDG_SHELL_PROTOCOL}
    VERBATIM
)

# XDG Decoration protocol (window decorations)
set(XDG_DECORATION_PROTOCOL "${WAYLAND_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml")
set(XDG_DECORATION_CLIENT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xdg-decoration-client-protocol.h")
set(XDG_DECORATION_PRIVATE_CODE "${CMAKE_CURRENT_BINARY_DIR}/xdg-decoration-protocol.c")

add_custom_command(
    OUTPUT ${XDG_DECORATION_CLIENT_HEADER}
    COMMAND ${WAYLAND_SCANNER} client-header ${XDG_DECORATION_PROTOCOL} ${XDG_DECORATION_CLIENT_HEADER}
    DEPENDS ${XDG_DECORATION_PROTOCOL}
    VERBATIM
)

add_custom_command(
    OUTPUT ${XDG_DECORATION_PRIVATE_CODE}
    COMMAND ${WAYLAND_SCANNER} private-code ${XDG_DECORATION_PROTOCOL} ${XDG_DECORATION_PRIVATE_CODE}
    DEPENDS ${XDG_DECORATION_PROTOCOL}
    VERBATIM
)

# Flux library sources
set(FLUX_SOURCES
    # Core
    src/Core/Application.cpp
    src/Core/Window.cpp
    src/Core/KeyboardInputHandler.cpp
    src/Core/MouseInputHandler.cpp
    src/Core/FocusState.cpp
    src/Core/ShortcutManager.cpp
    src/Core/PlatformWindowFactory.cpp
    
    # Graphics
    src/Graphics/Path.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/NanoVGRenderer.cpp
    src/Graphics/NanoVGRenderContext.cpp
    
    # Views
    src/Views/SVG.cpp
    src/Views/NanoSVG.cpp
    
    # Platform (Wayland)
    src/Platform/WaylandWindow.cpp
    
    # Generated Wayland protocols
    ${XDG_SHELL_CLIENT_HEADER}
    ${XDG_SHELL_PRIVATE_CODE}
    ${XDG_DECORATION_CLIENT_HEADER}
    ${XDG_DECORATION_PRIVATE_CODE}
)

# Build NanoVG
add_library(nanovg STATIC
    ${NANOVG_SOURCE_DIR}/src/nanovg.c
    ${NANOVG_SOURCE_DIR}/src/nanovg_gl.c
    ${NANOVG_SOURCE_DIR}/src/nanovg_gl_utils.c
)

target_include_directories(nanovg PUBLIC
    ${NANOVG_SOURCE_DIR}/src
    ${STB_SOURCE_DIR}
)

# Link Freetype for font rendering
target_link_libraries(nanovg PUBLIC
    Freetype::Freetype
)

# Set C standard for NanoVG
set_target_properties(nanovg PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Define OpenGL ES 2 for NanoVG (Linux/Wayland)
target_compile_definitions(nanovg PRIVATE NANOVG_GLES2 FONS_USE_FREETYPE)

# Link OpenGL to NanoVG
target_link_libraries(nanovg PUBLIC OpenGL::GL)

add_library(flux STATIC ${FLUX_SOURCES})

target_include_directories(flux PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${NANOSVG_SOURCE_DIR}>
    $<BUILD_INTERFACE:${STB_SOURCE_DIR}>
)

# Wayland-specific includes
target_include_directories(flux PRIVATE
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLES2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(flux PUBLIC
    Threads::Threads
    Freetype::Freetype
    nanovg
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_EGL_LIBRARIES}
    ${WAYLAND_CURSOR_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES2_LIBRARIES}
)

target_compile_options(flux PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-g -O0>
)

# Handle optimization for production builds
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    # Add additional optimization flags for production builds
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native")
endif()

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Examples
option(BUILD_EXAMPLES "Build example applications" ON)

if(BUILD_EXAMPLES)
    add_executable(hello_world examples/01-hello-world/main.cpp)
    target_link_libraries(hello_world PRIVATE flux)


    add_executable(counter examples/03-counter/main.cpp)
    target_link_libraries(counter PRIVATE flux)

    add_executable(todo_app examples/04-todo-app/main.cpp)
    target_link_libraries(todo_app PRIVATE flux)

    add_executable(colors_and_theming examples/05-colors-and-theming/main.cpp)
    target_link_libraries(colors_and_theming PRIVATE flux)



    add_executable(flexbox_demo examples/09-flexbox-demo/main.cpp)
    target_link_libraries(flexbox_demo PRIVATE flux)

    add_executable(justify_content_demo examples/10-justify-content-demo/main.cpp)
    target_link_libraries(justify_content_demo PRIVATE flux)

    add_executable(automotive_dashboard examples/11-automotive-dashboard/main.cpp)
    target_link_libraries(automotive_dashboard PRIVATE flux)



    add_executable(svg_demo examples/14-svg-demo/main.cpp)
    target_link_libraries(svg_demo PRIVATE flux)

    add_executable(render_context_demo examples/15-render-context-demo/main.cpp)
    target_link_libraries(render_context_demo PRIVATE flux)

    add_executable(login_manager examples/17-login-manager/main.cpp)
    target_link_libraries(login_manager PRIVATE flux)

    add_executable(clock examples/19-clock/main.cpp)
    target_link_libraries(clock PRIVATE flux)

add_executable(grid_demo examples/22-grid-demo/main.cpp)
target_link_libraries(grid_demo PRIVATE flux)

add_executable(spacer_grid_demo examples/23-spacer-grid-demo/main.cpp)
target_link_libraries(spacer_grid_demo PRIVATE flux)

add_executable(calculator examples/24-calculator/main.cpp)
target_link_libraries(calculator PRIVATE flux)

add_executable(cursor_demo examples/25-cursor-demo/main.cpp)
target_link_libraries(cursor_demo PRIVATE flux)

add_executable(declarative_cursor examples/26-declarative-cursor/main.cpp)
target_link_libraries(declarative_cursor PRIVATE flux)

add_executable(focus_keyboard_demo examples/27-focus-keyboard-demo/main.cpp)
target_link_libraries(focus_keyboard_demo PRIVATE flux)

add_executable(event_system_demo examples/28-event-system-demo/main.cpp)
target_link_libraries(event_system_demo PRIVATE flux)

add_executable(color_picker examples/29-color-picker/main.cpp)
target_link_libraries(color_picker PRIVATE flux)

add_executable(component_showcase examples/31-component-showcase/main.cpp)
target_link_libraries(component_showcase PRIVATE flux)

add_executable(label_position_demo examples/32-label-position-demo/main.cpp)
target_link_libraries(label_position_demo PRIVATE flux)

add_executable(scroll_area_demo examples/33-scroll-area-demo/main.cpp)
target_link_libraries(scroll_area_demo PRIVATE flux)

endif()

# Installation rules
install(TARGETS flux
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/Flux
    DESTINATION include
)

install(FILES include/Flux.hpp
    DESTINATION include
)