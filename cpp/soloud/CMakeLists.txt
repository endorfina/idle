set(SOLOUD_COMPILE_FLAGS "-O3")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(SOLOUD_COMPILE_FLAGS "${SOLOUD_COMPILE_FLAGS} -Wno-misleading-indentation -Wno-unused-but-set-variable -Wno-sign-compare -Wno-unused-result")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(SOLOUD_COMPILE_FLAGS "${SOLOUD_COMPILE_FLAGS} -Wno-deprecated-register")
endif()

add_definitions(-DWITH_MODPLUG=1)
file(GLOB SOLOUD_FILES
            "src/audiosource/modplug/*.cpp"
            "src/core/*.cpp"
            "src/filter/*.cpp")
file(GLOB LIBMODPLUG_FILES
            "ext/libmodplug/src/*.cpp")

add_library(soloud STATIC ${SOLOUD_FILES})

if(ANDROID)
    set(SOLOUD_DEFINITIONS "WITH_OPENSLES=1")
    file(GLOB SOLOUD_PLATFORM_FILES "src/backend/opensles/*.cpp")

    target_link_libraries(soloud "OpenSLES")
elseif(MACOSX)
    set(SOLOUD_DEFINITIONS "WITH_COREAUDIO=1")
    file(GLOB SOLOUD_PLATFORM_FILES "src/backend/coreaudio/*.cpp")
else()
    find_package(ALSA REQUIRED)

    set(SOLOUD_DEFINITIONS "WITH_ALSA=1")
    file(GLOB SOLOUD_PLATFORM_FILES "src/backend/alsa/*.cpp")

    target_include_directories(soloud PRIVATE ${ALSA_INCLUDE_DIRS})
    target_link_libraries(soloud ${ALSA_LIBRARIES})
endif()

target_sources(soloud PRIVATE ${SOLOUD_PLATFORM_FILES})

target_include_directories(soloud PUBLIC include)
target_compile_definitions(soloud PRIVATE ${SOLOUD_DEFINITIONS})
set_target_properties(soloud PROPERTIES COMPILE_FLAGS ${SOLOUD_COMPILE_FLAGS})
set_target_properties(soloud PROPERTIES CXX_STANDARD 11)

add_library(modplug STATIC
            "ext/libmodplug/src/libmodplug/it_defs.h"
            "ext/libmodplug/src/libmodplug/sndfile.h"
            "ext/libmodplug/src/libmodplug/stdafx.h")

target_sources(modplug PRIVATE ${LIBMODPLUG_FILES})

target_compile_definitions(modplug PRIVATE MODPLUG_BUILD MODPLUG_STATIC HAVE_SETENV HAVE_SINF)
set_target_properties(modplug PROPERTIES COMPILE_FLAGS ${SOLOUD_COMPILE_FLAGS})
set_target_properties(modplug PROPERTIES CXX_STANDARD 11)
target_include_directories(modplug
            PRIVATE ext/libmodplug/src
            PRIVATE ext/libmodplug/src/libmodplug)
