# Tests directory
enable_testing()

# Include paths for test helpers
set(TEST_HELPERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# ==============================
# Unit Tests (no Filament engine needed at runtime)
# ==============================

# Test: ResourceHandle
add_executable(test_resource_handle unit/test_resource_handle.cpp)
target_include_directories(test_resource_handle PRIVATE
    "${TEST_HELPERS_DIR}"
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_resource_handle PRIVATE entt)
add_test(NAME unit_resource_handle COMMAND test_resource_handle)

# Test: Components
add_executable(test_components unit/test_components.cpp)
target_include_directories(test_components PRIVATE
    "${TEST_HELPERS_DIR}"
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_components PRIVATE entt)
add_test(NAME unit_components COMMAND test_components)

# Test: Math Utils
add_executable(test_math_utils unit/test_math_utils.cpp)
target_include_directories(test_math_utils PRIVATE
    "${TEST_HELPERS_DIR}"
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_math_utils PRIVATE entt)
add_test(NAME unit_math_utils COMMAND test_math_utils)

# Test: Input
add_executable(test_input unit/test_input.cpp)
target_include_directories(test_input PRIVATE
    "${TEST_HELPERS_DIR}"
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_input PRIVATE filament_engine_lib)
add_test(NAME unit_input COMMAND test_input)

# ==============================
# Integration Tests (require Filament engine)
# ==============================

# Test: Filament Pipeline
add_executable(test_filament_pipeline integration/test_filament_pipeline.cpp)
target_include_directories(test_filament_pipeline PRIVATE
    "${TEST_HELPERS_DIR}"
    "${CMAKE_SOURCE_DIR}/engine/include"
    "${FILAMENT_DIST_DIR}/include"
)
target_link_libraries(test_filament_pipeline PRIVATE filament_engine_lib)
add_test(
    NAME integration_filament_pipeline
    COMMAND test_filament_pipeline
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/sandbox"
)
