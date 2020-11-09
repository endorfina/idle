set(IDLE_INCLUDE_DIR_BASENAME "${PROJECT_NAME}-include")
set(IDLE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${IDLE_INCLUDE_DIR_BASENAME}")
file(MAKE_DIRECTORY "${IDLE_INCLUDE_DIR}")

set(SHADER_OUTPUT_BASENAME "embedded_shaders.hpp")
set(SHADER_OUTPUT "${IDLE_INCLUDE_DIR_BASENAME}/${SHADER_OUTPUT_BASENAME}")
set(SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shader_source.glsl")
set(SHADER_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/generate.sh")

add_library(${PROJECT_NAME}-shaders INTERFACE)
target_include_directories(${PROJECT_NAME}-shaders INTERFACE "${IDLE_INCLUDE_DIR}")

# When we build idle this will automatically re-generate the header,
# provided either the shader source or the script had been modified.
add_custom_target(${PROJECT_NAME}-generate-sh
    COMMAND test
        -r "${SHADER_OUTPUT}"
        -a "${SHADER_OUTPUT}" -nt "${SHADER_SOURCE}"
        -a "${SHADER_OUTPUT}" -nt "${SHADER_GENERATOR}"
        && echo "🏮 ${SHADER_OUTPUT_BASENAME} is up-to-date."
        || "${SHADER_GENERATOR}" < "${SHADER_SOURCE}" > "${SHADER_OUTPUT}"
    BYPRODUCTS "${SHADER_OUTPUT}"
    COMMENT "Preparing ${SHADER_OUTPUT}"
    VERBATIM)

add_dependencies(${PROJECT_NAME}-shaders ${PROJECT_NAME}-generate-sh)
