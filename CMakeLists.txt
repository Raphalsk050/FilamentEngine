cmake_minimum_required(VERSION 3.22)
project(filament_engine VERSION 0.1.0 LANGUAGES C CXX)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Objective-C++ on macOS
if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 20)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
endif()

# Filament pre-built distribution directory
set(FILAMENT_DIST_DIR "${CMAKE_SOURCE_DIR}/vendor/filament/out/release/filament"
    CACHE PATH "Path to pre-built Filament distribution")

if(NOT EXISTS "${FILAMENT_DIST_DIR}")
    message(FATAL_ERROR
        "Filament distribution not found at: ${FILAMENT_DIST_DIR}\n"
        "Please build Filament first:\n"
        "  cd vendor/filament && ./build.sh -i release")
endif()

# EnTT (header-only)
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE
    "${CMAKE_SOURCE_DIR}/vendor/entt/single_include")

# Engine library
add_subdirectory(engine)

# Sandbox application
add_subdirectory(sandbox)

# Tests
enable_testing()
add_subdirectory(tests)
