cmake_minimum_required(VERSION 3.12.4)

project(idle
    VERSION 0.1
    LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -ffast-math")
# set(CMAKE_CXX_STANDARD 17) # 17 requires 3.8.2, 20 requires 3.12.4
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_CXX_EXTENSIONS OFF) # enforces -std=c++11 instead of -std=gnu++11
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

message(STATUS "Build configured as \"${CMAKE_BUILD_TYPE}\"")

set(${PROJECT_NAME}-files
        spinlock.cpp
        drawable.cpp
        crash_handler.cpp
        statistician.cpp
        hotel/service.cpp
        hotel/room_landing.cpp
        hotel/room_stage.cpp
        hotel/stage_objects.cpp
        hotel/stage_crawlers.cpp
        hotel/room_model.cpp
        hotel/image_loader.cpp
        room_controller.cpp
        lodge.cpp
        gl.cpp
        fonts.cpp
        pointer_wrapper.cpp
        application.cpp
        main_wrapper.cpp)

set(LOG_LEVEL "2" CACHE STRING "Print or hide logs. 0 - nothing, 1 - error, 2 - warning, 3 - info, 4 - verbose")

option(PRUNE_SYMBOLS "Removes symbols at build time" ON)

option(MAKESHIFT_UNITY "Tries to build the main library in unity mode, when cmake doesn't provide one" ON)

option(COMPILE_GALLERY "Rig testing gallery" OFF)

option(COMPILE_FPS_COUNTERS "Performance measurement" OFF)

option(COMPILE_FONT_DEBUG_SCREEN "Shows a splash of the entire loaded font texture" OFF)

option(DOUBLE_THE_FPS "Double it!" OFF)

function(jumbo library filelist)
    set(UNITY_FILE "${CMAKE_CURRENT_BINARY_DIR}/${library}.jumbo.cpp")
    file(WRITE ${UNITY_FILE} "/* Jumbo build file generated by cmake (MAKESHIFT_UNITY) */\n")

    foreach(FF IN LISTS filelist)
        if(NOT FF MATCHES "^/")
            set(FF "${CMAKE_CURRENT_SOURCE_DIR}/${FF}")
        endif()

        file(APPEND ${UNITY_FILE} "#include \"${FF}\"\n")
    endforeach()

    target_sources(${library} PRIVATE ${UNITY_FILE})
endfunction()

add_library(${PROJECT_NAME}-top INTERFACE)
add_library(${PROJECT_NAME}-obj STATIC)

target_compile_definitions(${PROJECT_NAME}-obj PRIVATE
    $<$<BOOL:${COMPILE_FPS_COUNTERS}>:IDLE_COMPILE_FPS_COUNTERS>
    $<$<BOOL:${COMPILE_FONT_DEBUG_SCREEN}>:IDLE_COMPILE_FONT_DEBUG_SCREEN>
    $<$<BOOL:${COMPILE_GALLERY}>:IDLE_COMPILE_GALLERY>)

if(MAKESHIFT_UNITY)
    jumbo(${PROJECT_NAME}-obj "${${PROJECT_NAME}-files}")
else()
    target_sources(${PROJECT_NAME}-obj PRIVATE ${${PROJECT_NAME}-files})
endif()

target_include_directories(${PROJECT_NAME}-top INTERFACE include)
target_compile_features(${PROJECT_NAME}-top INTERFACE cxx_std_20)
target_compile_definitions(${PROJECT_NAME}-top INTERFACE
        $<$<BOOL:${DOUBLE_THE_FPS}>:IDLE_DOUBLE_THE_FPS>
        $<$<CONFIG:Debug>:DEBUG> "LOG_LEVEL=${LOG_LEVEL}")
target_compile_options(${PROJECT_NAME}-top INTERFACE
        -Wall -Wredundant-move -fno-char8_t
        $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=268435456>
        $<$<CXX_COMPILER_ID:Clang>:-fconstexpr-steps=268435456>)

find_package(ZLIB REQUIRED)
add_subdirectory(platform)
add_subdirectory(game)
add_subdirectory(freetype)
add_subdirectory(shaders)
add_subdirectory(png)
# add_subdirectory(soloud) # TODO: turn this back on when we need sound again

target_link_libraries(${PROJECT_NAME}-obj
            PUBLIC ${PROJECT_NAME}-top
            PRIVATE ${PROJECT_NAME}-ft
            PRIVATE ${PROJECT_NAME}-game
            PRIVATE ${PROJECT_NAME}-shaders
            PRIVATE ${PROJECT_NAME}-png
            PRIVATE ZLIB::ZLIB)

if(NOT ANDROID)
    target_link_libraries(${PROJECT_NAME}-obj PRIVATE atomic)

    add_subdirectory(test)
endif()

