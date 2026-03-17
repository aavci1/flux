cmake_minimum_required(VERSION 3.25)

project(flux VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-w)

# Dependencies
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)

# SDL3: try system install first, fetch from GitHub as fallback
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found on system, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.0
        GIT_SHALLOW TRUE
    )
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
endif()

# NanoVG for high-quality 2D vector graphics
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanovg/src/nanovg.c")
    message(STATUS "NanoVG submodule found")
    set(NANOVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanovg")
else()
    message(FATAL_ERROR "NanoVG submodule is required. Please run: ./scripts/setup-nanovg.sh")
endif()

# NanoSVG for SVG parsing
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src/nanosvg.h")
    message(STATUS "NanoSVG found")
    set(NANOSVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src")
else()
    message(FATAL_ERROR "NanoSVG is required. Please ensure nanosvg submodule is initialized.")
endif()

# STB single-file libraries
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/stb/stb_truetype.h")
    message(STATUS "STB found")
    set(STB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/stb")
else()
    message(FATAL_ERROR "STB is required. Please ensure stb submodule is initialized.")
endif()

# Flux library sources
set(FLUX_SOURCES
    # Core
    src/Core/Runtime.cpp
    src/Core/Window.cpp
    src/Core/KeyboardInputHandler.cpp
    src/Core/MouseInputHandler.cpp
    src/Core/FocusState.cpp
    src/Core/ShortcutManager.cpp
    src/Core/PlatformWindowFactory.cpp
    src/Core/Demangle.cpp
    src/Core/Element.cpp

    # Graphics
    src/Graphics/Path.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/NanoVGRenderer.cpp
    src/Graphics/NanoVGRenderContext.cpp

    # Views
    src/Views/SVG.cpp
    src/Views/NanoSVG.cpp

    # Platform (SDL3)
    src/Platform/SDLWindow.cpp
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

target_link_libraries(nanovg PUBLIC
    Freetype::Freetype
    OpenGL::GL
)

set_target_properties(nanovg PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

target_compile_definitions(nanovg PRIVATE NANOVG_GL2 FONS_USE_FREETYPE)

# Flux static library
add_library(flux STATIC ${FLUX_SOURCES})

target_include_directories(flux PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${NANOSVG_SOURCE_DIR}>
    $<BUILD_INTERFACE:${STB_SOURCE_DIR}>
)

target_link_libraries(flux PUBLIC
    Threads::Threads
    Freetype::Freetype
    OpenGL::GL
    nanovg
    SDL3::SDL3
)

target_compile_options(flux PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-g -O0>
)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native")
    endif()
endif()

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

    add_executable(passwords examples/passwords/main.cpp)
    target_link_libraries(passwords PRIVATE flux)
endif()

# --- Tests ---
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
    enable_testing()

    add_executable(flux_tests
        tests/test_types.cpp
        tests/test_property.cpp
        tests/test_element.cpp
        tests/test_layout.cpp
    )
    target_link_libraries(flux_tests PRIVATE flux Catch2::Catch2WithMain)

    include(CTest)
    include(Catch)
    catch_discover_tests(flux_tests)
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
