# CompileShaders.cmake
# Compiles GLSL -> SPIR-V (glslc) -> MSL (spirv-cross), then embeds .metal and .spv
# into FluxEmbeddedShaders.cpp so the library works when the executable is run from any cwd.
#
# Usage:
#   compile_flux_shaders(
#     TARGET flux
#     SHADERS shaders/foo.vert.glsl ...
#     OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated/shaders
#   )

find_program(GLSLC glslc HINTS /opt/homebrew/bin /usr/local/bin)
find_program(SPIRV_CROSS spirv-cross HINTS /opt/homebrew/bin /usr/local/bin)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

function(compile_flux_shaders)
    cmake_parse_arguments(CS "" "TARGET;OUTPUT_DIR" "SHADERS" ${ARGN})

    if(NOT GLSLC OR NOT SPIRV_CROSS)
        message(WARNING "glslc or spirv-cross not found — shader compilation disabled")
        return()
    endif()

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    file(MAKE_DIRECTORY ${CS_OUTPUT_DIR})
    set(ALL_OUTPUTS "")
    set(EMBED_ARGS "")

    foreach(SRC ${CS_SHADERS})
        get_filename_component(FNAME ${SRC} NAME)
        string(REPLACE "." "_" STEM ${FNAME})

        # Determine shader stage from filename
        if(FNAME MATCHES "\\.vert\\.")
            set(STAGE vertex)
        elseif(FNAME MATCHES "\\.frag\\.")
            set(STAGE fragment)
        elseif(FNAME MATCHES "\\.comp\\.")
            set(STAGE compute)
        else()
            message(FATAL_ERROR "Cannot determine shader stage for ${FNAME}")
        endif()

        set(SPV ${CS_OUTPUT_DIR}/${FNAME}.spv)
        set(MSL ${CS_OUTPUT_DIR}/${FNAME}.metal)

        add_custom_command(
            OUTPUT ${SPV}
            COMMAND ${GLSLC} -fshader-stage=${STAGE} -o ${SPV} ${CMAKE_SOURCE_DIR}/${SRC}
            DEPENDS ${CMAKE_SOURCE_DIR}/${SRC}
            COMMENT "GLSL -> SPIR-V: ${FNAME}"
        )
        add_custom_command(
            OUTPUT ${MSL}
            COMMAND ${SPIRV_CROSS} ${SPV} --msl --msl-version 20000 --output ${MSL}
            DEPENDS ${SPV}
            COMMENT "SPIR-V -> MSL: ${FNAME}"
        )
        list(APPEND ALL_OUTPUTS ${SPV} ${MSL})
        list(APPEND EMBED_ARGS ${MSL} ${SPV} ${STEM})
    endforeach()

    set(EMBED_BASE ${CMAKE_BINARY_DIR}/generated/FluxEmbeddedShaders)
    set(EMBED_GEN_HPP ${EMBED_BASE}.hpp)
    set(EMBED_GEN_CPP ${EMBED_BASE}.cpp)

    add_custom_command(
        OUTPUT ${EMBED_GEN_HPP} ${EMBED_GEN_CPP}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/embed_shaders.py
            ${EMBED_BASE} ${EMBED_ARGS}
        DEPENDS ${ALL_OUTPUTS} ${CMAKE_SOURCE_DIR}/cmake/embed_shaders.py
        COMMENT "Embedding GPU shaders (MSL + SPIR-V)"
    )

    add_custom_target(${CS_TARGET}_compile_shaders DEPENDS ${ALL_OUTPUTS})
    add_dependencies(${CS_TARGET} ${CS_TARGET}_compile_shaders)

    target_sources(${CS_TARGET} PRIVATE ${EMBED_GEN_CPP})
    target_include_directories(${CS_TARGET} PRIVATE ${CMAKE_BINARY_DIR}/generated)
    target_compile_definitions(${CS_TARGET} PRIVATE FLUX_HAS_EMBEDDED_SHADERS=1)
endfunction()
