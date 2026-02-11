# Tests directory
enable_testing()

# ==============================
# Fetch Google Test
# ==============================
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ==============================
# Helper function to create a unit test target
# ==============================
function(add_unit_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    target_include_directories(${TEST_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/engine/include"
        "${FILAMENT_DIST_DIR}/include"
    )
    target_link_libraries(${TEST_NAME} PRIVATE
        GTest::gtest_main
        entt
    )
    gtest_discover_tests(${TEST_NAME})
endfunction()

# ==============================
# Unit Tests (no Filament engine needed at runtime)
# ==============================

add_unit_test(test_resource_handle unit/test_resource_handle.cpp)
add_unit_test(test_components      unit/test_components.cpp)
add_unit_test(test_math_utils      unit/test_math_utils.cpp)
add_unit_test(test_event_bus       unit/test_event_bus.cpp)

# Clock test links the full engine lib (needs Clock implementation)
add_executable(test_clock unit/test_clock.cpp)
target_include_directories(test_clock PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_clock PRIVATE
    GTest::gtest_main
    filament_engine_lib
)
gtest_discover_tests(test_clock)

# Input test links the full engine lib (needs Input implementation)
add_executable(test_input unit/test_input.cpp)
target_include_directories(test_input PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_input PRIVATE
    GTest::gtest_main
    filament_engine_lib
)
gtest_discover_tests(test_input)

# EntityBridge test links the full engine lib (needs Filament utils)
add_executable(test_entity_bridge unit/test_entity_bridge.cpp)
target_include_directories(test_entity_bridge PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_entity_bridge PRIVATE
    GTest::gtest_main
    filament_engine_lib
)
gtest_discover_tests(test_entity_bridge)

# ==============================
# Integration Tests (require Filament engine)
# ==============================

add_executable(test_filament_pipeline integration/test_filament_pipeline.cpp)
target_include_directories(test_filament_pipeline PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_filament_pipeline PRIVATE
    GTest::gtest_main
    filament_engine_lib
)
# Use add_test instead of gtest_discover_tests for integration tests.
# Discovery would try to run the binary at build time, which needs GPU and hangs.
add_test(
    NAME test_filament_pipeline
    COMMAND test_filament_pipeline
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/sandbox"
)
