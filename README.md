# Filament Engine

A lightweight 3D engine built on top of [Google Filament](https://github.com/google/filament) with an ECS architecture powered by [EnTT](https://github.com/skypjack/entt).

The idea is to have a thin layer that handles window management, input, scene hierarchy, and PBR rendering while keeping the flexibility of Filament's low-level API available when you need it.

## What it does

- **ECS (Entity-Component-System)**: entities are EnTT handles with components like `TransformComponent`, `MeshRendererComponent`, `CameraComponent`, and `LightComponent`. Systems run each frame to sync transforms, build renderables, update cameras, and push lights to Filament.
- **PBR rendering**: materials are compiled from `.mat` files (Filament's material format) and support `baseColor`, `metallic`, `roughness`, and `reflectance`. IBL-based lighting is loaded from KTX cubemaps.
- **Platform abstraction**: SDL2 handles windowing and input. Graphics backend is auto-selected per platform (Metal on macOS, Vulkan on Linux).
- **Editor camera**: built-in FPS-style camera with WASD + mouse look for quick scene inspection.

## Project layout

| Directory | What's in there |
|---|---|
| `engine/` | Core library. Headers in `include/filament_engine/`, implementation in `src/`. Organized by subsystem: `core/`, `ecs/`, `rendering/`, `resources/`. |
| `sandbox/` | Demo app — a rotating PBR cube with shadows and IBL lighting. |
| `materials/` | Filament material source files (`.mat`). Compiled to `.filamat` during build. |
| `tests/` | Unit tests (GTest) and integration tests. |
| `scripts/` | `setup.sh` (dependency setup), `compile_materials.sh` (standalone matc wrapper). |
| `cmake/` | CMake modules, including the material compilation rules. |
| `vendor/` | Third-party deps (gitignored, populated by `setup.sh`). |

## Requirements

- **macOS** (Apple Silicon or Intel) or **Linux**
- CMake ≥ 3.22
- Ninja
- C++20 compiler (Clang 15+ / GCC 12+)
- SDL2 (installed via Homebrew on macOS or apt on Linux)

## Getting started

```bash
# 1. Clone
git clone https://github.com/Raphalsk050/FilamentEngine.git
cd FilamentEngine

# 2. Run setup (downloads Filament, clones EnTT, configures CMake)
./scripts/setup.sh

# 3. Build
cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

# 4. Run the sandbox
./build/sandbox/sandbox
```

The setup script downloads a pre-built Filament release (~45 MB) so you don't need to build it from source. It also compiles all `.mat` files in `materials/` during the build step.

## Materials

Material files live in `materials/` and use Filament's [material format](https://google.github.io/filament/Materials.html). They're compiled to `.filamat` binaries automatically when you build.

If you want to recompile materials without a full rebuild:

```bash
./scripts/compile_materials.sh              # all materials
./scripts/compile_materials.sh standard_lit # just one
```

To add a new material, just drop a `.mat` file in `materials/` — CMake picks it up on the next configure.

## How the ECS works

Every entity gets a `TransformComponent` and a `TagComponent` by default. You attach rendering-related components to make things visible:

```cpp
auto entity = world.createEntity("MyCube");

auto& renderer = world.addComponent<fe::MeshRendererComponent>(entity);
renderer.mesh = myMeshHandle;
renderer.material = myMaterialHandle;

auto& transform = world.getComponent<fe::TransformComponent>(entity);
transform.position = {0, 2, 0};
transform.dirty = true;
```

Systems run in priority order each frame:
1. `TransformSyncSystem` — pushes dirty transforms to Filament's TransformManager
2. `RenderSyncSystem` — builds Filament renderables from MeshRendererComponents
3. `LightSystem` — creates/updates Filament lights
4. `CameraSystem` — syncs the active camera
5. `EditorCameraSystem` — handles FPS camera controls

The `EntityBridge` keeps a bidirectional map between EnTT entities and Filament entities, so the systems can translate between the two worlds.

## Writing a new app

Subclass `fe::Application`:

```cpp
#include <filament_engine/filament_engine.h>

class MyApp : public fe::Application {
public:
    MyApp() : fe::Application(makeConfig()) {}

    void onInit() override {
        // Set up your scene here
    }

    void onUpdate(float dt) override {
        // Called every frame
    }

    void onShutdown() override {
        // Cleanup
    }

private:
    static fe::ApplicationConfig makeConfig() {
        fe::ApplicationConfig cfg;
        cfg.window.title = "My App";
        cfg.window.width = 1280;
        cfg.window.height = 720;
        cfg.backend = fe::GraphicsBackend::Default;
        return cfg;
    }
};

int main() {
    MyApp app;
    app.run();
}
```

## Tests

Unit tests use Google Test (fetched via CMake's FetchContent):

```bash
cd build && ctest --output-on-failure
```

There are ~111 tests covering components, entity bridge, input, clock, event bus, math utilities, and resource handles. Integration tests that need a GPU are excluded from the default test run.

## Dependencies

| Dependency | Version | How it's used |
|---|---|---|
| [Google Filament](https://github.com/google/filament) | v1.69.2 | Rendering backend (pre-built, downloaded by setup.sh) |
| [EnTT](https://github.com/skypjack/entt) | v3.16.0 | ECS registry (header-only, cloned by setup.sh) |
| [SDL2](https://www.libsdl.org/) | System | Windowing and input |
| [Google Test](https://github.com/google/googletest) | v1.14.0 | Testing (fetched by CMake) |

## License

See the individual dependency licenses. The engine code in `engine/` and `sandbox/` is available under the MIT license.
