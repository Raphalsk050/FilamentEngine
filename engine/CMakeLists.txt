set(ENGINE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# SDL2 paths (from Filament's third_party source and intermediate build)
set(SDL2_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/filament/third_party/libsdl2/include")
set(SDL2_BUILD_LIB "${CMAKE_SOURCE_DIR}/vendor/filament/out/cmake-release/third_party/libsdl2/tnt/libsdl2.a")

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

target_include_directories(filament_engine_lib PUBLIC
    "${ENGINE_INCLUDE_DIR}"
    "${FILAMENT_DIST_DIR}/include"
    "${SDL2_INCLUDE_DIR}"
)

target_link_libraries(filament_engine_lib PUBLIC
    entt
)

# Link pre-built Filament static libraries
set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/arm64")

# Check for lib/arm64 (Apple Silicon) or lib/x86_64 or just lib/
if(NOT EXISTS "${FILAMENT_LIB_DIR}")
    set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib/x86_64")
    if(NOT EXISTS "${FILAMENT_LIB_DIR}")
        set(FILAMENT_LIB_DIR "${FILAMENT_DIST_DIR}/lib")
    endif()
endif()

message(STATUS "Filament lib dir: ${FILAMENT_LIB_DIR}")

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
link_filament_lib(utils)
link_filament_lib(geometry)
link_filament_lib(filabridge)
link_filament_lib(filaflat)
link_filament_lib(ibl)
link_filament_lib(image)
link_filament_lib(ktxreader)
link_filament_lib(camutils)
link_filament_lib(shaders)

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

# SDL2 (from Filament's intermediate build)
if(EXISTS "${SDL2_BUILD_LIB}")
    target_link_libraries(filament_engine_lib PUBLIC "${SDL2_BUILD_LIB}")
    message(STATUS "  Found: SDL2 at ${SDL2_BUILD_LIB}")
else()
    # Fallback: try system SDL2
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        target_link_libraries(filament_engine_lib PUBLIC SDL2::SDL2)
    else()
        message(FATAL_ERROR "SDL2 not found. Please build Filament first or install SDL2.")
    endif()
endif()

# Platform-specific frameworks
if(APPLE)
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
