#include "quarkgl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


#include "../imguizmo_quat/imGuIZMOquat.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"


//ABSL_FLAG(std::string, model, "", "Path to a model file");

const char* lampShaderSource = R"SHADER(
#version 460 core
out vec4 fragColor;

void main() { fragColor = vec4(1.0); }
)SHADER";

const char* normalShaderSource = R"SHADER(
#version 460 core
out vec4 fragColor;

void main() { fragColor = vec4(1.0, 1.0, 0.0, 1.0); }
)SHADER";

enum class CameraControlType
{
    FLY = 0,
    ORBIT,
};

enum class LightingModel 
{
    BLINN_PHONG = 0,
    COOK_TORRANCE_GGX,
};

enum class SkyboxImage
{
    ALEXS_APT = 0,
    FROZEN_WATERFALL,
    KLOPPENHEIM,
    MILKYWAY,
    MON_VALLEY,
    UENO_SHRINE,
    WINTER_FOREST,
};

enum class GBufferVis 
{
    DISABLED = 0,
    POSITIONS,
    AO,
    NORMALS,
    ROUGHNESS,
    ALBEDO,
    METALLIC,
    EMISSION,
};

enum class ToneMapping 
{
    NONE = 0,
    REINHARD,
    REINHARD_LUMINANCE,
    ACES_APPROX,
    AMD,
};

// Options for the model render UI. The defaults here are used at startup.
struct ModelRenderOptions
{
    // Model.
    glm::quat modelRotation = glm::identity<glm::quat>();
    float modelScale = 1.0f;

    // Rendering.
    LightingModel lightingModel = LightingModel::COOK_TORRANCE_GGX;

    glm::vec3 directionalDiffuse = glm::vec3(0.5f);
    glm::vec3 directionalSpecular = glm::vec3(0.5f);
    float directionalIntensity = 10.0f;
    glm::vec3 directionalDirection =  glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));

    bool shadowMapping = false;
    float shadowCameraCuboidExtents = 2.0f;
    float shadowCameraNear = 0.1f;
    float shadowCameraFar = 15.0f;
    float shadowCameraDistance = 5.0f;
    float shadowBiasMin = 0.0001;
    float shadowBiasMax = 0.001;

    SkyboxImage skyboxImage = SkyboxImage::WINTER_FOREST;

    bool useIBL = true;
    glm::vec3 ambientColor = glm::vec3(0.1f);
    bool ssao = false;
    float ssaoRadius = 0.5f;
    float ssaoBias = 0.025f;
    float shininess = 32.0f;
    float emissionIntensity = 5.0f;
    glm::vec3 emissionAttenuation = glm::vec3(0, 0, 1.0f);

    bool bloom = true;
    float bloomMix = 0.004;
    ToneMapping toneMapping = ToneMapping::ACES_APPROX;
    bool gammaCorrect = true;
    float gamma = 2.2f;

    bool fxaa = true;

    // Camera.
    CameraControlType cameraControlType = CameraControlType::ORBIT;
    float speed = 0;
    float sensitivity = 0;
    float fov = 0;
    float near = 0;
    float far = 0;
    bool captureMouse = false;

    // Debug.
    GBufferVis gBufferVis = GBufferVis::DISABLED;
    bool wireframe = false;
    bool drawNormals = false;

    // Performance.
    const float* frameDeltas = nullptr;
    int numFrameDeltas = 0;
    int frameDeltasOffset = 0;
    float avgFPS = 0;
    bool enableVsync = true;
};

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void imguiHelpMarker(const char* desc) 
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) 
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

enum class Scale
{
    LINEAR = 0,
    LOG,
};

// Helper for a float slider value.
static bool imguiFloatSlider(const char* desc, float* value, float min,
    float max, const char* fmt = nullptr,
    Scale scale = Scale::LINEAR) 
{
    ImGuiSliderFlags flags = ImGuiSliderFlags_None;
    if (scale == Scale::LOG)
    {
        flags = ImGuiSliderFlags_Logarithmic;
    }
    return ImGui::SliderScalar(desc, ImGuiDataType_Float, value, &min, &max, fmt, flags);
}

// Helper for an image control.
static void imguiImage(const Cme::Texture& texture, glm::vec2 size)
{
    ImTextureID texID = reinterpret_cast<void*>(texture.getId());
    // Flip the image.
    ImGui::Image(texID, ImVec2(size.x, size.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
    //ImGui::Image(texID, size, /*uv0=*/glm::vec2(0.0f, 1.0f),
}

// Non-normative context for UI rendering. Used for accessing renderer info.
struct UIContext 
{
    Cme::Camera& camera;
    Cme::ShadowMap& shadowMap;
    Cme::SsaoBuffer& ssaoBuffer;
};

// Called during game loop.
void renderImGuiUI(ModelRenderOptions& opts, Cme::Camera camera, Cme::ShadowMap shadowMap, Cme::SsaoBuffer ssaoBuffer)
{
    // ImGui::ShowDemoWindow();

    ImGui::Begin("Model Render");

    constexpr float IMAGE_BASE_SIZE = 160.0f;

    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Perform some shenanigans so that the gizmo rotates along with the
        // camera while still representing the same model rotation.
        glm::quat rotViewSpace =  glm::quat_cast(camera.getViewTransform()) * opts.modelRotation;
        ImGui::gizmo3D("Model rotation", rotViewSpace, IMAGE_BASE_SIZE);
        opts.modelRotation =
            glm::quat_cast(glm::inverse(camera.getViewTransform())) *
            glm::normalize(rotViewSpace);

        ImGui::SameLine();

        // Perform some shenanigans so that the gizmo rotates along with the
        // camera while still representing the same light dir..
        glm::vec3 dirViewSpace =
            glm::vec3(camera.getViewTransform() *
                glm::vec4(opts.directionalDirection, 0.0f));
        ImGui::gizmo3D("Light dir", dirViewSpace, IMAGE_BASE_SIZE);
        opts.directionalDirection =
            glm::vec3(glm::inverse(camera.getViewTransform()) *
                glm::vec4(glm::normalize(dirViewSpace), 0.0f));
        ImGui::SliderFloat3("Light dir",
            reinterpret_cast<float*>(&opts.directionalDirection),
            -1.0f, 1.0f);

        if (ImGui::Button("Reset rotation")) 
        {
            opts.modelRotation = glm::identity<glm::quat>();
        }
        imguiFloatSlider("Model scale", &opts.modelScale, 0.0001f, 100.0f, "%.04f",
            Scale::LOG);
    }

    ImGui::Separator();

    // Create a child so that this section can scroll separately.
    ImGui::BeginChild("MainOptions");

    if (ImGui::CollapsingHeader("Rendering"))
    {
        ImGui::Combo("Lighting model", reinterpret_cast<int*>(&opts.lightingModel),
            "Blinn-Phong\0Cook-Torrance GGX\0\0");
        ImGui::SameLine();
        imguiHelpMarker("Which lighting model to use for shading.");

        ImGui::Separator();
        if (ImGui::TreeNode("Directional light"))
        {
            static bool lockSpecular = true;
            ImGui::ColorEdit3("Diffuse color",
                reinterpret_cast<float*>(&opts.directionalDiffuse),
                ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::BeginDisabled(lockSpecular);
            ImGui::ColorEdit3("Specular color",
                reinterpret_cast<float*>(&opts.directionalSpecular),
                ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::EndDisabled();
            ImGui::Checkbox("Lock specular", &lockSpecular);
            ImGui::SameLine();
            imguiHelpMarker(
                "Whether to lock the specular light color to the diffuse. Usually "
                "desired for PBR.");
            if (lockSpecular)
            {
                opts.directionalSpecular = opts.directionalDiffuse;
            }
            imguiFloatSlider("Intensity", &opts.directionalIntensity, 0.0f, 50.0f, nullptr, Scale::LINEAR);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Emission lights")) 
        {
            imguiFloatSlider("Emission intensity", &opts.emissionIntensity, 0.0f,
                1000.0f, nullptr, Scale::LOG);
            ImGui::DragFloat3("Emission attenuation",
                reinterpret_cast<float*>(&opts.emissionAttenuation),
                /*v_speed=*/0.01f, 0.0f, 10.0f);
            ImGui::SameLine();
            imguiHelpMarker(
                "Constant, linear, and quadratic attenuation of emission lights.");

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Shadows")) 
        {
            ImGui::Checkbox("Shadow mapping", &opts.shadowMapping);
            ImGui::BeginDisabled(!opts.shadowMapping);
            // Shadow map texture is a square, so extend both width/height by the
            // aspect ratio.
            imguiImage(shadowMap.getDepthTexture(),
                glm::vec2(IMAGE_BASE_SIZE * camera.getAspectRatio(),
                    IMAGE_BASE_SIZE * camera.getAspectRatio()));
            imguiFloatSlider("Cuboid extents", &opts.shadowCameraCuboidExtents, 0.1f,
                50.0f, nullptr, Scale::LOG);

            if (imguiFloatSlider("Near plane", &opts.shadowCameraNear, 0.01, 1000.0,
                nullptr, Scale::LOG)) 
            {
                if (opts.shadowCameraNear > opts.shadowCameraFar) 
                {
                    opts.shadowCameraFar = opts.shadowCameraNear;
                }
            }
            if (imguiFloatSlider("Far plane", &opts.shadowCameraFar, 0.01, 1000.0,
                nullptr, Scale::LOG)) {
                if (opts.shadowCameraFar < opts.shadowCameraNear) 
                {
                    opts.shadowCameraNear = opts.shadowCameraFar;
                }
            }
            imguiFloatSlider("Distance from origin", &opts.shadowCameraDistance, 0.01,
                100.0f, nullptr, Scale::LOG);
            if (imguiFloatSlider("Bias min", &opts.shadowBiasMin, 0.0001, 1.0,
                "%.04f", Scale::LOG)) 
            {
                if (opts.shadowBiasMin > opts.shadowBiasMax) 
                {
                    opts.shadowBiasMax = opts.shadowBiasMin;
                }
            }
            if (imguiFloatSlider("Bias max", &opts.shadowBiasMax, 0.0001, 1.0,
                "%.04f", Scale::LOG)) 
            {
                if (opts.shadowBiasMax < opts.shadowBiasMin)
                {
                    opts.shadowBiasMin = opts.shadowBiasMax;
                }
            }

            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Environment"))
        {
            ImGui::Combo("Skybox image", reinterpret_cast<int*>(&opts.skyboxImage),
                "Alex's apt\0Frozen waterfall\0Kloppenheim\0Milkyway\0Mon "
                "Valley\0Ueno shrine\0Winter forest\0");

            ImGui::BeginDisabled(opts.lightingModel == LightingModel::BLINN_PHONG);
            ImGui::Checkbox("Use IBL", &opts.useIBL);
            ImGui::EndDisabled();

            ImGui::BeginDisabled(opts.lightingModel != LightingModel::BLINN_PHONG &&
                opts.useIBL);
            ImGui::ColorEdit3("Ambient color",
                reinterpret_cast<float*>(&opts.ambientColor),
                ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
            ImGui::SameLine();
            imguiHelpMarker("The color of the fixed ambient component.");
            ImGui::EndDisabled();

            ImGui::Checkbox("SSAO", &opts.ssao);
            ImGui::BeginDisabled(!opts.ssao);
            imguiImage(ssaoBuffer.getSsaoTexture(),
                glm::vec2(IMAGE_BASE_SIZE * camera.getAspectRatio(),
                    IMAGE_BASE_SIZE));

            imguiFloatSlider("SSAO radius", &opts.ssaoRadius, 0.01, 5.0, "%.04f",
                Scale::LOG);
            imguiFloatSlider("SSAO bias", &opts.ssaoBias, 0.0001, 1.0, "%.04f",
                Scale::LOG);
            ImGui::EndDisabled();

            ImGui::BeginDisabled(opts.lightingModel != LightingModel::BLINN_PHONG);
            imguiFloatSlider("Shininess", &opts.shininess, 1.0f, 1000.0f, nullptr,
                Scale::LOG);
            ImGui::SameLine();
            imguiHelpMarker(
                "Shininess of specular highlights. Only applies to Phong.");
            ImGui::EndDisabled();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Post-processing")) 
        {
            ImGui::Checkbox("Bloom", &opts.bloom);
            ImGui::BeginDisabled(!opts.bloom);
            imguiFloatSlider("Bloom mix", &opts.bloomMix, 0.001f, 1.0f, nullptr,
                Scale::LOG);
            ImGui::EndDisabled();

            ImGui::Combo(
                "Tone mapping", reinterpret_cast<int*>(&opts.toneMapping),
                "None\0Reinhard\0Reinhard luminance\0ACES (approx)\0AMD\0\0");
            ImGui::Checkbox("Gamma correct", &opts.gammaCorrect);
            ImGui::BeginDisabled(!opts.gammaCorrect);
            imguiFloatSlider("Gamma", &opts.gamma, 0.01f, 8.0f, nullptr, Scale::LOG);
            ImGui::EndDisabled();

            ImGui::Checkbox("FXAA", &opts.fxaa);

            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Camera")) 
    {
        ImGui::RadioButton("Fly controls",
            reinterpret_cast<int*>(&opts.cameraControlType),
            static_cast<int>(CameraControlType::FLY));
        ImGui::SameLine();
        ImGui::RadioButton("Orbit controls",
            reinterpret_cast<int*>(&opts.cameraControlType),
            static_cast<int>(CameraControlType::ORBIT));

        imguiFloatSlider("Speed", &opts.speed, 0.1, 50.0);
        imguiFloatSlider("Sensitivity", &opts.sensitivity, 0.01, 1.0, nullptr,
            Scale::LOG);
        imguiFloatSlider("FoV", &opts.fov, Cme::MIN_FOV, Cme::MAX_FOV, "%.1f��");
        if (imguiFloatSlider("Near plane", &opts.near, 0.01, 1000.0, nullptr,
            Scale::LOG)) 
        {
            if (opts.near > opts.far) 
            {
                opts.far = opts.near;
            }
        }
        if (imguiFloatSlider("Far plane", &opts.far, 0.01, 1000.0, nullptr, Scale::LOG)) 
        {
            if (opts.far < opts.near)
            {
                opts.near = opts.far;
            }
        }

        ImGui::Checkbox("Capture mouse", &opts.captureMouse);
    }

    if (ImGui::CollapsingHeader("Debug")) 
    {
        ImGui::Combo(
            "G-Buffer vis", reinterpret_cast<int*>(&opts.gBufferVis),
            "Disabled\0Positions\0Ambient "
            "occlusion\0Normals\0Roughness\0Albedo\0Metallic\0Emission\0\0");
        ImGui::SameLine();
        imguiHelpMarker("What component of the G-Buffer to visualize.");

        ImGui::Checkbox("Wireframe", &opts.wireframe);
        ImGui::Checkbox("Draw vertex normals", &opts.drawNormals);
    }

    if (ImGui::CollapsingHeader("Performance")) 
    {
        char overlay[32];
        sprintf_s(overlay, "Avg FPS %.02f", opts.avgFPS);
        ImGui::PlotLines("Frame time", opts.frameDeltas, opts.numFrameDeltas,
            opts.frameDeltasOffset, overlay, 0.0f, 0.03f,
            ImVec2(0, 80.0f));

        ImGui::Checkbox("Enable VSync", &opts.enableVsync);
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::Render();
}

/** Loads a model based on command line flag, or a default. */
std::unique_ptr<Cme::Model> loadModelOrDefault()
{
    // Default to the gltf DamagedHelmet.
    auto helmet = std::make_unique<Cme::Model>("assets//models//DamagedHelmet/DamagedHelmet.gltf");
    return helmet;
}

/** Loads a skybox image as a cubemap and generates IBL info. */
void loadSkyboxImage(
    SkyboxImage skyboxImage, Cme::SkyboxMesh& skybox,
    Cme::EquirectCubemapConverter& equirectCubemapConverter,
    Cme::CubemapIrradianceCalculator& irradianceCalculator,
    Cme::GGXPrefilteredEnvMapCalculator& prefilteredEnvMapCalculator) 
{
    std::string hdrPath;
    switch (skyboxImage)
    {
    case SkyboxImage::ALEXS_APT:
        hdrPath = "assets//models//ibl/AlexsApt.hdr";
        break;
    case SkyboxImage::FROZEN_WATERFALL:
        hdrPath = "assets//models//ibl//FrozenWaterfall.hdr";
        break;
    case SkyboxImage::KLOPPENHEIM:
        hdrPath = "assets//models//ibl//Kloppenheim.hdr";
        break;
    case SkyboxImage::MILKYWAY:
        hdrPath = "assets//models//ibl//Milkyway.hdr";
        break;
    case SkyboxImage::MON_VALLEY:
        hdrPath = "assets//models//ibl//MonValley.hdr";
        break;
    case SkyboxImage::UENO_SHRINE:
        hdrPath = "assets//models//ibl//UenoShrine.hdr";
        break;
    case SkyboxImage::WINTER_FOREST:
        hdrPath = "assets//models//ibl//WinterForest.hdr";
        break;
    }

    Cme::Texture hdr = Cme::Texture::loadHdr(hdrPath.c_str());

    // Process HDR cubemap
    {
        Cme::DebugGroup debugGroup("HDR equirect to cubemap");
        equirectCubemapConverter.multipassDraw(hdr);
    }
    auto cubemap = equirectCubemapConverter.getCubemap();
    {
        Cme::DebugGroup debugGroup("Irradiance calculation");
        irradianceCalculator.multipassDraw(cubemap);
    }
    {
        Cme::DebugGroup debugGroup("Prefiltered env map calculation");
        prefilteredEnvMapCalculator.multipassDraw(cubemap);
    }

    skybox.setTexture(cubemap);

    // Don't need this anymore.
    hdr.free();
}

int main(int argc, char** argv)
{

    Cme::Window win(1920, 1080, "Model Render", /* fullscreen */ false, 0);
    win.setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
    win.setEscBehavior(Cme::EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE);

    // Setup Dear ImGui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(win.getGlfwRef(), /*install_callbacks=*/true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    // == Main setup ==

    // Prepare opts for usage.
    ModelRenderOptions opts;

    // Setup the camera.
    auto camera = std::make_shared<Cme::Camera>(/* position */ glm::vec3(0.0f, 0.0f, 3.0f));
    std::shared_ptr<Cme::CameraControls> cameraControls = std::make_shared<Cme::OrbitCameraControls>(*camera);
    win.bindCamera(camera);
    win.bindCameraControls(cameraControls);

    // Create light registry and add lights.
    auto lightRegistry = std::make_shared<Cme::LightRegistry>();
    lightRegistry->setViewSource(camera);

    auto directionalLight = std::make_shared<Cme::DirectionalLight>();
    lightRegistry->addLight(directionalLight);

    auto pointLight = std::make_shared<Cme::PointLight>(glm::vec3(1.2f, 1.0f, 2.0f));
    pointLight->setSpecular(glm::vec3(0.5f, 0.5f, 0.5f));
    // lightRegistry->addLight(pointLight);

    // Create a mesh for the light.
    Cme::SphereMesh lightSphere;
    lightSphere.setModelTransform(
        glm::scale(glm::translate(glm::mat4(1.0f), pointLight->getPosition()),
            glm::vec3(0.2f)));

    // Set up the main framebuffer that will store intermediate states.
    Cme::Framebuffer mainFb(win.getSize());
    auto mainColorAttachment =  mainFb.attachTexture(Cme::BufferType::COLOR_HDR_ALPHA);
    mainFb.attachRenderbuffer(Cme::BufferType::DEPTH_AND_STENCIL);

    Cme::Framebuffer finalFb(win.getSize());
    auto finalColorAttachment = finalFb.attachTexture(Cme::BufferType::COLOR_ALPHA);

    // Build the G-Buffer and prepare deferred shading.
    Cme::DeferredGeometryPassShader geometryPassShader;
    geometryPassShader.addUniformSource(camera);

    auto gBuffer = std::make_shared<Cme::GBuffer>(win.getSize());
    auto lightingTextureRegistry = std::make_shared<Cme::TextureRegistry>();
    lightingTextureRegistry->addTextureSource(gBuffer);

    Cme::ScreenQuadMesh screenQuad;
    Cme::ScreenShader gBufferVisShader(
        Cme::ShaderPath("assets//model_shaders//gbuffer_vis.frag"));

    Cme::ScreenShader lightingPassShader(
        Cme::ShaderPath("assets//model_shaders//lighting_pass.frag"));
    lightingPassShader.addUniformSource(camera);
    lightingPassShader.addUniformSource(lightingTextureRegistry);
    lightingPassShader.addUniformSource(lightRegistry);

    // Setup shadow mapping.
    constexpr int SHADOW_MAP_SIZE = 1024;
    auto shadowMap = std::make_shared<Cme::ShadowMap>(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    lightingTextureRegistry->addTextureSource(shadowMap);

    Cme::ShadowMapShader shadowShader;
    auto shadowCamera = std::make_shared<Cme::ShadowCamera>(directionalLight);
    shadowShader.addUniformSource(shadowCamera);
    lightingPassShader.addUniformSource(shadowCamera);

    // Setup SSAO.
    Cme::SsaoShader ssaoShader;
    ssaoShader.addUniformSource(camera);

    auto ssaoKernel = std::make_shared<Cme::SsaoKernel>();
    ssaoShader.addUniformSource(ssaoKernel);
    auto ssaoBuffer = std::make_shared<Cme::SsaoBuffer>(win.getSize());

    auto ssaoTextureRegistry = std::make_shared<Cme::TextureRegistry>();
    ssaoTextureRegistry->addTextureSource(gBuffer);
    ssaoTextureRegistry->addTextureSource(ssaoKernel);
    ssaoShader.addUniformSource(ssaoTextureRegistry);

    Cme::SsaoBlurShader ssaoBlurShader;
    auto ssaoBlurredBuffer = std::make_shared<Cme::SsaoBuffer>(win.getSize());
    lightingTextureRegistry->addTextureSource(ssaoBlurredBuffer);

    // Setup post processing.
    auto bloomPass = std::make_shared<Cme::BloomPass>(win.getSize());

    auto postprocessTextureRegistry = std::make_shared<Cme::TextureRegistry>();
    postprocessTextureRegistry->addTextureSource(bloomPass);
    Cme::ScreenShader postprocessShader(
        Cme::ShaderPath("assets//model_shaders//post_processing.frag"));
    postprocessShader.addUniformSource(postprocessTextureRegistry);

    Cme::FXAAShader fxaaShader;

    // Setup skybox and IBL.
    Cme::SkyboxShader skyboxShader;
    skyboxShader.addUniformSource(camera);

    constexpr int CUBEMAP_SIZE = 1024;
    Cme::EquirectCubemapConverter equirectCubemapConverter(
        CUBEMAP_SIZE, CUBEMAP_SIZE, /*generateMips=*/true);

    // Irradiance map averages radiance uniformly so it doesn't have a lot of high
    // frequency details and can thus be small.
    auto irradianceCalculator =
        std::make_shared<Cme::CubemapIrradianceCalculator>(32, 32);
    auto irradianceMap = irradianceCalculator->getIrradianceMap();
    lightingTextureRegistry->addTextureSource(irradianceCalculator);

    // Create prefiltered envmap for specular IBL. It doesn't have to be super
    // large.
    auto prefilteredEnvMapCalculator =
        std::make_shared<Cme::GGXPrefilteredEnvMapCalculator>(CUBEMAP_SIZE,
            CUBEMAP_SIZE);
    auto prefilteredEnvMap = prefilteredEnvMapCalculator->getPrefilteredEnvMap();
    lightingTextureRegistry->addTextureSource(prefilteredEnvMapCalculator);
    lightingPassShader.addUniformSource(prefilteredEnvMapCalculator);

    auto brdfLUT = std::make_shared<Cme::GGXBrdfIntegrationCalculator>(
        CUBEMAP_SIZE, CUBEMAP_SIZE);
    {
        // Only needs to be calculated once up front.
        Cme::DebugGroup debugGroup("BRDF LUT calculation");
        brdfLUT->draw();
    }
    auto brdfIntegrationMap = brdfLUT->getBrdfIntegrationMap();
    lightingTextureRegistry->addTextureSource(brdfLUT);

    Cme::SkyboxMesh skybox;

    // Load the actual env map and generate IBL textures.
    loadSkyboxImage(opts.skyboxImage, skybox, equirectCubemapConverter,
        *irradianceCalculator, *prefilteredEnvMapCalculator);

    // Prepare some debug shaders.
    Cme::Shader normalShader(
        Cme::ShaderPath("assets//model_shaders//model.vert"),
        Cme::ShaderInline(normalShaderSource),
        Cme::ShaderPath("assets//model_shaders//model_normals.geom"));
    normalShader.addUniformSource(camera);

    Cme::Shader lampShader(Cme::ShaderPath("assets//model_shaders//model.vert"), Cme::ShaderInline(lampShaderSource));
    lampShader.addUniformSource(camera);

    // Load primary model.
    std::unique_ptr<Cme::Model> model = loadModelOrDefault();

    win.enableFaceCull();
    win.loop([&](float deltaTime) 
    {
        // ImGui logic.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        win.setMouseInputPaused(io.WantCaptureMouse);
        win.setKeyInputPaused(io.WantCaptureKeyboard);

        ModelRenderOptions prevOpts = opts;

        // Initialize with certain current options.
        opts.speed = cameraControls->getSpeed();
        opts.sensitivity = cameraControls->getSensitivity();
        opts.fov = camera->getFov();
        opts.near = camera->getNearPlane();
        opts.far = camera->getFarPlane();

        opts.frameDeltas = win.getFrameDeltas();
        opts.numFrameDeltas = win.getNumFrameDeltas();
        opts.frameDeltasOffset = win.getFrameDeltasOffset();
        opts.avgFPS = win.getAvgFPS();

        //renderImGuiUI(opts, ctx);
        renderImGuiUI(opts, *camera, *shadowMap, *ssaoBlurredBuffer);

        // Post-process options. Some option values are used later during rendering.
        model->setModelTransform(glm::scale(glm::mat4_cast(opts.modelRotation),
            glm::vec3(opts.modelScale)));

        directionalLight->setDiffuse(opts.directionalDiffuse *
            opts.directionalIntensity);
        directionalLight->setSpecular(opts.directionalSpecular *
            opts.directionalIntensity);
        directionalLight->setDirection(opts.directionalDirection);

        cameraControls->setSpeed(opts.speed);
        cameraControls->setSensitivity(opts.sensitivity);
        camera->setFov(opts.fov);
        camera->setNearPlane(opts.near);
        camera->setFarPlane(opts.far);

        if (opts.cameraControlType != prevOpts.cameraControlType) 
        {
            std::shared_ptr<Cme::CameraControls> newControls;
            switch (opts.cameraControlType)
            {
            case CameraControlType::FLY:
                newControls = std::make_shared<Cme::FlyCameraControls>();
                break;
            case CameraControlType::ORBIT:
                newControls = std::make_shared<Cme::OrbitCameraControls>(*camera);
                break;
            }
            newControls->setSpeed(cameraControls->getSpeed());
            newControls->setSensitivity(cameraControls->getSensitivity());
            cameraControls = newControls;
            win.bindCameraControls(cameraControls);
        }
        if (opts.enableVsync != prevOpts.enableVsync) 
        {
            if (opts.enableVsync)
            {
                win.enableVsync();
            }
            else
            {
                win.disableVsync();
            }
        }
        if (opts.skyboxImage != prevOpts.skyboxImage) 
        {
            loadSkyboxImage(opts.skyboxImage, skybox, equirectCubemapConverter,
                *irradianceCalculator, *prefilteredEnvMapCalculator);
        }

        win.setMouseButtonBehavior(opts.captureMouse
            ? Cme::MouseButtonBehavior::CAPTURE_MOUSE
            : Cme::MouseButtonBehavior::NONE);

        // == Main render path ==
        // Step 0: optional shadow pass.
        if (opts.shadowMapping) 
        {
            Cme::DebugGroup debugGroup("Directional shadow map");
            shadowCamera->setCuboidExtents(opts.shadowCameraCuboidExtents);
            shadowCamera->setNearPlane(opts.shadowCameraNear);
            shadowCamera->setFarPlane(opts.shadowCameraFar);
            shadowCamera->setDistanceFromOrigin(opts.shadowCameraDistance);

            shadowMap->activate();
            shadowMap->clear();
            shadowShader.updateUniforms();
            model->draw(shadowShader);
            shadowMap->deactivate();
        }

        // Step 1: geometry pass. Build the G-Buffer.
        {
            Cme::DebugGroup debugGroup("Geometry pass");
            gBuffer->activate();
            gBuffer->clear();

            geometryPassShader.updateUniforms();

            // Draw model.
            if (opts.wireframe)
            {
                win.enableWireframe();
            }
            model->draw(geometryPassShader);
            if (opts.wireframe) 
            {
                win.disableWireframe();
            }

            gBuffer->deactivate();
        }

        if (opts.gBufferVis != GBufferVis::DISABLED)
        {
            {
                Cme::DebugGroup debugGroup("G-Buffer vis");
                switch (opts.gBufferVis) {
                case GBufferVis::POSITIONS:
                case GBufferVis::AO:
                    screenQuad.setTexture(gBuffer->getPositionAOTexture());
                    break;
                case GBufferVis::NORMALS:
                case GBufferVis::ROUGHNESS:
                    screenQuad.setTexture(gBuffer->getNormalRoughnessTexture());
                    break;
                case GBufferVis::ALBEDO:
                case GBufferVis::METALLIC:
                    screenQuad.setTexture(gBuffer->getAlbedoMetallicTexture());
                    break;
                case GBufferVis::EMISSION:
                    screenQuad.setTexture(gBuffer->getEmissionTexture());
                    break;
                case GBufferVis::DISABLED:
                    break;
                };
                gBufferVisShader.setInt("gBufferVis",
                    static_cast<int>(opts.gBufferVis));
                screenQuad.draw(gBufferVisShader);
            }

            // TODO: Refactor avoid needing to copy this.
            {
                Cme::DebugGroup debugGroup("Imgui pass");
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            return;
        }

        // Step 1.2: optional SSAO pass.
        // TODO: Extract into an "SSAO pass".
        if (opts.ssao)
        {
            Cme::DebugGroup debugGroup("SSAO pass");
            ssaoKernel->setRadius(opts.ssaoRadius);
            ssaoKernel->setBias(opts.ssaoBias);

            ssaoBuffer->activate();
            ssaoBuffer->clear();

            ssaoShader.updateUniforms();

            screenQuad.unsetTexture();
            screenQuad.draw(ssaoShader, ssaoTextureRegistry.get());

            ssaoBuffer->deactivate();

            // Step 1.2.1: SSAO blur.
            ssaoBlurredBuffer->activate();
            ssaoBlurredBuffer->clear();

            ssaoBlurShader.configureWith(*ssaoKernel, *ssaoBuffer);
            screenQuad.draw(ssaoBlurShader);

            ssaoBlurredBuffer->deactivate();
        }

        // Step 2: lighting pass. Draw to the main framebuffer.
        {
            Cme::DebugGroup debugGroup("Deferred lighting pass");
            mainFb.activate();
            mainFb.clear();

            // TODO: Set up environment mapping with the skybox.
            lightingPassShader.updateUniforms();
            lightingPassShader.setBool("shadowMapping", opts.shadowMapping);
            lightingPassShader.setFloat("shadowBiasMin", opts.shadowBiasMin);
            lightingPassShader.setFloat("shadowBiasMax", opts.shadowBiasMax);
            lightingPassShader.setBool("useIBL", opts.useIBL);
            lightingPassShader.setBool("ssao", opts.ssao);
            lightingPassShader.setInt("lightingModel",
                static_cast<int>(opts.lightingModel));
            // TODO: Pull this out into a material class.
            lightingPassShader.setVec3("ambient", opts.ambientColor);
            lightingPassShader.setFloat("shininess", opts.shininess);
            lightingPassShader.setFloat("emissionIntensity", opts.emissionIntensity);
            lightingPassShader.setFloat("emissionAttenuation.constant",
                opts.emissionAttenuation.x);
            lightingPassShader.setFloat("emissionAttenuation.linear",
                opts.emissionAttenuation.y);
            lightingPassShader.setFloat("emissionAttenuation.quadratic",
                opts.emissionAttenuation.z);

            screenQuad.unsetTexture();
            screenQuad.draw(lightingPassShader, lightingTextureRegistry.get());

            mainFb.deactivate();
        }

        // Step 3: forward render anything else on top.
        {
            Cme::DebugGroup debugGroup("Forward pass");

            // Before we do so, we have to blit the depth buffer.
            gBuffer->blit(mainFb, GL_DEPTH_BUFFER_BIT);

            mainFb.activate();

            if (opts.drawNormals) 
            {
                // Draw the normals.
                normalShader.updateUniforms();
                model->draw(normalShader);
            }

            // Draw light source.
            lampShader.updateUniforms();
            if (opts.wireframe)
            {
                win.enableWireframe();
            }
            // TODO: Make point lights more part of the UI.
            // lightSphere.draw(lampShader);
            if (opts.wireframe)
            {
                win.disableWireframe();
            }

            // Draw skybox.
            skyboxShader.updateUniforms();
            skybox.draw(skyboxShader);

            mainFb.deactivate();
        }

        // Step 4: post processing.
        if (opts.bloom) 
        {
            Cme::DebugGroup debugGroup("Bloom pass");
            bloomPass->multipassDraw(/*sourceFb=*/mainFb);
        }

        {
            Cme::DebugGroup debugGroup("Tonemap & gamma");
            finalFb.activate();
            finalFb.clear();

            // Draw to the final FB using the post process shader.
            postprocessShader.updateUniforms();
            postprocessShader.setBool("bloom", opts.bloom);
            postprocessShader.setFloat("bloomMix", opts.bloomMix);
            postprocessShader.setInt("toneMapping",
                static_cast<int>(opts.toneMapping));
            postprocessShader.setBool("gammaCorrect", opts.gammaCorrect);
            postprocessShader.setFloat("gamma", static_cast<int>(opts.gamma));
            screenQuad.setTexture(mainColorAttachment);
            screenQuad.draw(postprocessShader, postprocessTextureRegistry.get());

            finalFb.deactivate();
        }

        win.setViewport();

        // Finally draw to the screen via the FXAA shader.
        if (opts.fxaa) 
        {
            Cme::DebugGroup debugGroup("FXAA");
            screenQuad.setTexture(finalColorAttachment);
            screenQuad.draw(fxaaShader);
        }
        else 
        {
            finalFb.blitToDefault(GL_COLOR_BUFFER_BIT);
        }

        // == End render path ==

        // Finally, draw ImGui data.
        {
            Cme::DebugGroup debugGroup("Imgui pass");
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    });

    // Cleanup.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
