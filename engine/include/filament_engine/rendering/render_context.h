#pragma once

#include <filament_engine/core/window.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/SwapChain.h>
#include <filament/Camera.h>
#include <filament/Skybox.h>
#include <filament/IndirectLight.h>
#include <filament/Texture.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <string>

namespace fe {

// Graphics backend selection
enum class GraphicsBackend {
    Vulkan,
    Metal,
    OpenGL,
    Default // Auto-detect: Metal on macOS, Vulkan on Linux/Windows
};

// Owns and manages all core Filament rendering objects
class RenderContext {
public:
    RenderContext(Window& window, GraphicsBackend backend = GraphicsBackend::Default);

    // Returns the preferred graphics backend for the current platform
    static GraphicsBackend getPlatformDefaultBackend();
    ~RenderContext();

    // Non-copyable
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    // Frame rendering
    bool beginFrame();
    void render();
    void endFrame();

    // Handle window resize
    void resize(int width, int height);

    // Accessors
    filament::Engine* getEngine() const { return m_engine; }
    filament::Renderer* getRenderer() const { return m_renderer; }
    filament::Scene* getScene() const { return m_scene; }
    filament::View* getView() const { return m_view; }
    filament::SwapChain* getSwapChain() const { return m_swapChain; }

    // Convenience: access Filament managers
    filament::TransformManager& getTransformManager() const;
    filament::RenderableManager& getRenderableManager() const;
    filament::LightManager& getLightManager() const;

    // Camera management
    filament::Camera* createCamera();
    void setActiveCamera(filament::Camera* camera);
    filament::Camera* getActiveCamera() const { return m_activeCamera; }

    // IBL: loads KTX cubemaps from a directory containing _ibl.ktx, _skybox.ktx, and sh.txt
    bool loadIBL(const std::string& iblDirectory);

private:
    void createSwapChain(Window& window);

    filament::Engine* m_engine = nullptr;
    filament::Renderer* m_renderer = nullptr;
    filament::Scene* m_scene = nullptr;
    filament::View* m_view = nullptr;
    filament::SwapChain* m_swapChain = nullptr;
    filament::Camera* m_activeCamera = nullptr;
    filament::Skybox* m_skybox = nullptr;
    filament::IndirectLight* m_indirectLight = nullptr;
    filament::Texture* m_iblTexture = nullptr;
    filament::Texture* m_skyboxTexture = nullptr;

    utils::Entity m_cameraEntity;
    Window& m_window;
};

} // namespace fe
