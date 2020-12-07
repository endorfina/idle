set(${PROJECT_NAME}-game-files
        characters/octavia.cpp)

add_library(${PROJECT_NAME}-game STATIC)

if(MAKESHIFT_UNITY)
    jumbo(${PROJECT_NAME}-game "${${PROJECT_NAME}-game-files}")
else()
    target_sources(${PROJECT_NAME}-game PRIVATE ${${PROJECT_NAME}-game-files})
endif()

target_link_libraries(${PROJECT_NAME}-game PUBLIC ${PROJECT_NAME}-top)

