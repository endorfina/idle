set(IDLE_TEST ${PROJECT_NAME}-test)

set(IDLE_TEST_UNITS "")

macro(new_test testname filelist)
    add_executable(${IDLE_TEST}-${testname} EXCLUDE_FROM_ALL ${filelist})
    list(APPEND IDLE_TEST_UNITS "${testname}")
endmacro()

new_test(glass glass.cpp)

add_library(${IDLE_TEST}-main STATIC EXCLUDE_FROM_ALL "interface.cpp")
target_include_directories(${IDLE_TEST}-main INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_options(${IDLE_TEST}-main PUBLIC "-fexceptions")
target_link_libraries(${IDLE_TEST}-main PUBLIC ${PROJECT_NAME}-top)

foreach(UT IN LISTS IDLE_TEST_UNITS)
    target_link_libraries(${IDLE_TEST}-${UT} PRIVATE ${IDLE_TEST}-main)
    list(APPEND TEST_BINS ${IDLE_TEST}-${UT})

    add_test(
        NAME    ${IDLE_TEST}-${UT}
        COMMAND ${IDLE_TEST}-${UT}
        DEPENDS ${IDLE_TEST}-${UT}
    )
endforeach()

add_custom_target(${IDLE_TEST} DEPENDS ${TEST_BINS})

enable_testing()

add_custom_target(${IDLE_TEST}-run
    COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure -j4
    DEPENDS ${IDLE_TEST}
)
