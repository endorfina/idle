if(ANDROID)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    target_include_directories(${PROJECT_NAME}-top INTERFACE ${ANDROID_NDK}/sources/android/native_app_glue)
    add_library(android_glue OBJECT "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")

    get_filename_component(FREETYPE_SOURCE_LOCATION ../freetype REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}" CACHE)

    message(STATUS "Local freetype: ${FREETYPE_SOURCE_LOCATION}")

    function(resolve_files library base_dir filelist)
        set(globs "")
        foreach(FF IN LISTS filelist)
            list(APPEND globs "${base_dir}/${FF}")
        endforeach()

        file(GLOB new_filelist ${globs})
        target_sources(${library} PRIVATE ${new_filelist})
    endfunction()

    set(FT_HEADERS "ft2build.h" "freetype/*.h" "freetype/config/*.h" "freetype/internal/*.h")
    set(FT_SOURCES
            autofit/autofit.c
            base/ftbase.c
            base/ftbbox.c
            base/ftbitmap.c
            base/ftdebug.c
            base/ftfntfmt.c
            base/ftfstype.c
            base/ftgasp.c
            base/ftglyph.c
            base/ftgxval.c
            base/ftinit.c
            base/ftlcdfil.c
            base/ftmm.c
            base/ftotval.c
            base/ftpatent.c
            base/ftpfr.c
            base/ftstroke.c
            base/ftsynth.c
            base/ftsystem.c
            base/fttype1.c
            base/ftwinfnt.c
            bdf/bdf.c
            cache/ftcache.c
            cff/cff.c
            cid/type1cid.c
            gzip/ftgzip.c
            lzw/ftlzw.c
            pcf/pcf.c
            pfr/pfr.c
            psaux/psaux.c
            pshinter/pshinter.c
            psnames/psmodule.c
            raster/raster.c
            sfnt/sfnt.c
            smooth/smooth.c
            truetype/truetype.c
            type1/type1.c
            type42/type42.c
            winfonts/winfnt.c)

    add_library(freetype STATIC)
    resolve_files(freetype "${FREETYPE_SOURCE_LOCATION}/include" "${FT_HEADERS}")
    resolve_files(freetype "${FREETYPE_SOURCE_LOCATION}/src" "${FT_SOURCES}")
    target_compile_definitions(freetype PRIVATE FT2_BUILD_LIBRARY)
    target_include_directories(freetype PUBLIC "${FREETYPE_SOURCE_LOCATION}/include")

    # Export ANativeActivity_onCreate(),
    # Refer to: https://github.com/android-ndk/ndk/issues/381.
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:android_glue> "egl.cpp")
    target_link_libraries(${PROJECT_NAME} PRIVATE "android" "log" "EGL" "GLESv2")

else()
    get_filename_component(OUTPUT_PATH "../out" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}" CACHE)

    option(X11_USE_CLIENTMESSAGE "Makes x11 handle closing in a cleaner manner by intercepting close calls from your WM" ON)

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    find_package(X11 REQUIRED)
    cmake_policy(SET CMP0072 NEW)
    find_package(OpenGL REQUIRED)

    add_executable(${PROJECT_NAME} "x.cpp")
    target_link_libraries(${PROJECT_NAME} PRIVATE X11::X11 OpenGL::GL Threads::Threads atomic)

    if (X11_USE_CLIENTMESSAGE)
        target_compile_definitions(${PROJECT_NAME} PRIVATE X11_USE_CLIENTMESSAGE)
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_PATH}")

    set(CLI_RUN_COMMAND "cd '${OUTPUT_PATH}' && './${PROJECT_NAME}'" CACHE STRING "A command which can be used to boot ${PROJECT_NAME}" FORCE)

    if(NOT EXISTS "${OUTPUT_PATH}")
        file(MAKE_DIRECTORY "${OUTPUT_PATH}")
        # links seem to depend on cmake 3.14, but we're gonna assume its 3.14 on X11
        file(CREATE_LINK "${CMAKE_SOURCE_DIR}/config/runtime_assets" "${OUTPUT_PATH}/assets" COPY_ON_ERROR SYMBOLIC)
    endif()
endif()

add_library(${PROJECT_NAME}-opengl-glue OBJECT "opengl_core_adaptive.cpp")

target_sources(${PROJECT_NAME} PRIVATE $<TARGET_OBJECTS:${PROJECT_NAME}-opengl-glue> "cmd_queue.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-obj)

if(PRUNE_SYMBOLS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_STRIP} --strip-debug --strip-unneeded "$<TARGET_FILE:${PROJECT_NAME}>")
endif()
