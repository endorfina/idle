find_package(ZLIB REQUIRED)

add_library(${PROJECT_NAME}-ft STATIC "../FreeType.cpp")

if(ANDROID)
    set(PLATFORM_LIBRARIES "android" "log" "EGL" "GLESv2")
    target_include_directories(${PROJECT_NAME}-obj PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)
    target_include_directories(${PROJECT_NAME}-ft PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)
    add_library(android_glue OBJECT "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")

    get_filename_component(FREETYPE_SOURCE_LOCATION ../freetype REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}" CACHE)

    message(STATUS "Local freetype: ${FREETYPE_SOURCE_LOCATION}")

    file(GLOB FT_PUBLIC_HEADERS "${FREETYPE_SOURCE_LOCATION}/include/ft2build.h" "${FREETYPE_SOURCE_LOCATION}/include/freetype/*.h")
    file(GLOB FT_PUBLIC_CONFIG_HEADERS "${FREETYPE_SOURCE_LOCATION}/include/freetype/config/*.h")
    file(GLOB FT_PRIVATE_HEADERS "${FREETYPE_SOURCE_LOCATION}/include/freetype/internal/*.h")

    add_library(freetype STATIC
            ${FT_PUBLIC_HEADERS}
            ${FT_PUBLIC_CONFIG_HEADERS}
            ${FT_PRIVATE_HEADERS}
            ${FREETYPE_SOURCE_LOCATION}/src/autofit/autofit.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftbase.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftbbox.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftbitmap.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftdebug.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftfntfmt.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftfstype.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftgasp.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftglyph.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftgxval.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftinit.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftlcdfil.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftmm.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftotval.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftpatent.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftpfr.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftstroke.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftsynth.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftsystem.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/fttype1.c
            ${FREETYPE_SOURCE_LOCATION}/src/base/ftwinfnt.c
            ${FREETYPE_SOURCE_LOCATION}/src/bdf/bdf.c
            ${FREETYPE_SOURCE_LOCATION}/src/cache/ftcache.c
            ${FREETYPE_SOURCE_LOCATION}/src/cff/cff.c
            ${FREETYPE_SOURCE_LOCATION}/src/cid/type1cid.c
            ${FREETYPE_SOURCE_LOCATION}/src/gzip/ftgzip.c
            ${FREETYPE_SOURCE_LOCATION}/src/lzw/ftlzw.c
            ${FREETYPE_SOURCE_LOCATION}/src/pcf/pcf.c
            ${FREETYPE_SOURCE_LOCATION}/src/pfr/pfr.c
            ${FREETYPE_SOURCE_LOCATION}/src/psaux/psaux.c
            ${FREETYPE_SOURCE_LOCATION}/src/pshinter/pshinter.c
            ${FREETYPE_SOURCE_LOCATION}/src/psnames/psmodule.c
            ${FREETYPE_SOURCE_LOCATION}/src/raster/raster.c
            ${FREETYPE_SOURCE_LOCATION}/src/sfnt/sfnt.c
            ${FREETYPE_SOURCE_LOCATION}/src/smooth/smooth.c
            ${FREETYPE_SOURCE_LOCATION}/src/truetype/truetype.c
            ${FREETYPE_SOURCE_LOCATION}/src/type1/type1.c
            ${FREETYPE_SOURCE_LOCATION}/src/type42/type42.c
            ${FREETYPE_SOURCE_LOCATION}/src/winfonts/winfnt.c)
    target_compile_definitions(freetype PRIVATE FT2_BUILD_LIBRARY)
    target_include_directories(freetype PUBLIC "${FREETYPE_SOURCE_LOCATION}/include")
    target_link_libraries(${PROJECT_NAME}-ft freetype)

    # Export ANativeActivity_onCreate(),
    # Refer to: https://github.com/android-ndk/ndk/issues/381.
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:android_glue> "egl.cpp")

    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall")

else()
    find_package(Freetype REQUIRED)
    target_include_directories(${PROJECT_NAME}-ft PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}-ft ${FREETYPE_LIBRARIES})

    get_filename_component(OUTPUT_PATH "../out" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}" CACHE)

    # mostly so I can run tests in isolation at work
    if(NOT MACOSX)
        find_package(X11 REQUIRED)
        cmake_policy(SET CMP0072 NEW)
        find_package(OpenGL REQUIRED)
        set(PLATFORM_LIBRARIES ${X11_X11_LIB} ${OPENGL_LIBRARIES} pthread atomic)

        add_executable(${PROJECT_NAME} "x.cpp")

        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall")
        target_include_directories(${PROJECT_NAME} PRIVATE ${X11_X11_INCLUDE_PATH})

        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
    endif()
endif()

if(NOT MACOSX)
    target_sources(${PROJECT_NAME} PRIVATE "opengl_core_adaptive.cpp")

    target_link_libraries(${PROJECT_NAME}
                  ${PROJECT_NAME}-obj
                  ${PROJECT_NAME}-ft
                  ${PLATFORM_LIBRARIES}
                  ${ZLIB_LIBRARIES})

    if(PRUNE_SYMBOLS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
              COMMAND ${CMAKE_STRIP} --strip-debug --strip-unneeded "$<TARGET_FILE:${PROJECT_NAME}>")
    endif()

endif()
