set(ENGINE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Collect sources
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Collect Objective-C++ sources on macOS
if(APPLE)
    file(GLOB_RECURSE ENGINE_OBJCXX_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm"
    )
    list(APPEND ENGINE_SOURCES ${ENGINE_OBJCXX_SOURCES})
endif()

# Engine static library
add_library(filament_engine_lib STATIC ${ENGINE_SOURCES})

# ==============================================================================
# SDL2
# ==============================================================================
if(WIN32)
    set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/vendor/SDL2")
    set(SDL2_INCLUDE_DIR "${SDL2_ROOT}/include")
    set(SDL2_LIB_DIR "${SDL2_ROOT}/lib/x64")
else()
    # Unix: SDL2 installed via system package manager (apt / brew)
    find_package(SDL2 REQUIRED)
endif()

# ==============================================================================
# Include directories
# ==============================================================================
target_include_directories(filament_engine_lib PUBLIC
    "${ENGINE_INCLUDE_DIR}"
    "${FILAMENT_DIST_DIR}/include"
)

if(WIN32)
    target_include_directories(filament_engine_lib PUBLIC "${SDL2_INCLUDE_DIR}")
else()
    if(TARGET SDL2::SDL2)
        # Modern CMake target (SDL2 >= 2.0.12 with CMake config)
    else()
        target_include_directories(filament_engine_lib PUBLIC ${SDL2_INCLUDE_DIRS})
    endif()
endif()

target_link_libraries(filament_engine_lib PUBLIC
    entt
)

# ==============================================================================
# Filament static libraries
# ==============================================================================
set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/arm64")

# Detect correct library directory per platform/arch
if(NOT EXISTS "${FILAMENT_LIB_DIR}")
    if(WIN32)
        # Windows Filament has CRT variants: md (dynamic /MD), mt (static /MT)
        # Default MSVC uses /MD, so use md for Release, mdd for Debug
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(_FILAMENT_CRT_SUFFIX "mdd")
        else()
            set(_FILAMENT_CRT_SUFFIX "md")
        endif()
        set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/x86_64/${_FILAMENT_CRT_SUFFIX}")

        if(NOT EXISTS "${FILAMENT_LIB_DIR}")
            # Fallback: try the other variant
            if(_FILAMENT_CRT_SUFFIX STREQUAL "md")
                set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/x86_64/mt")
            else()
                set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/x86_64/mtd")
            endif()
        endif()
    else()
        set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/x86_64")
    endif()

    if(NOT EXISTS "${FILAMENT_LIB_DIR}")
        set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib")
    endif()
endif()

message(STATUS "Filament lib dir: ${FILAMENT_LIB_DIR}")

if(NOT EXISTS "${FILAMENT_LIB_DIR}")
    message(FATAL_ERROR "Filament library directory not found: ${FILAMENT_LIB_DIR}")
endif()

# Helper macro to find and link a Filament library
macro(link_filament_lib LIB_NAME)
    find_library(FILAMENT_${LIB_NAME}_LIB
        NAMES ${LIB_NAME}
        PATHS "${FILAMENT_LIB_DIR}"
        NO_DEFAULT_PATH
    )
    if(FILAMENT_${LIB_NAME}_LIB)
        target_link_libraries(filament_engine_lib PUBLIC "${FILAMENT_${LIB_NAME}_LIB}")
        message(STATUS "  Found: ${LIB_NAME}")
    else()
        message(WARNING "  NOT found: ${LIB_NAME}")
    endif()
endmacro()

# Core Filament libraries (order matters for static linking!)
link_filament_lib(filament)
link_filament_lib(backend)
link_filament_lib(geometry)
link_filament_lib(filabridge)
link_filament_lib(filaflat)
link_filament_lib(ibl)
link_filament_lib(image)
link_filament_lib(ktxreader)
link_filament_lib(camutils)
link_filament_lib(shaders)
link_filament_lib(utils)

# Vulkan support
link_filament_lib(bluevk)
link_filament_lib(smol-v)

# OpenGL support
link_filament_lib(bluegl)

# Support libraries
link_filament_lib(matdbg)
link_filament_lib(civetweb)
link_filament_lib(uberzlib)
link_filament_lib(uberarchive)
link_filament_lib(zstd)
link_filament_lib(abseil)
link_filament_lib(perfetto)

# ==============================================================================
# SDL2 linking
# ==============================================================================
if(WIN32)
    target_link_libraries(filament_engine_lib PUBLIC
        "${SDL2_LIB_DIR}/SDL2.lib"
        "${SDL2_LIB_DIR}/SDL2main.lib"
    )
    message(STATUS "  Found: SDL2 at ${SDL2_LIB_DIR}")
else()
    if(TARGET SDL2::SDL2)
        target_link_libraries(filament_engine_lib PUBLIC SDL2::SDL2)
    else()
        target_link_libraries(filament_engine_lib PUBLIC ${SDL2_LIBRARIES})
    endif()
endif()

# ==============================================================================
# Platform-specific system libraries and definitions
# ==============================================================================
if(WIN32)
    target_compile_definitions(filament_engine_lib PUBLIC
        _USE_MATH_DEFINES
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )

    target_link_libraries(filament_engine_lib PUBLIC
        opengl32
        gdi32
        user32
        shlwapi
        dwmapi
        ws2_32
    )
elseif(APPLE)
    target_link_libraries(filament_engine_lib PUBLIC
        "-framework Cocoa"
        "-framework QuartzCore"
        "-framework Metal"
        "-framework CoreVideo"
        "-framework IOKit"
        "-framework IOSurface"
        "-framework CoreGraphics"
        "-framework AppKit"
        "-framework Carbon"
        "-framework ForceFeedback"
        "-framework GameController"
        "-framework CoreHaptics"
        "-framework AudioToolbox"
        "-framework CoreAudio"
    )
    # MoltenVK / Vulkan — bluevk loads Vulkan dynamically, so we don't need to link libvulkan
endif()

# Compile definitions
target_compile_definitions(filament_engine_lib PUBLIC
    FILAMENT_ENGINE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    FILAMENT_ENGINE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    FILAMENT_ENGINE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)
