# ==============================================================================
# FilamentMaterials.cmake
#
# Reusable CMake module for compiling Filament material files (.mat → .filamat).
#
# Usage:
#   include(cmake/FilamentMaterials.cmake)
#
#   # Option 1: Compile a single material
#   compile_material(
#       SOURCE materials/standard_lit.mat
#       OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/materials
#   )
#
#   # Option 2: Compile all materials in a directory
#   compile_materials(
#       SOURCE_DIR materials
#       OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/materials
#       TARGET_NAME my_materials
#   )
#
#   # Then add the target as a dependency
#   add_dependencies(my_app my_materials)
# ==============================================================================

# Find the matc tool from Filament distribution
if(NOT DEFINED MATC_EXECUTABLE)
    set(MATC_EXECUTABLE "${FILAMENT_DIST_DIR}/bin/matc" CACHE FILEPATH
        "Path to the Filament material compiler (matc)")
endif()

if(NOT EXISTS "${MATC_EXECUTABLE}")
    message(FATAL_ERROR
        "Filament material compiler (matc) not found at: ${MATC_EXECUTABLE}\n"
        "Make sure FILAMENT_DIST_DIR is set correctly.")
endif()

# Default graphics API backends to compile for.
# On macOS we need Metal; on Linux/Windows we need Vulkan + OpenGL.
if(NOT DEFINED MATC_API_FLAGS)
    if(APPLE)
        set(MATC_API_FLAGS -a metal -a opengl CACHE STRING
            "Graphics API backends for material compilation")
    elseif(WIN32)
        set(MATC_API_FLAGS -a vulkan -a opengl CACHE STRING
            "Graphics API backends for material compilation")
    else()
        set(MATC_API_FLAGS -a vulkan -a opengl CACHE STRING
            "Graphics API backends for material compilation")
    endif()
endif()

# --------------------------------------------------------------------------
# compile_material(<SOURCE> <OUTPUT_DIR> [RESULT_VAR <var>])
#
# Adds a custom command to compile a single .mat file to .filamat.
# Sets <RESULT_VAR> (if provided) to the absolute output path.
# --------------------------------------------------------------------------
function(compile_material)
    cmake_parse_arguments(ARG "" "SOURCE;OUTPUT_DIR;RESULT_VAR" "" ${ARGN})

    if(NOT ARG_SOURCE)
        message(FATAL_ERROR "compile_material: SOURCE is required")
    endif()
    if(NOT ARG_OUTPUT_DIR)
        message(FATAL_ERROR "compile_material: OUTPUT_DIR is required")
    endif()

    # Resolve absolute path
    get_filename_component(MAT_ABSOLUTE "${ARG_SOURCE}" ABSOLUTE)
    get_filename_component(MAT_NAME_WE "${ARG_SOURCE}" NAME_WE)

    set(OUTPUT_FILE "${ARG_OUTPUT_DIR}/${MAT_NAME_WE}.filamat")

    file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")

    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND "${MATC_EXECUTABLE}" ${MATC_API_FLAGS}
                -o "${OUTPUT_FILE}" "${MAT_ABSOLUTE}"
        DEPENDS "${MAT_ABSOLUTE}"
        COMMENT "Compiling material: ${MAT_NAME_WE}.mat → ${MAT_NAME_WE}.filamat"
        VERBATIM
    )

    # Export the output path to the caller
    if(ARG_RESULT_VAR)
        set(${ARG_RESULT_VAR} "${OUTPUT_FILE}" PARENT_SCOPE)
    endif()
endfunction()

# --------------------------------------------------------------------------
# compile_materials(
#     SOURCE_DIR <dir>
#     OUTPUT_DIR <dir>
#     TARGET_NAME <target>
#     [API_FLAGS <flags...>]
# )
#
# Finds all .mat files in SOURCE_DIR, compiles each one, and creates a
# custom target that depends on all compiled outputs. Any target that
# needs the compiled materials should add_dependencies() on TARGET_NAME.
# --------------------------------------------------------------------------
function(compile_materials)
    cmake_parse_arguments(ARG "" "SOURCE_DIR;OUTPUT_DIR;TARGET_NAME" "API_FLAGS" ${ARGN})

    if(NOT ARG_SOURCE_DIR)
        message(FATAL_ERROR "compile_materials: SOURCE_DIR is required")
    endif()
    if(NOT ARG_OUTPUT_DIR)
        message(FATAL_ERROR "compile_materials: OUTPUT_DIR is required")
    endif()
    if(NOT ARG_TARGET_NAME)
        message(FATAL_ERROR "compile_materials: TARGET_NAME is required")
    endif()

    # Allow per-call API flag override
    if(ARG_API_FLAGS)
        set(_API_FLAGS ${ARG_API_FLAGS})
    else()
        set(_API_FLAGS ${MATC_API_FLAGS})
    endif()

    # Find all .mat files
    file(GLOB MAT_FILES "${ARG_SOURCE_DIR}/*.mat")

    if(NOT MAT_FILES)
        message(WARNING "compile_materials: No .mat files found in ${ARG_SOURCE_DIR}")
    endif()

    set(ALL_OUTPUTS "")

    foreach(MAT_FILE ${MAT_FILES})
        get_filename_component(MAT_NAME_WE "${MAT_FILE}" NAME_WE)
        set(OUTPUT_FILE "${ARG_OUTPUT_DIR}/${MAT_NAME_WE}.filamat")

        file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")

        add_custom_command(
            OUTPUT "${OUTPUT_FILE}"
            COMMAND "${MATC_EXECUTABLE}" ${_API_FLAGS}
                    -o "${OUTPUT_FILE}" "${MAT_FILE}"
            DEPENDS "${MAT_FILE}"
            COMMENT "Compiling material: ${MAT_NAME_WE}.mat → ${MAT_NAME_WE}.filamat"
            VERBATIM
        )

        list(APPEND ALL_OUTPUTS "${OUTPUT_FILE}")
    endforeach()

    # Create a target that depends on all compiled materials
    add_custom_target(${ARG_TARGET_NAME} ALL DEPENDS ${ALL_OUTPUTS})

    # Log summary
    list(LENGTH ALL_OUTPUTS NUM_MATERIALS)
    message(STATUS "Materials: ${NUM_MATERIALS} file(s) in ${ARG_SOURCE_DIR} → ${ARG_OUTPUT_DIR}")
endfunction()
