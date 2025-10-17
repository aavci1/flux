cmake_minimum_required(VERSION 3.25)
project(flux VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common dependencies
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)

# GLFW for cross-platform windowing
find_package(glfw3 REQUIRED)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW3 is required. Please install GLFW3.")
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
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanosvg/nanosvg.h")
    message(STATUS "NanoSVG found - building with SVG support")
    set(FLUX_HAS_NANOSVG ON)
    set(NANOSVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanosvg")
else()
    message(FATAL_ERROR "NanoSVG is required. Please download nanosvg.h to third_party/nanosvg/")
endif()

# Platform-specific dependencies
if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    message(STATUS "Building for Linux with GLFW + NanoVG + OpenGL")
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    message(STATUS "Building for macOS with GLFW + NanoVG + OpenGL")
elseif(WIN32)
    message(STATUS "Building for Windows with GLFW + NanoVG + OpenGL")
endif()

# Flux library
    set(FLUX_SOURCES
        src/Core/Application.cpp
        src/Core/Window.cpp
        src/Platform/GLFWWindow.cpp
        src/Graphics/NanoVGRenderer.cpp
        src/Graphics/NanoVGRenderContext.cpp
        src/Views/SVG.cpp
        src/Views/NanoSVG.cpp
    )

# Build NanoVG
add_library(nanovg STATIC
    ${NANOVG_SOURCE_DIR}/src/nanovg.c
)

target_include_directories(nanovg PUBLIC
    ${NANOVG_SOURCE_DIR}/src
)

# Set C standard for NanoVG
set_target_properties(nanovg PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

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
)

target_link_libraries(flux PUBLIC
    Threads::Threads
    Freetype::Freetype
    glfw
    nanovg
)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(flux PUBLIC OpenGL::GL)
elseif(APPLE)
    target_link_libraries(flux PUBLIC ${OPENGL_LIBRARY})
elseif(WIN32)
    target_link_libraries(flux PUBLIC opengl32)
endif()

target_compile_options(flux PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-g -O0>
)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Examples
option(BUILD_EXAMPLES "Build example applications" ON)

if(BUILD_EXAMPLES)
    add_executable(hello_world examples/01-hello-world/main.cpp)
    target_link_libraries(hello_world PRIVATE flux)

    add_executable(stack_demo examples/02-stack-alignment-demo/main.cpp)
    target_link_libraries(stack_demo PRIVATE flux)

    add_executable(counter examples/03-counter/main.cpp)
    target_link_libraries(counter PRIVATE flux)

    add_executable(todo_app examples/04-todo-app/main.cpp)
    target_link_libraries(todo_app PRIVATE flux)

    add_executable(colors_and_theming examples/05-colors-and-theming/main.cpp)
    target_link_libraries(colors_and_theming PRIVATE flux)

    add_executable(media_player examples/06-media-player/main.cpp)
    target_link_libraries(media_player PRIVATE flux)

    add_executable(dashboard examples/08-dashboard/main.cpp)
    target_link_libraries(dashboard PRIVATE flux)

    add_executable(flexbox_demo examples/09-flexbox-demo/main.cpp)
    target_link_libraries(flexbox_demo PRIVATE flux)

    add_executable(justify_content_demo examples/10-justify-content-demo/main.cpp)
    target_link_libraries(justify_content_demo PRIVATE flux)

        add_executable(automotive_dashboard examples/11-automotive-dashboard/main.cpp)
        target_link_libraries(automotive_dashboard PRIVATE flux)

        add_executable(nanovg_demo examples/12-nanovg-demo/main.cpp)
        target_link_libraries(nanovg_demo PRIVATE flux)

        add_executable(resize_test examples/13-resize-test/main.cpp)
        target_link_libraries(resize_test PRIVATE flux)

        add_executable(svg_demo examples/14-svg-demo/main.cpp)
        target_link_libraries(svg_demo PRIVATE flux)

        add_executable(render_context_demo examples/15-render-context-demo/main.cpp)
        target_link_libraries(render_context_demo PRIVATE flux)

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