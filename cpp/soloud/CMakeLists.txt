add_library(soloud-top INTERFACE)
target_compile_options(soloud-top INTERFACE -O3)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    target_compile_options(soloud-top INTERFACE -Wno-misleading-indentation -Wno-unused-but-set-variable -Wno-sign-compare -Wno-unused-result)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(soloud-top INTERFACE -Wno-deprecated-register)
endif()

file(GLOB SOLOUD_FILES
            "src/audiosource/modplug/*.cpp"
            "src/core/*.cpp"
            "src/filter/*.cpp")
file(GLOB LIBMODPLUG_FILES
            "ext/libmodplug/src/*.cpp")

add_library(soloud STATIC ${SOLOUD_FILES})
target_compile_definitions(soloud PRIVATE WITH_MODPLUG=1)

if(ANDROID)
    target_compile_definitions(soloud PRIVATE WITH_OPENSLES=1)
    file(GLOB SOLOUD_PLATFORM_FILES "src/backend/opensles/*.cpp")

    target_link_libraries(soloud PRIVATE "OpenSLES")
elseif(MACOSX)
    target_compile_definitions(soloud PRIVATE WITH_COREAUDIO=1)
    file(GLOB SOLOUD_PLATFORM_FILES "src/backend/coreaudio/*.cpp")
else()
    target_compile_definitions(soloud PRIVATE WITH_ALSA=1)
    file(GLOB SOLOUD_PLATFORM_FILES "src/backend/alsa/*.cpp")

    find_package(ALSA REQUIRED)
    target_link_libraries(soloud PRIVATE ALSA::ALSA)
endif()

target_sources(soloud PRIVATE ${SOLOUD_PLATFORM_FILES})
target_include_directories(soloud PUBLIC include)
target_link_libraries(soloud PRIVATE soloud-top)

add_library(modplug STATIC
            "ext/libmodplug/src/libmodplug/it_defs.h"
            "ext/libmodplug/src/libmodplug/sndfile.h"
            "ext/libmodplug/src/libmodplug/stdafx.h")

target_sources(modplug PRIVATE ${LIBMODPLUG_FILES})

target_compile_definitions(modplug PRIVATE MODPLUG_BUILD MODPLUG_STATIC HAVE_SETENV HAVE_SINF)
target_include_directories(modplug
            PRIVATE ext/libmodplug/src
            PRIVATE ext/libmodplug/src/libmodplug)
target_link_libraries(modplug PRIVATE soloud-top)

set_target_properties(modplug soloud PROPERTIES CXX_STANDARD 11)

