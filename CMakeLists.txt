cmake_minimum_required(VERSION 3.25)

project(flux VERSION 2.0.0 LANGUAGES CXX C OBJCXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-w)

# Dependencies
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)

include(FetchContent)

# SDL3: try system install first, fetch from GitHub as fallback
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

# Pin commits match former submodules; override with -DFLUX_*_GIT_TAG=...
set(FLUX_NANOVG_GIT_REPOSITORY "https://github.com/inniyah/nanovg.git" CACHE STRING "NanoVG Git repository")
set(FLUX_NANOVG_GIT_TAG "313ef1368bb186c7df21a578e4c5700061a7a68c" CACHE STRING "NanoVG Git tag or commit")
set(FLUX_NANOSVG_GIT_REPOSITORY "https://github.com/memononen/nanosvg.git" CACHE STRING "NanoSVG Git repository")
set(FLUX_NANOSVG_GIT_TAG "66579081d84b613daa49a64f76357ce65925e13b" CACHE STRING "NanoSVG Git tag or commit")
set(FLUX_STB_GIT_REPOSITORY "https://github.com/nothings/stb.git" CACHE STRING "stb Git repository")
set(FLUX_STB_GIT_TAG "fede005abaf93d9d7f3a679d1999b2db341b360f" CACHE STRING "stb Git tag or commit")

# NanoVG: Populate only (upstream CMake also defines target "nanovg" — would clash with ours)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanovg/src/nanovg.c")
    set(NANOVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanovg")
    message(STATUS "NanoVG: using local third_party/nanovg")
else()
    FetchContent_Declare(flux_nanovg
        GIT_REPOSITORY ${FLUX_NANOVG_GIT_REPOSITORY}
        GIT_TAG ${FLUX_NANOVG_GIT_TAG}
        GIT_SUBMODULES "" # upstream has optional submodules; not needed for core sources
    )
    FetchContent_GetProperties(flux_nanovg)
    if(NOT flux_nanovg_POPULATED)
        message(STATUS "NanoVG: fetching ${FLUX_NANOVG_GIT_TAG} from ${FLUX_NANOVG_GIT_REPOSITORY}")
        FetchContent_Populate(flux_nanovg)
    endif()
    set(NANOVG_SOURCE_DIR "${flux_nanovg_SOURCE_DIR}")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src/nanosvg.h")
    set(NANOSVG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/nanosvg/src")
    message(STATUS "NanoSVG: using local third_party/nanosvg")
else()
    FetchContent_Declare(flux_nanosvg
        GIT_REPOSITORY ${FLUX_NANOSVG_GIT_REPOSITORY}
        GIT_TAG ${FLUX_NANOSVG_GIT_TAG}
        GIT_SUBMODULES ""
    )
    FetchContent_GetProperties(flux_nanosvg)
    if(NOT flux_nanosvg_POPULATED)
        message(STATUS "NanoSVG: fetching ${FLUX_NANOSVG_GIT_TAG}")
        FetchContent_Populate(flux_nanosvg)
    endif()
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
    FetchContent_GetProperties(flux_stb)
    if(NOT flux_stb_POPULATED)
        message(STATUS "stb: fetching ${FLUX_STB_GIT_TAG}")
        FetchContent_Populate(flux_stb)
    endif()
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
    target_include_directories(libtess2 PUBLIC ${libtess2_SOURCE_DIR}/Include)
endif()

# NanoVG includes <stb/stb_image.h>; nothings/stb ships headers as stb_*.h at repo root
set(FLUX_STB_SHIM_DIR "${CMAKE_BINARY_DIR}/flux_stb_shim")
file(MAKE_DIRECTORY "${FLUX_STB_SHIM_DIR}/stb")
file(GLOB _flux_stb_shim_headers "${STB_SOURCE_DIR}/stb_*.h")
foreach(_flux_stb_shim_h IN LISTS _flux_stb_shim_headers)
    get_filename_component(_flux_stb_shim_name "${_flux_stb_shim_h}" NAME)
    configure_file("${_flux_stb_shim_h}" "${FLUX_STB_SHIM_DIR}/stb/${_flux_stb_shim_name}" COPYONLY)
endforeach()

# NanoVG includes <GL/gl.h>; macOS provides OpenGL via framework headers
if(APPLE)
    set(FLUX_GL_SHIM_DIR "${CMAKE_BINARY_DIR}/flux_gl_shim")
    file(MAKE_DIRECTORY "${FLUX_GL_SHIM_DIR}/GL")
    file(WRITE "${FLUX_GL_SHIM_DIR}/GL/gl.h" "#include <OpenGL/gl.h>\n")
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
    src/Core/FontDiscovery.cpp
    src/Core/ResourceManager.cpp

    # Graphics
    src/Graphics/Path.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/NanoVGRenderer.cpp
    src/Graphics/NanoVGRenderContext.cpp
    src/Graphics/NanoVGBackend.cpp
    src/Graphics/CommandCompiler.cpp
    src/Graphics/GPURendererBackend.cpp
    src/Graphics/GlyphAtlas.cpp
    src/Graphics/PathFlattener.cpp
    src/Graphics/ImageCache.cpp
    src/Graphics/GPURenderContext.cpp
    src/Platform/GPUPlatformRenderer.cpp

    # Testing
    src/Testing/ScreenCaptureImpl.cpp

    # Views
    src/Views/SVG.cpp
    src/Views/NanoSVG.cpp

    # Platform (SDL3)
    src/Platform/SDLWindow.cpp
)

# GPU backend sources
list(APPEND FLUX_SOURCES src/GPU/DeviceFactory.cpp)

if(APPLE)
    list(APPEND FLUX_SOURCES src/GPU/Metal/MetalDevice.mm)
endif()

# Vulkan backend (optional — requires vulkan headers + loader)
find_package(Vulkan QUIET)
if(NOT Vulkan_FOUND)
    # Try pkg-config / homebrew fallback
    find_path(Vulkan_INCLUDE_DIR vulkan/vulkan.h HINTS /opt/homebrew/include /usr/local/include)
    find_library(Vulkan_LIBRARY NAMES vulkan HINTS /opt/homebrew/lib /usr/local/lib)
    if(Vulkan_INCLUDE_DIR AND Vulkan_LIBRARY)
        set(Vulkan_FOUND TRUE)
    endif()
endif()

if(Vulkan_FOUND)
    message(STATUS "Vulkan found — building Vulkan GPU backend")

    # VMA (Vulkan Memory Allocator) — header-only
    FetchContent_Declare(VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.2.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(VulkanMemoryAllocator)

    list(APPEND FLUX_SOURCES src/GPU/Vulkan/VulkanDevice.cpp)
endif()

# Build NanoVG
add_library(nanovg STATIC
    ${NANOVG_SOURCE_DIR}/src/nanovg.c
    ${NANOVG_SOURCE_DIR}/src/nanovg_gl.c
    ${NANOVG_SOURCE_DIR}/src/nanovg_gl_utils.c
)

target_include_directories(nanovg PUBLIC
    ${NANOVG_SOURCE_DIR}/src
    ${FLUX_STB_SHIM_DIR}
)
if(APPLE)
    target_include_directories(nanovg PRIVATE ${FLUX_GL_SHIM_DIR})
endif()

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
    libtess2
)

if(APPLE)
    target_link_libraries(flux PRIVATE
        "-framework CoreText"
        "-framework CoreFoundation"
        "-framework Metal"
        "-framework QuartzCore"
    )
endif()

if(Vulkan_FOUND)
    target_compile_definitions(flux PUBLIC FLUX_HAS_VULKAN=1)
    if(TARGET VulkanMemoryAllocator)
        target_include_directories(flux PRIVATE
            ${vulkanmemoryallocator_SOURCE_DIR}/include
        )
    endif()
    if(Vulkan_INCLUDE_DIR)
        target_include_directories(flux PRIVATE ${Vulkan_INCLUDE_DIR})
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
    add_executable(shapes examples/shapes/main.cpp)
    target_link_libraries(shapes PRIVATE flux)

    add_executable(clock examples/19-clock/main.cpp)
    target_link_libraries(clock PRIVATE flux)

    add_executable(calculator examples/24-calculator/main.cpp)
    target_link_libraries(calculator PRIVATE flux)

    add_executable(llm_studio examples/llm-studio/main.cpp)
    target_include_directories(llm_studio PRIVATE ${CMAKE_SOURCE_DIR}/examples/llm-studio)
    target_link_libraries(llm_studio PRIVATE flux)

    add_executable(gpu_test examples/gpu-test/main.cpp)
    target_link_libraries(gpu_test PRIVATE flux)

    add_executable(gpu_shapes_test examples/gpu-test/shapes_test.cpp)
    target_link_libraries(gpu_shapes_test PRIVATE flux)

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

    add_custom_target(run_ui_tests
        COMMAND ${CMAKE_COMMAND} -E env FLUX_BUILD_DIR=${CMAKE_BINARY_DIR}
            python3 ${CMAKE_SOURCE_DIR}/tests/ui/run_tests.py -v
        DEPENDS
            ui_test_layout ui_test_button ui_test_checkbox_toggle
            ui_test_radio_select ui_test_text_input ui_test_focus
            ui_test_scroll ui_test_visibility ui_test_slider
            ui_test_dialog ui_test_nested_state
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Flux UI tests..."
    )
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
