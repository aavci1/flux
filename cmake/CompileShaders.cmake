# CompileShaders.cmake
# Compiles GLSL -> SPIR-V (glslc) -> MSL (spirv-cross) and embeds both
# as C++ constexpr arrays in a generated header.
#
# Usage:
#   compile_flux_shaders(
#     TARGET flux
#     SHADERS shaders/test_triangle.vert.glsl shaders/test_triangle.frag.glsl
#     OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated/shaders
#   )

find_program(GLSLC glslc HINTS /opt/homebrew/bin /usr/local/bin)
find_program(SPIRV_CROSS spirv-cross HINTS /opt/homebrew/bin /usr/local/bin)

function(compile_flux_shaders)
    cmake_parse_arguments(CS "" "TARGET;OUTPUT_DIR" "SHADERS" ${ARGN})

    if(NOT GLSLC OR NOT SPIRV_CROSS)
        message(WARNING "glslc or spirv-cross not found — shader compilation disabled")
        return()
    endif()

    file(MAKE_DIRECTORY ${CS_OUTPUT_DIR})
    set(ALL_OUTPUTS "")

    foreach(SRC ${CS_SHADERS})
        get_filename_component(FNAME ${SRC} NAME)

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
    endforeach()

    add_custom_target(${CS_TARGET}_compile_shaders DEPENDS ${ALL_OUTPUTS})
    add_dependencies(${CS_TARGET} ${CS_TARGET}_compile_shaders)
    target_include_directories(${CS_TARGET} PUBLIC ${CS_OUTPUT_DIR})
endfunction()
