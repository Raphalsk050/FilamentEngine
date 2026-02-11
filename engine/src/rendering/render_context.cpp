#include <filament_engine/rendering/render_context.h>
#include <filament_engine/core/log.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/SwapChain.h>
#include <filament/Camera.h>
#include <filament/Skybox.h>
#include <filament/IndirectLight.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/RenderableManager.h>
#include <filament/LightManager.h>

#include <ktxreader/Ktx1Reader.h>

#include <utils/EntityManager.h>

#include <backend/DriverEnums.h>

#include <fstream>
#include <sstream>
#include <cstdlib>

// macOS native helpers (defined in native_window_cocoa.mm)
#if defined(__APPLE__)
extern "C" {
    void* fe_cocoa_get_native_view(void* nswindow);
    void  fe_cocoa_prepare_window(void* nswindow);
    void* fe_cocoa_setup_metal_layer(void* nativeView);
    void* fe_cocoa_resize_metal_layer(void* nativeView);
}
#endif

namespace fe {

GraphicsBackend RenderContext::getPlatformDefaultBackend() {
#if defined(__APPLE__)
    return GraphicsBackend::Metal;
#elif defined(_WIN32)
    return GraphicsBackend::Vulkan;
#elif defined(__linux__)
    return GraphicsBackend::Vulkan;
#else
    return GraphicsBackend::OpenGL;
#endif
}

static const char* backendToString(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::Vulkan:  return "Vulkan";
        case GraphicsBackend::Metal:   return "Metal";
        case GraphicsBackend::OpenGL:  return "OpenGL";
        case GraphicsBackend::Default: return "Default";
    }
    return "Unknown";
}

static filament::Engine::Backend toFilamentBackend(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::Vulkan:  return filament::Engine::Backend::VULKAN;
        case GraphicsBackend::Metal:   return filament::Engine::Backend::METAL;
        case GraphicsBackend::OpenGL:  return filament::Engine::Backend::OPENGL;
        case GraphicsBackend::Default: return filament::Engine::Backend::DEFAULT;
    }
    return filament::Engine::Backend::DEFAULT;
}

RenderContext::RenderContext(Window& window, GraphicsBackend backend)
    : m_window(window) {

    // Resolve Default to the platform-appropriate backend
    if (backend == GraphicsBackend::Default) {
        backend = getPlatformDefaultBackend();
        FE_LOG_INFO("Auto-detected platform backend: %s", backendToString(backend));
    }

    FE_LOG_INFO("Creating Filament engine with %s backend", backendToString(backend));

    // Create Filament engine
    m_engine = filament::Engine::Builder()
        .backend(toFilamentBackend(backend))
        .build();

    if (!m_engine) {
        FE_LOG_FATAL("Failed to create Filament engine");
        return;
    }

    // Create swap chain
    createSwapChain(window);

    // Create renderer
    m_renderer = m_engine->createRenderer();

    // Create scene
    m_scene = m_engine->createScene();

    // Create view
    m_view = m_engine->createView();
    m_view->setScene(m_scene);
    m_view->setViewport({0, 0,
        static_cast<uint32_t>(window.getWidth()),
        static_cast<uint32_t>(window.getHeight())});

    // Create a default camera
    m_cameraEntity = utils::EntityManager::get().create();
    m_activeCamera = m_engine->createCamera(m_cameraEntity);
    m_view->setCamera(m_activeCamera);

    // Default camera setup: perspective projection looking at origin
    const float aspect = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
    m_activeCamera->setProjection(60.0f, aspect, 0.1f, 1000.0f);
    m_activeCamera->lookAt({0, 2, 5}, {0, 0, 0}, {0, 1, 0});

    // Create a default skybox (dark color)
    m_skybox = filament::Skybox::Builder()
        .color({0.05f, 0.05f, 0.1f, 1.0f})
        .build(*m_engine);
    m_scene->setSkybox(m_skybox);

    // Configure shadow type — using DPCF (default) for stability on Metal/M3
    // NOTE: VSM (Variance Shadow Maps) crashes on Metal backend with Apple M3
    m_view->setShadowType(filament::View::ShadowType::DPCF);
    filament::View::DynamicResolutionOptions dro;
    dro.enabled = false; // avoid dynamic resolution for shadow buffer
    m_view->setDynamicResolutionOptions(dro);

    FE_LOG_INFO("RenderContext initialized successfully");
}

RenderContext::~RenderContext() {
    if (m_engine) {
        if (m_indirectLight) m_engine->destroy(m_indirectLight);
        if (m_iblTexture) m_engine->destroy(m_iblTexture);
        if (m_skyboxTexture) m_engine->destroy(m_skyboxTexture);
        if (m_skybox) m_engine->destroy(m_skybox);
        if (m_view) m_engine->destroy(m_view);
        if (m_scene) m_engine->destroy(m_scene);
        if (m_renderer) m_engine->destroy(m_renderer);
        if (m_swapChain) m_engine->destroy(m_swapChain);

        m_engine->destroyCameraComponent(m_cameraEntity);
        utils::EntityManager::get().destroy(m_cameraEntity);

        filament::Engine::destroy(&m_engine);
        m_engine = nullptr;
    }
    FE_LOG_INFO("RenderContext destroyed");
}

void RenderContext::createSwapChain(Window& window) {
    void* nativeWindow = nullptr;

#if defined(__APPLE__)
    // Get the NSWindow from SDL
    void* nswindow = window.getNativeWindow();
    fe_cocoa_prepare_window(nswindow);

    // Get the NSView and set up a CAMetalLayer for Metal backend
    void* nsview = fe_cocoa_get_native_view(nswindow);
    nativeWindow = fe_cocoa_setup_metal_layer(nsview);
#else
    nativeWindow = window.getNativeWindow();
#endif

    if (!nativeWindow) {
        FE_LOG_FATAL("Failed to get native window handle");
        return;
    }

    m_swapChain = m_engine->createSwapChain(nativeWindow);
    if (!m_swapChain) {
        FE_LOG_FATAL("Failed to create SwapChain");
    }
}

bool RenderContext::beginFrame() {
    if (!m_renderer || !m_swapChain) return false;
    return m_renderer->beginFrame(m_swapChain);
}

void RenderContext::render() {
    if (m_renderer && m_view) {
        m_renderer->render(m_view);
    }
}

void RenderContext::endFrame() {
    if (m_renderer) {
        m_renderer->endFrame();
    }
}

void RenderContext::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;

    m_view->setViewport({0, 0,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)});

    if (m_activeCamera) {
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        m_activeCamera->setProjection(60.0f, aspect, 0.1f, 1000.0f);
    }

    // Recreate swap chain on resize
    if (m_swapChain) {
        m_engine->destroy(m_swapChain);
    }
    createSwapChain(m_window);
}

filament::TransformManager& RenderContext::getTransformManager() const {
    return m_engine->getTransformManager();
}

filament::RenderableManager& RenderContext::getRenderableManager() const {
    return m_engine->getRenderableManager();
}

filament::LightManager& RenderContext::getLightManager() const {
    return m_engine->getLightManager();
}

filament::Camera* RenderContext::createCamera() {
    auto entity = utils::EntityManager::get().create();
    return m_engine->createCamera(entity);
}

void RenderContext::setActiveCamera(filament::Camera* camera) {
    m_activeCamera = camera;
    if (m_view && camera) {
        m_view->setCamera(camera);
    }
}

// Helper: read entire file into a byte vector
static std::vector<uint8_t> readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

// Helper: parse spherical harmonics from sh.txt
static bool parseSH(const std::string& path, filament::math::float3 sh[9]) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    int index = 0;
    while (std::getline(file, line) && index < 9) {
        // Format: ( x, y, z); // comment
        // Find the parenthesized values
        auto openParen = line.find('(');
        auto closeParen = line.find(')');
        if (openParen == std::string::npos || closeParen == std::string::npos) continue;

        std::string values = line.substr(openParen + 1, closeParen - openParen - 1);
        // Replace commas with spaces for easier parsing
        for (auto& c : values) {
            if (c == ',') c = ' ';
        }

        std::istringstream iss(values);
        float x, y, z;
        if (iss >> x >> y >> z) {
            sh[index] = {x, y, z};
            index++;
        }
    }
    return index == 9;
}

bool RenderContext::loadIBL(const std::string& iblDirectory) {
    // Find KTX files in the directory
    // cmgen outputs: <name>_ibl.ktx, <name>_skybox.ktx, sh.txt
    std::string iblPath;
    std::string skyboxPath;
    std::string shPath = iblDirectory + "/sh.txt";

    // Try to find the KTX files by scanning for *_ibl.ktx and *_skybox.ktx patterns
    // For simplicity, try common patterns
    std::vector<std::string> possibleNames = {"ibl", "lightroom_14b"};

    for (const auto& name : possibleNames) {
        std::string testIbl = iblDirectory + "/" + name + "_ibl.ktx";
        std::string testSky = iblDirectory + "/" + name + "_skybox.ktx";
        std::ifstream f1(testIbl), f2(testSky);
        if (f1.good() && f2.good()) {
            iblPath = testIbl;
            skyboxPath = testSky;
            break;
        }
    }

    if (iblPath.empty() || skyboxPath.empty()) {
        FE_LOG_ERROR("IBL KTX files not found in: %s", iblDirectory.c_str());
        return false;
    }

    // Load IBL cubemap
    auto iblData = readBinaryFile(iblPath);
    if (iblData.empty()) {
        FE_LOG_ERROR("Failed to read IBL KTX: %s", iblPath.c_str());
        return false;
    }

    auto* iblBundle = new image::Ktx1Bundle(iblData.data(), static_cast<uint32_t>(iblData.size()));
    m_iblTexture = ktxreader::Ktx1Reader::createTexture(m_engine, iblBundle, false);
    if (!m_iblTexture) {
        FE_LOG_ERROR("Failed to create IBL texture from: %s", iblPath.c_str());
        return false;
    }

    // Load skybox cubemap
    auto skyboxData = readBinaryFile(skyboxPath);
    if (skyboxData.empty()) {
        FE_LOG_ERROR("Failed to read skybox KTX: %s", skyboxPath.c_str());
        return false;
    }

    auto* skyBundle = new image::Ktx1Bundle(skyboxData.data(), static_cast<uint32_t>(skyboxData.size()));
    m_skyboxTexture = ktxreader::Ktx1Reader::createTexture(m_engine, skyBundle, false);
    if (!m_skyboxTexture) {
        FE_LOG_ERROR("Failed to create skybox texture from: %s", skyboxPath.c_str());
        return false;
    }

    // Parse spherical harmonics
    filament::math::float3 sh[9];
    if (!parseSH(shPath, sh)) {
        FE_LOG_ERROR("Failed to parse SH from: %s", shPath.c_str());
        return false;
    }

    // Create IndirectLight
    if (m_indirectLight) {
        m_engine->destroy(m_indirectLight);
    }
    m_indirectLight = filament::IndirectLight::Builder()
        .reflections(m_iblTexture)
        .irradiance(3, sh)
        .intensity(30000.0f)
        .build(*m_engine);
    m_scene->setIndirectLight(m_indirectLight);

    // Replace skybox with the IBL skybox
    if (m_skybox) {
        m_engine->destroy(m_skybox);
    }
    m_skybox = filament::Skybox::Builder()
        .environment(m_skyboxTexture)
        .build(*m_engine);
    m_scene->setSkybox(m_skybox);

    FE_LOG_INFO("IBL loaded successfully from: %s", iblDirectory.c_str());
    return true;
}

} // namespace fe
