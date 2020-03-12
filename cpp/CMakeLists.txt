cmake_minimum_required(VERSION 3.8.2)

project(idle
    VERSION 0.1
    LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -ffast-math")
set(CMAKE_CXX_STANDARD 17) # 17 requires 3.8.2, 20 requires 3.12.4
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_CXX_EXTENSIONS OFF) # enforces -std=c++11 instead of -std=gnu++11
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "${PROJECT_NAME} build configuration" FORCE)
endif()
message(STATUS "Build configured as \"${CMAKE_BUILD_TYPE}\"")

if(ANDROID)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

option(PRUNE_SYMBOLS "Removes symbols at build time" OFF)

option(MAKESHIFT_UNITY "Tries to build the main library in unity mode, when cmake doesn't provide one" OFF)

option(COMPILE_GALLERY OFF)

add_library(lodepng STATIC "lodepng/lodepng.cpp")
target_compile_definitions(lodepng PUBLIC LODEPNG_NO_COMPILE_ZLIB LODEPNG_NO_COMPILE_DISK)
target_include_directories(lodepng INTERFACE lodepng)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(${PROJECT_NAME}-files
        database.cpp
        drawable.cpp
        pointer.cpp
        fonts.cpp
        gl.cpp
        spinlock.cpp
        main_wrapper.cpp
        room_controller.cpp
        room_landing.cpp
        room_model.cpp
        scoped_thread.cpp
        additional_textures.cpp
        lodge.cpp
        application.cpp)

function(jumbo library filelist)
    set(UNITY_FILE "${CMAKE_BINARY_DIR}/${library}.jumbo.cpp")
    file(WRITE ${UNITY_FILE} "/* Jumbo build file generated by cmake (MAKESHIFT_UNITY) */\n")

    foreach(FF IN LISTS filelist)
        file(APPEND ${UNITY_FILE} "#include \"${CMAKE_SOURCE_DIR}/${FF}\"\n")
    endforeach()

    target_sources(${library} PRIVATE ${UNITY_FILE})
endfunction()

set(SHADER_OUTPUT "${PROJECT_NAME}-include/embedded_shaders.hpp")
set(SHADER_SOURCE "${CMAKE_SOURCE_DIR}/shader_source.txt")
set(SHADER_GENERATOR "${CMAKE_SOURCE_DIR}/generate.sh")

add_library(${PROJECT_NAME}-obj STATIC "${CMAKE_BINARY_DIR}/${SHADER_OUTPUT}")
target_link_libraries(${PROJECT_NAME}-obj lodepng)

add_custom_target(${PROJECT_NAME}-generate-dir
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_NAME}-include")

add_custom_target(${PROJECT_NAME}-generate-sh
    COMMAND test -r "${SHADER_OUTPUT}" -a "${SHADER_OUTPUT}" -ot "${SHADER_SOURCE}" -a "${SHADER_OUTPUT}" -ot "${SHADER_GENERATOR}" && echo "${SHADER_OUTPUT} is up-to-date." || "${SHADER_GENERATOR}" < "${SHADER_SOURCE}" > "${SHADER_OUTPUT}"
    BYPRODUCTS "${SHADER_OUTPUT}"
    COMMENT "Preparing ${SHADER_OUTPUT}"
    VERBATIM)

add_dependencies(${PROJECT_NAME}-generate-sh ${PROJECT_NAME}-generate-dir)
add_dependencies(${PROJECT_NAME}-obj ${PROJECT_NAME}-generate-sh)

target_include_directories(${PROJECT_NAME}-obj PRIVATE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-include")

if(MAKESHIFT_UNITY)
    jumbo(${PROJECT_NAME}-obj "${${PROJECT_NAME}-files}")
else()
    target_sources(${PROJECT_NAME}-obj PRIVATE ${${PROJECT_NAME}-files})
endif()

target_compile_definitions(${PROJECT_NAME}-obj PUBLIC APPLICATION_FPS=40)
set_target_properties(${PROJECT_NAME}-obj PROPERTIES COMPILE_FLAGS "-Wall -Wredundant-move")
set_source_files_properties(jpeg.cpp PROPERTIES COMPILE_FLAGS "-Wno-sign-compare")

if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME}-obj PUBLIC DEBUG)
else()
    target_compile_definitions(lodepng PUBLIC LODEPNG_NO_COMPILE_ERROR_TEXT)
endif()

if(COMPILE_GALLERY)
    target_compile_definitions(${PROJECT_NAME}-obj PUBLIC IDLE_COMPILE_GALLERY)
endif()

add_subdirectory(platform)
add_subdirectory(soloud)

if(NOT ANDROID)
    add_subdirectory(test)
endif()
