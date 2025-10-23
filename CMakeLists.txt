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

# Common dependencies
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)

# Platform-specific windowing
if(UNIX AND NOT APPLE)
    # Linux: Use Wayland
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
    pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
    pkg_check_modules(EGL REQUIRED egl)
    pkg_check_modules(GLES2 REQUIRED glesv2)
    
    # Wayland protocol generation
    find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
    pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
    
    message(STATUS "Building with Wayland backend")
else()
    # macOS and Windows: Use GLFW
    find_package(glfw3 REQUIRED)
    if(NOT glfw3_FOUND)
        message(FATAL_ERROR "GLFW3 is required. Please install GLFW3.")
    endif()
    message(STATUS "Building with GLFW backend")
endif()

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

# Platform-specific dependencies
if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    message(STATUS "Building for Linux with Wayland + NanoVG + OpenGL")
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    message(STATUS "Building for macOS with GLFW + NanoVG + OpenGL")
elseif(WIN32)
    message(STATUS "Building for Windows with GLFW + NanoVG + OpenGL")
endif()

# Generate Wayland protocol files for Linux
if(UNIX AND NOT APPLE)
    # XDG Shell protocol
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

    # XDG Decoration protocol for window decorations
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
endif()

# Flux library sources
set(FLUX_SOURCES
    src/Core/Application.cpp
    src/Core/Window.cpp
    src/Graphics/Path.cpp
    src/Graphics/NanoVGRenderer.cpp
    src/Graphics/NanoVGRenderContext.cpp
    src/Views/SVG.cpp
    src/Views/NanoSVG.cpp
)

# Platform-specific sources
if(UNIX AND NOT APPLE)
    list(APPEND FLUX_SOURCES
        src/Platform/WaylandWindow.cpp
        ${XDG_SHELL_CLIENT_HEADER}
        ${XDG_SHELL_PRIVATE_CODE}
        ${XDG_DECORATION_CLIENT_HEADER}
        ${XDG_DECORATION_PRIVATE_CODE}
    )
else()
    list(APPEND FLUX_SOURCES
        src/Platform/GLFWWindow.cpp
    )
endif()

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

# Define OpenGL version for NanoVG based on platform
if(APPLE)
    # macOS: Use OpenGL 2 (OpenGL ES 2 headers not readily available)
    target_compile_definitions(nanovg PRIVATE NANOVG_GL2 GL_SILENCE_DEPRECATION FONS_USE_FREETYPE)
elseif(UNIX)
    # Linux: Use OpenGL ES 2
    target_compile_definitions(nanovg PRIVATE NANOVG_GLES2 FONS_USE_FREETYPE)
elseif(WIN32)
    # Windows: Use OpenGL 2
    target_compile_definitions(nanovg PRIVATE NANOVG_GL2 FONS_USE_FREETYPE)
endif()

# Link OpenGL to NanoVG
if(UNIX AND NOT APPLE)
    target_link_libraries(nanovg PUBLIC OpenGL::GL)
elseif(APPLE)
    target_link_libraries(nanovg PUBLIC ${OPENGL_LIBRARY})
elseif(WIN32)
    target_link_libraries(nanovg PUBLIC opengl32)
endif()

add_library(flux STATIC ${FLUX_SOURCES})

target_include_directories(flux PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${NANOSVG_SOURCE_DIR}>
    $<BUILD_INTERFACE:${STB_SOURCE_DIR}>
)

# Platform-specific includes for Wayland
if(UNIX AND NOT APPLE)
    target_include_directories(flux PRIVATE
        ${WAYLAND_CLIENT_INCLUDE_DIRS}
        ${WAYLAND_EGL_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${GLES2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

target_link_libraries(flux PUBLIC
    Threads::Threads
    Freetype::Freetype
    nanovg
)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(flux PUBLIC
        ${WAYLAND_CLIENT_LIBRARIES}
        ${WAYLAND_EGL_LIBRARIES}
        ${EGL_LIBRARIES}
        ${GLES2_LIBRARIES}
    )
else()
    target_link_libraries(flux PUBLIC
        glfw
    )
endif()

# Additional platform-specific linking (beyond windowing)
if(APPLE)
    target_link_libraries(flux PUBLIC ${OPENGL_LIBRARY})
elseif(WIN32)
    target_link_libraries(flux PUBLIC opengl32)
endif()

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