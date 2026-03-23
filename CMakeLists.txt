cmake_minimum_required(VERSION 3.25)

project(flux VERSION 2.0.0 LANGUAGES CXX C OBJCXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)

include(FetchContent)

# SDL3: Linux/Windows only (macOS uses native Cocoa)
if(NOT APPLE)
    find_package(SDL3 QUIET)
    if(NOT SDL3_FOUND)
        message(STATUS "SDL3 not found on system, fetching from GitHub...")
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
endif()

# Pin commits match former submodules; override with -DFLUX_*_GIT_TAG=...
set(FLUX_NANOSVG_GIT_REPOSITORY "https://github.com/memononen/nanosvg.git" CACHE STRING "NanoSVG Git repository")
set(FLUX_NANOSVG_GIT_TAG "66579081d84b613daa49a64f76357ce65925e13b" CACHE STRING "NanoSVG Git tag or commit")
set(FLUX_STB_GIT_REPOSITORY "https://github.com/nothings/stb.git" CACHE STRING "stb Git repository")
set(FLUX_STB_GIT_TAG "fede005abaf93d9d7f3a679d1999b2db341b360f" CACHE STRING "stb Git tag or commit")

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src/nanosvg.h")
    set(NANOSVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src")
    message(STATUS "NanoSVG: using local third_party/nanosvg")
else()
    FetchContent_Declare(flux_nanosvg
        GIT_REPOSITORY ${FLUX_NANOSVG_GIT_REPOSITORY}
        GIT_TAG ${FLUX_NANOSVG_GIT_TAG}
        GIT_SUBMODULES ""
    )
    message(STATUS "NanoSVG: fetching ${FLUX_NANOSVG_GIT_TAG}")
    FetchContent_MakeAvailable(flux_nanosvg)
    set(NANOSVG_SOURCE_DIR "${flux_nanosvg_SOURCE_DIR}/src")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/stb/stb_truetype.h")
    set(STB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/stb")
    message(STATUS "stb: using local third_party/stb")
else()
    FetchContent_Declare(flux_stb
        GIT_REPOSITORY ${FLUX_STB_GIT_REPOSITORY}
        GIT_TAG ${FLUX_STB_GIT_TAG}
        GIT_SUBMODULES ""
    )
    message(STATUS "stb: fetching ${FLUX_STB_GIT_TAG}")
    FetchContent_MakeAvailable(flux_stb)
    set(STB_SOURCE_DIR "${flux_stb_SOURCE_DIR}")
endif()

# libtess2 for path tessellation
FetchContent_Declare(libtess2
    GIT_REPOSITORY https://github.com/memononen/libtess2.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libtess2)
if(NOT TARGET libtess2)
    add_library(libtess2 STATIC
        ${libtess2_SOURCE_DIR}/Source/bucketalloc.c
        ${libtess2_SOURCE_DIR}/Source/dict.c
        ${libtess2_SOURCE_DIR}/Source/geom.c
        ${libtess2_SOURCE_DIR}/Source/mesh.c
        ${libtess2_SOURCE_DIR}/Source/priorityq.c
        ${libtess2_SOURCE_DIR}/Source/sweep.c
        ${libtess2_SOURCE_DIR}/Source/tess.c
    )
endif()
set_target_properties(libtess2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ""
)
target_include_directories(libtess2 PUBLIC
    $<BUILD_INTERFACE:${libtess2_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# NanoVG includes <stb/stb_image.h>; nothings/stb ships headers as stb_*.h at repo root
set(FLUX_STB_SHIM_DIR "${CMAKE_BINARY_DIR}/flux_stb_shim")
file(MAKE_DIRECTORY "${FLUX_STB_SHIM_DIR}/stb")
file(GLOB _flux_stb_shim_headers "${STB_SOURCE_DIR}/stb_*.h")
foreach(_flux_stb_shim_h IN LISTS _flux_stb_shim_headers)
    get_filename_component(_flux_stb_shim_name "${_flux_stb_shim_h}" NAME)
    configure_file("${_flux_stb_shim_h}" "${FLUX_STB_SHIM_DIR}/stb/${_flux_stb_shim_name}" COPYONLY)
endforeach()

# Flux library sources
set(FLUX_SOURCES
    # Core
    src/Core/Application.cpp
    src/Core/Window.cpp
    src/Core/KeyboardInputHandler.cpp
    src/Core/MouseInputHandler.cpp
    src/Core/FocusState.cpp
    src/Core/ShortcutManager.cpp

    # Layout
    src/Layout/LayoutEngine.cpp
)

list(APPEND FLUX_SOURCES
    src/Platform/PlatformRegistry.cpp
    src/Platform/EventLoopWake.cpp
    src/Core/Demangle.cpp
    src/Core/Element.cpp
    src/Graphics/FontProvider.cpp
    src/Core/ResourceManager.cpp

    # Graphics
    src/Graphics/Path.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/CommandCompiler.cpp
    src/Graphics/GPURendererBackend.cpp
    src/Graphics/Atlas.cpp
    src/Graphics/GlyphAtlas.cpp
    src/Graphics/PathFlattener.cpp
    src/Graphics/ImageCache.cpp
    src/Graphics/GPURenderContext.cpp
    src/Platform/GPUPlatformRenderer.cpp

    # Testing
    src/Testing/ScreenCaptureImpl.cpp

    # Utils
    src/Utils/SVGUtils.cpp

    # Views
    src/Views/SVG.cpp
    src/Views/NanoSVG.cpp
)

if(APPLE)
    list(APPEND FLUX_SOURCES
        src/Platform/PlatformRegistryApple.mm
        src/Platform/MacWindowFactory.mm
        src/Platform/AppleClipboard.mm
        src/Platform/AppleFontResolver.mm
        src/Platform/AppleEventLoopWake.mm
        src/Platform/ApplePathUtil.mm
        src/Platform/MacWindow.mm
        src/Platform/AppleMemoryFootprint.mm
    )
else()
    list(APPEND FLUX_SOURCES
        src/Platform/MemoryFootprintStub.cpp
        src/Platform/PlatformRegistrySDL.cpp
        src/Platform/SDLWindowFactory.cpp
        src/Platform/SDLClipboard.cpp
        src/Platform/SDLEventLoopWake.cpp
        src/Platform/SDLWindow.cpp
    )
    if(UNIX)
        list(APPEND FLUX_SOURCES
            src/Platform/LinuxFontResolver.cpp
            src/Platform/LinuxPathUtil.cpp
        )
    else()
        list(APPEND FLUX_SOURCES
            src/Platform/StubFontResolver.cpp
            src/Platform/StubPathUtil.cpp
        )
    endif()
endif()

# GPU backend sources
list(APPEND FLUX_SOURCES src/GPU/DeviceFactory.cpp)

if(APPLE)
    list(APPEND FLUX_SOURCES src/GPU/Metal/MetalDevice.mm)
endif()

# Vulkan: Linux/Windows
set(FLUX_VULKAN_AVAILABLE OFF)
if(NOT APPLE)
    find_package(Vulkan QUIET)
    if(NOT Vulkan_FOUND)
        find_path(Vulkan_INCLUDE_DIR vulkan/vulkan.h HINTS /opt/homebrew/include /usr/local/include)
        find_library(Vulkan_LIBRARY NAMES vulkan HINTS /opt/homebrew/lib /usr/local/lib)
        if(Vulkan_INCLUDE_DIR AND Vulkan_LIBRARY)
            set(Vulkan_FOUND TRUE)
        endif()
    endif()
    if(Vulkan_FOUND)
        set(FLUX_VULKAN_AVAILABLE ON)
    else()
        message(FATAL_ERROR "Vulkan not found. Install the Vulkan SDK.")
    endif()
endif()

if(FLUX_VULKAN_AVAILABLE)
    message(STATUS "Vulkan found — building Vulkan GPU backend")

    FetchContent_Declare(VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.2.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(VulkanMemoryAllocator)

    list(APPEND FLUX_SOURCES src/GPU/Vulkan/VulkanDevice.cpp)
endif()

# Flux static library
add_library(flux STATIC ${FLUX_SOURCES})

target_include_directories(flux PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(flux SYSTEM PUBLIC
    $<BUILD_INTERFACE:${NANOSVG_SOURCE_DIR}>
    $<BUILD_INTERFACE:${STB_SOURCE_DIR}>
)

target_link_libraries(flux PUBLIC
    Threads::Threads
    Freetype::Freetype
    libtess2
)
if(NOT APPLE)
    target_link_libraries(flux PUBLIC SDL3::SDL3)
endif()

if(APPLE)
    target_link_libraries(flux PRIVATE
        "-framework Cocoa"
        "-framework Carbon"
        "-framework AppKit"
        "-framework CoreText"
        "-framework CoreFoundation"
        "-framework Metal"
        "-framework QuartzCore"
    )
endif()

if(FLUX_VULKAN_AVAILABLE)
    target_compile_definitions(flux PUBLIC FLUX_HAS_VULKAN=1)
    if(TARGET VulkanMemoryAllocator)
        target_include_directories(flux SYSTEM PRIVATE
            ${vulkanmemoryallocator_SOURCE_DIR}/include
        )
    endif()
    if(Vulkan_INCLUDE_DIR)
        target_include_directories(flux SYSTEM PRIVATE ${Vulkan_INCLUDE_DIR})
    endif()
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(flux PRIVATE Vulkan::Vulkan)
    elseif(Vulkan_LIBRARY)
        target_link_libraries(flux PRIVATE ${Vulkan_LIBRARY})
    endif()
endif()

# Shader cross-compilation pipeline
include(${CMAKE_SOURCE_DIR}/cmake/CompileShaders.cmake OPTIONAL)
if(COMMAND compile_flux_shaders)
    compile_flux_shaders(
        TARGET flux
        SHADERS
            shaders/test_triangle.vert.glsl
            shaders/test_triangle.frag.glsl
            shaders/sdf_quad.vert.glsl
            shaders/sdf_line.vert.glsl
            shaders/rect.frag.glsl
            shaders/circle.frag.glsl
            shaders/line.frag.glsl
            shaders/glyph.vert.glsl
            shaders/glyph.frag.glsl
            shaders/path.vert.glsl
            shaders/path.frag.glsl
            shaders/image.vert.glsl
            shaders/image.frag.glsl
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated/shaders
    )
endif()

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
    add_executable(hello_world examples/hello-world/main.cpp)
    target_link_libraries(hello_world PRIVATE flux)

    add_executable(svg_demo examples/svg-demo/main.cpp)
    target_link_libraries(svg_demo PRIVATE flux)

    add_executable(clock examples/clock/main.cpp)
    target_link_libraries(clock PRIVATE flux)

    add_executable(calculator examples/calculator/main.cpp)
    target_link_libraries(calculator PRIVATE flux)

    add_executable(llm_studio examples/llm-studio/main.cpp)
    target_include_directories(llm_studio PRIVATE ${CMAKE_SOURCE_DIR}/examples/llm-studio)
    target_link_libraries(llm_studio PRIVATE flux)

    if(UNIX)
        add_executable(terminal
            examples/terminal/main.cpp
            examples/terminal/Pty.cpp
            examples/terminal/TerminalEmulator.cpp
            examples/terminal/TerminalKeyBindings.cpp
            examples/terminal/TerminalSession.cpp
            examples/terminal/TerminalView.cpp
        )
        target_include_directories(terminal PRIVATE ${CMAKE_SOURCE_DIR}/examples/terminal)
        target_link_libraries(terminal PRIVATE flux util)

        add_executable(terminal_memory_bench
            examples/terminal/memory_benchmark.cpp
            examples/terminal/TerminalEmulator.cpp
        )
        target_include_directories(terminal_memory_bench PRIVATE
            ${CMAKE_SOURCE_DIR}/examples/terminal
            ${CMAKE_SOURCE_DIR}/include
        )
        target_link_libraries(terminal_memory_bench PRIVATE Threads::Threads)
    endif()

endif()

# --- Unit Tests (Catch2) ---
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
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

# --- UI Tests (test server + Python runner) ---
option(BUILD_UI_TESTS "Build UI test applications" OFF)
if(BUILD_UI_TESTS)
    add_executable(ui_test_layout tests/ui/apps/test_layout.cpp)
    target_link_libraries(ui_test_layout PRIVATE flux)

    add_executable(ui_test_button tests/ui/apps/test_button.cpp)
    target_link_libraries(ui_test_button PRIVATE flux)

    add_executable(ui_test_checkbox_toggle tests/ui/apps/test_checkbox_toggle.cpp)
    target_link_libraries(ui_test_checkbox_toggle PRIVATE flux)

    add_executable(ui_test_radio_select tests/ui/apps/test_radio_select.cpp)
    target_link_libraries(ui_test_radio_select PRIVATE flux)

    add_executable(ui_test_text_input tests/ui/apps/test_text_input.cpp)
    target_link_libraries(ui_test_text_input PRIVATE flux)

    add_executable(ui_test_focus tests/ui/apps/test_focus.cpp)
    target_link_libraries(ui_test_focus PRIVATE flux)

    add_executable(ui_test_scroll tests/ui/apps/test_scroll.cpp)
    target_link_libraries(ui_test_scroll PRIVATE flux)

    add_executable(ui_test_visibility tests/ui/apps/test_visibility.cpp)
    target_link_libraries(ui_test_visibility PRIVATE flux)

    add_executable(ui_test_slider tests/ui/apps/test_slider.cpp)
    target_link_libraries(ui_test_slider PRIVATE flux)

    add_executable(ui_test_dialog tests/ui/apps/test_dialog.cpp)
    target_link_libraries(ui_test_dialog PRIVATE flux)

    add_executable(ui_test_nested_state tests/ui/apps/test_nested_state.cpp)
    target_link_libraries(ui_test_nested_state PRIVATE flux)

    set(FLUX_UI_TEST_TARGETS
        ui_test_layout ui_test_button ui_test_checkbox_toggle
        ui_test_radio_select ui_test_text_input ui_test_focus
        ui_test_scroll ui_test_visibility ui_test_slider
        ui_test_dialog ui_test_nested_state
    )
    if(UNIX)
        add_executable(ui_test_terminal
            examples/terminal/main.cpp
            examples/terminal/Pty.cpp
            examples/terminal/TerminalEmulator.cpp
            examples/terminal/TerminalKeyBindings.cpp
            examples/terminal/TerminalSession.cpp
            examples/terminal/TerminalView.cpp
        )
        target_include_directories(ui_test_terminal PRIVATE ${CMAKE_SOURCE_DIR}/examples/terminal)
        target_link_libraries(ui_test_terminal PRIVATE flux util)
        list(APPEND FLUX_UI_TEST_TARGETS ui_test_terminal)
    endif()

    add_custom_target(run_ui_tests
        COMMAND ${CMAKE_COMMAND} -E env FLUX_BUILD_DIR=${CMAKE_BINARY_DIR}
            python3 ${CMAKE_SOURCE_DIR}/tests/ui/run_tests.py -v
        DEPENDS ${FLUX_UI_TEST_TARGETS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Flux UI tests..."
    )
endif()

# Installation rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS flux libtess2
    EXPORT fluxTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/Flux
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES include/Flux.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT fluxTargets
    FILE fluxTargets.cmake
    NAMESPACE flux::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flux
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/fluxConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fluxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fluxConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flux
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/fluxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/fluxConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flux
)
