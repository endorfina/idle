add_library(${PROJECT_NAME}-ft STATIC "glue.cpp")
target_link_libraries(${PROJECT_NAME}-ft PUBLIC ${PROJECT_NAME}-top)

if(ANDROID)
    target_link_libraries(${PROJECT_NAME}-ft PRIVATE freetype)
else()
    find_package(Freetype REQUIRED)
    target_link_libraries(${PROJECT_NAME}-ft PRIVATE Freetype::Freetype)
endif()
