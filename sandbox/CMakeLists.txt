# Sandbox executable

# Include the materials compilation module
include(${CMAKE_SOURCE_DIR}/cmake/FilamentMaterials.cmake)

# Compile all .mat files in the materials/ directory
compile_materials(
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/materials"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/materials"
    TARGET_NAME sandbox_materials
)

# Sandbox target
add_executable(sandbox main.cpp)

target_link_libraries(sandbox PRIVATE filament_engine_lib)

add_dependencies(sandbox sandbox_materials)

# Copy IBL assets to build directory so sandbox can find them at runtime
set(IBL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/sandbox/assets/ibl")
set(IBL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets/ibl")

if(EXISTS "${IBL_SOURCE_DIR}")
    add_custom_command(TARGET sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${IBL_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${IBL_SOURCE_DIR}" "${IBL_OUTPUT_DIR}"
        COMMENT "Copying IBL assets to build directory"
    )
endif()
