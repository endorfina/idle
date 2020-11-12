add_library(lodepng STATIC "lodepng/lodepng.cpp")
target_include_directories(lodepng INTERFACE lodepng)
target_compile_definitions(lodepng PUBLIC
            LODEPNG_NO_COMPILE_ZLIB LODEPNG_NO_COMPILE_DISK
            $<$<NOT:$<CONFIG:Debug>>:LODEPNG_NO_COMPILE_ERROR_TEXT>)

add_library(${PROJECT_NAME}-png STATIC "png.cpp")
target_link_libraries(${PROJECT_NAME}-png
            PUBLIC ${PROJECT_NAME}-top
            PRIVATE lodepng
            PRIVATE ZLIB::ZLIB)

