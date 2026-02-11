# Sandbox executable

set(MATC_TOOL "${FILAMENT_DIST_DIR}/bin/matc")

# Material compilation setup
set(MATERIAL_DIR "${CMAKE_SOURCE_DIR}/materials")
set(MATERIAL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/materials")
file(MAKE_DIRECTORY "${MATERIAL_OUTPUT_DIR}")

# Compile standard_lit.mat → standard_lit.filamat
set(STANDARD_LIT_MAT "${MATERIAL_DIR}/standard_lit.mat")
set(STANDARD_LIT_FILAMAT "${MATERIAL_OUTPUT_DIR}/standard_lit.filamat")

add_custom_command(
    OUTPUT "${STANDARD_LIT_FILAMAT}"
    COMMAND "${MATC_TOOL}" -a vulkan -a opengl -a metal -o "${STANDARD_LIT_FILAMAT}" "${STANDARD_LIT_MAT}"
    DEPENDS "${STANDARD_LIT_MAT}"
    COMMENT "Compiling material: standard_lit.mat"
    VERBATIM
)

add_custom_target(sandbox_materials DEPENDS "${STANDARD_LIT_FILAMAT}")

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

