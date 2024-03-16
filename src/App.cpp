#include "App.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


#include "../imguizmo_quat/imGuIZMOquat.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

namespace Cme
{
	//App* App::Instance()
	//{
	//	static App* instance = nullptr;
	//	if (instance == nullptr)
	//	{
	//		instance = new App;
	//	}
	//	return instance;
	//}

    App::App()
    {
    }

    App::~App()
    {
        //glfwDestroyWindow(m_pWindow);
    }

	void App::Init(bool bFullScreen)
	{
        m_pWindow = new Cme::Window(1920, 1080, "Model Render", false, 0);
        m_pWindow->setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
        m_pWindow->setEscBehavior(Cme::EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE);

        // Setup Dear ImGui.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(m_pWindow->getGlfwRef(), true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        // == Main setup ==

        // Prepare opts for usage.
        //ModelRenderOptions opts;

        // Setup the camera.
        m_spCamera = std::make_shared<Cme::Camera>(/* position */ glm::vec3(0.0f, 0.0f, 3.0f));
        m_spCameraControls = std::make_shared<Cme::OrbitCameraControls>(*m_spCamera);

        m_pWindow->bindCamera(m_spCamera);
        m_pWindow->bindCameraControls(m_spCameraControls);

        // Create light registry and add lights.
        auto lightRegistry = std::make_shared<Cme::LightRegistry>();
        lightRegistry->setViewSource(m_spCamera);

        m_spDirectionalLight = std::make_shared<Cme::DirectionalLight>();
        lightRegistry->addLight(m_spDirectionalLight);

        auto pointLight = std::make_shared<Cme::PointLight>(glm::vec3(1.2f, 1.0f, 2.0f));
        pointLight->setSpecular(glm::vec3(0.5f, 0.5f, 0.5f));
        // lightRegistry->addLight(pointLight);

        // Create a mesh for the light.
        Cme::SphereMesh lightSphere;
        lightSphere.setModelTransform(glm::scale(glm::translate(glm::mat4(1.0f), pointLight->getPosition()), glm::vec3(0.2f)));

        // Set up the main framebuffer that will store intermediate states.
        //Cme::Framebuffer mainFb(m_pWindow->getSize());
        m_spMainFb = std::make_shared<Cme::Framebuffer>(m_pWindow->getSize());
        m_MainColorAttachmentObj = m_spMainFb->attachTexture(Cme::BufferType::COLOR_HDR_ALPHA);
        m_spMainFb->attachRenderbuffer(Cme::BufferType::DEPTH_AND_STENCIL);

        //Cme::Framebuffer finalFb(m_pWindow->getSize());
        m_spFinalFb = std::make_shared<Cme::Framebuffer>(m_pWindow->getSize());
        m_FinalColorAttachmentObj = m_spFinalFb->attachTexture(Cme::BufferType::COLOR_ALPHA);

        // Build the G-Buffer and prepare deferred shading.
        m_spGeometryPassShader = std::make_shared<Cme::DeferredGeometryPassShader>();
        m_spGeometryPassShader->addUniformSource(m_spCamera);

        // GBuffer
        m_spGBuffer = std::make_shared<Cme::GBuffer>(m_pWindow->getSize());
        m_spLightingTextureRegistry = std::make_shared<Cme::TextureRegistry>();
        m_spLightingTextureRegistry->addTextureSource(m_spGBuffer);

        // Screen
        //Cme::ScreenQuadMesh screenQuad;
        //Cme::ScreenShader gBufferVisShader(Cme::ShaderPath("assets//model_shaders//gbuffer_vis.frag"));
        m_spScreenQuad = std::make_shared<Cme::ScreenQuadMesh>();
        m_spGBufferVisShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//gbuffer_vis.frag"));
        m_spLightingPassShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//lighting_pass.frag"));

        m_spLightingPassShader->addUniformSource(m_spCamera);
        m_spLightingPassShader->addUniformSource(m_spLightingTextureRegistry);
        m_spLightingPassShader->addUniformSource(lightRegistry);

        // Setup shadow mapping.
        constexpr int SHADOW_MAP_SIZE = 1024;
        m_spShadowMap = std::make_shared<Cme::ShadowMap>(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        m_spLightingTextureRegistry->addTextureSource(m_spShadowMap);

        m_spShadowCamera = std::make_shared<Cme::ShadowCamera>(m_spDirectionalLight);

        m_spShadowShader = std::make_shared<Cme::ShadowMapShader>();
        m_spShadowShader->addUniformSource(m_spShadowCamera);
        m_spLightingPassShader->addUniformSource(m_spShadowCamera);

        // Setup SSAO.
        //Cme::SsaoShader ssaoShader;
        m_spSsaoShader = std::make_shared<Cme::SsaoShader>();
        m_spSsaoShader->addUniformSource(m_spCamera);

        m_spSsaoKernel = std::make_shared<Cme::SsaoKernel>();
        m_spSsaoShader->addUniformSource(m_spSsaoKernel);
        m_spSsaoBuffer = std::make_shared<Cme::SsaoBuffer>(m_pWindow->getSize());

        m_spSsaoTextureRegistry = std::make_shared<Cme::TextureRegistry>();
        m_spSsaoTextureRegistry->addTextureSource(m_spGBuffer);
        m_spSsaoTextureRegistry->addTextureSource(m_spSsaoKernel);
        m_spSsaoShader->addUniformSource(m_spSsaoTextureRegistry);

        //Cme::SsaoBlurShader ssaoBlurShader;
        m_spSsaoBlurShader = std::make_shared<Cme::SsaoBlurShader>();
        m_spSsaoBlurredBuffer = std::make_shared<Cme::SsaoBuffer>(m_pWindow->getSize());
        m_spLightingTextureRegistry->addTextureSource(m_spSsaoBlurredBuffer);


        // Setup post processing.
        m_spBloomPass = std::make_shared<Cme::BloomPass>(m_pWindow->getSize());

        m_spPostprocessTextureRegistry = std::make_shared<Cme::TextureRegistry>();
        m_spPostprocessTextureRegistry->addTextureSource(m_spBloomPass);

        //Cme::ScreenShader postprocessShader(Cme::ShaderPath("assets//model_shaders//post_processing.frag"));
        m_spPostprocessShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//post_processing.frag"));
        m_spPostprocessShader->addUniformSource(m_spPostprocessTextureRegistry);

        //Cme::FXAAShader fxaaShader;
        m_spFxaaShader = std::make_shared<Cme::FXAAShader>();

        // Setup skybox and IBL.
        //Cme::SkyboxShader skyboxShader;
        m_spSkyboxShader = std::make_shared<Cme::SkyboxShader>();
        m_spSkyboxShader->addUniformSource(m_spCamera);

        constexpr int CUBEMAP_SIZE = 1024;
        m_spEquirectCubemapConverter = std::make_shared<Cme::EquirectCubemapConverter>(CUBEMAP_SIZE, CUBEMAP_SIZE, true);

        // Irradiance map averages radiance uniformly so it doesn't have a lot of high
        // frequency details and can thus be small.
        m_spIrradianceCalculator = std::make_shared<Cme::CubemapIrradianceCalculator>(32, 32);
        auto irradianceMap = m_spIrradianceCalculator->getIrradianceMap();
        m_spLightingTextureRegistry->addTextureSource(m_spIrradianceCalculator);

        // Create prefiltered envmap for specular IBL. It doesn't have to be super
        // large.
        m_spPrefilteredEnvMapCalculator = std::make_shared<Cme::GGXPrefilteredEnvMapCalculator>(CUBEMAP_SIZE, CUBEMAP_SIZE);
        auto prefilteredEnvMap = m_spPrefilteredEnvMapCalculator->getPrefilteredEnvMap();
        m_spLightingTextureRegistry->addTextureSource(m_spPrefilteredEnvMapCalculator);
        m_spLightingPassShader->addUniformSource(m_spPrefilteredEnvMapCalculator);

        auto brdfLUT = std::make_shared<Cme::GGXBrdfIntegrationCalculator>(
            CUBEMAP_SIZE, CUBEMAP_SIZE);
        {
            // Only needs to be calculated once up front.
            Cme::DebugGroup debugGroup("BRDF LUT calculation");
            brdfLUT->draw();
        }
        auto brdfIntegrationMap = brdfLUT->getBrdfIntegrationMap();
        m_spLightingTextureRegistry->addTextureSource(brdfLUT);

        //Cme::SkyboxMesh skybox;
        m_spSkybox = std::make_shared<Cme::SkyboxMesh>();
        // Load the actual env map and generate IBL textures.
        LoadSkyboxImage(m_OptsObj.skyboxImage, *m_spSkybox, *m_spEquirectCubemapConverter,
            *m_spIrradianceCalculator, *m_spPrefilteredEnvMapCalculator);

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

        // Prepare some debug shaders.
        m_spNormalShader = std::make_shared<Cme::Shader>(Cme::ShaderPath("assets//model_shaders//model.vert"),
                                                        Cme::ShaderInline(normalShaderSource),
                                                        Cme::ShaderPath("assets//model_shaders//model_normals.geom"));
        m_spNormalShader->addUniformSource(m_spCamera);

        m_spLampShader = std::make_shared<Cme::Shader>(Cme::ShaderPath("assets//model_shaders//model.vert"), Cme::ShaderInline(lampShaderSource));
        m_spLampShader->addUniformSource(m_spCamera);

        // Load primary model.
        m_upModel = LoadModelOrDefault();

        m_pWindow->enableFaceCull();
	}

	bool App::Run()
	{
        ImGuiIO& io = ImGui::GetIO();
        m_pWindow->loop([&](float deltaTime)
        {
            // ImGui logic.
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            m_pWindow->setMouseInputPaused(io.WantCaptureMouse);
            m_pWindow->setKeyInputPaused(io.WantCaptureKeyboard);

            ModelRenderOptions prevOpts = m_OptsObj;

            // Initialize with certain current options.
            m_OptsObj.speed = m_spCameraControls->getSpeed();
            m_OptsObj.sensitivity = m_spCameraControls->getSensitivity();
            m_OptsObj.fov = m_spCamera->getFov();
            m_OptsObj.near = m_spCamera->getNearPlane();
            m_OptsObj.far = m_spCamera->getFarPlane();

            m_OptsObj.frameDeltas = m_pWindow->getFrameDeltas();
            m_OptsObj.numFrameDeltas = m_pWindow->getNumFrameDeltas();
            m_OptsObj.frameDeltasOffset = m_pWindow->getFrameDeltasOffset();
            m_OptsObj.avgFPS = m_pWindow->getAvgFPS();

            //renderImGuiUI(opts, ctx);
            RenderImGuiUI(m_OptsObj, *m_spCamera, *m_spShadowMap, *m_spSsaoBlurredBuffer);

            // Post-process options. Some option values are used later during rendering.
            m_upModel->setModelTransform(glm::scale(glm::mat4_cast(m_OptsObj.modelRotation),
                glm::vec3(m_OptsObj.modelScale)));

            m_spDirectionalLight->setDiffuse(m_OptsObj.directionalDiffuse *
                m_OptsObj.directionalIntensity);
            m_spDirectionalLight->setSpecular(m_OptsObj.directionalSpecular *
                m_OptsObj.directionalIntensity);
            m_spDirectionalLight->setDirection(m_OptsObj.directionalDirection);

            m_spCameraControls->setSpeed(m_OptsObj.speed);
            m_spCameraControls->setSensitivity(m_OptsObj.sensitivity);
            m_spCamera->setFov(m_OptsObj.fov);
            m_spCamera->setNearPlane(m_OptsObj.near);
            m_spCamera->setFarPlane(m_OptsObj.far);

            if (m_OptsObj.cameraControlType != prevOpts.cameraControlType)
            {
                std::shared_ptr<Cme::CameraControls> newControls;
                switch (m_OptsObj.cameraControlType)
                {
                case CameraControlType::FLY:
                    newControls = std::make_shared<Cme::FlyCameraControls>();
                    break;
                case CameraControlType::ORBIT:
                    newControls = std::make_shared<Cme::OrbitCameraControls>(*m_spCamera);
                    break;
                }
                newControls->setSpeed(m_spCameraControls->getSpeed());
                newControls->setSensitivity(m_spCameraControls->getSensitivity());
                m_spCameraControls = newControls;
                m_pWindow->bindCameraControls(m_spCameraControls);
            }
            if (m_OptsObj.enableVsync != prevOpts.enableVsync)
            {
                if (m_OptsObj.enableVsync)
                {
                    m_pWindow->enableVsync();
                }
                else
                {
                    m_pWindow->disableVsync();
                }
            }
            if (m_OptsObj.skyboxImage != prevOpts.skyboxImage)
            {
                LoadSkyboxImage(m_OptsObj.skyboxImage, *m_spSkybox, *m_spEquirectCubemapConverter, *m_spIrradianceCalculator, *m_spPrefilteredEnvMapCalculator);
            }

            m_pWindow->setMouseButtonBehavior(m_OptsObj.captureMouse
                ? Cme::MouseButtonBehavior::CAPTURE_MOUSE
                : Cme::MouseButtonBehavior::NONE);

            // == Main render path ==
            // Step 0: optional shadow pass.
            if (m_OptsObj.shadowMapping)
            {
                Cme::DebugGroup debugGroup("Directional shadow map");
                m_spShadowCamera->setCuboidExtents(m_OptsObj.shadowCameraCuboidExtents);
                m_spShadowCamera->setNearPlane(m_OptsObj.shadowCameraNear);
                m_spShadowCamera->setFarPlane(m_OptsObj.shadowCameraFar);
                m_spShadowCamera->setDistanceFromOrigin(m_OptsObj.shadowCameraDistance);

                m_spShadowMap->activate();
                m_spShadowMap->clear();
                m_spShadowShader->updateUniforms();
                m_upModel->draw(*m_spShadowShader);
                m_spShadowMap->deactivate();
            }

            // Step 1: geometry pass. Build the G-Buffer.
            {
                Cme::DebugGroup debugGroup("Geometry pass");
                m_spGBuffer->activate();
                m_spGBuffer->clear();

                m_spGeometryPassShader->updateUniforms();

                // Draw model.
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->enableWireframe();
                }
                m_upModel->draw(*m_spGeometryPassShader);
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->disableWireframe();
                }

                m_spGBuffer->deactivate();
            }

            if (m_OptsObj.gBufferVis != GBufferVis::DISABLED)
            {
                {
                    Cme::DebugGroup debugGroup("G-Buffer vis");
                    switch (m_OptsObj.gBufferVis)
                    {
                    case GBufferVis::POSITIONS:
                    case GBufferVis::AO:
                        m_spScreenQuad->setTexture(m_spGBuffer->getPositionAOTexture());
                        break;
                    case GBufferVis::NORMALS:
                    case GBufferVis::ROUGHNESS:
                        m_spScreenQuad->setTexture(m_spGBuffer->getNormalRoughnessTexture());
                        break;
                    case GBufferVis::ALBEDO:
                    case GBufferVis::METALLIC:
                        m_spScreenQuad->setTexture(m_spGBuffer->getAlbedoMetallicTexture());
                        break;
                    case GBufferVis::EMISSION:
                        m_spScreenQuad->setTexture(m_spGBuffer->getEmissionTexture());
                        break;
                    case GBufferVis::DISABLED:
                        break;
                    };
                    m_spGBufferVisShader->setInt("gBufferVis", static_cast<int>(m_OptsObj.gBufferVis));
                    m_spScreenQuad->draw(*m_spGBufferVisShader);
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
            if (m_OptsObj.ssao)
            {
                Cme::DebugGroup debugGroup("SSAO pass");
                m_spSsaoKernel->setRadius(m_OptsObj.ssaoRadius);
                m_spSsaoKernel->setBias(m_OptsObj.ssaoBias);

                m_spSsaoBuffer->activate();
                m_spSsaoBuffer->clear();

                m_spSsaoShader->updateUniforms();

                m_spScreenQuad->unsetTexture();
                m_spScreenQuad->draw(*m_spSsaoShader, m_spSsaoTextureRegistry.get());

                m_spSsaoBuffer->deactivate();

                // Step 1.2.1: SSAO blur.
                m_spSsaoBlurredBuffer->activate();
                m_spSsaoBlurredBuffer->clear();

                m_spSsaoBlurShader->configureWith(*m_spSsaoKernel, *m_spSsaoBuffer);
                m_spScreenQuad->draw(*m_spSsaoBlurShader);

                m_spSsaoBlurredBuffer->deactivate();
            }

            // Step 2: lighting pass. Draw to the main framebuffer.
            {
                Cme::DebugGroup debugGroup("Deferred lighting pass");
                m_spMainFb->activate();
                m_spMainFb->clear();

                // TODO: Set up environment mapping with the skybox.
                m_spLightingPassShader->updateUniforms();
                m_spLightingPassShader->setBool("shadowMapping", m_OptsObj.shadowMapping);
                m_spLightingPassShader->setFloat("shadowBiasMin", m_OptsObj.shadowBiasMin);
                m_spLightingPassShader->setFloat("shadowBiasMax", m_OptsObj.shadowBiasMax);
                m_spLightingPassShader->setBool("useIBL", m_OptsObj.useIBL);
                m_spLightingPassShader->setBool("ssao", m_OptsObj.ssao);
                m_spLightingPassShader->setInt("lightingModel", static_cast<int>(m_OptsObj.lightingModel));
                // TODO: Pull this out into a material class.
                m_spLightingPassShader->setVec3("ambient", m_OptsObj.ambientColor);
                m_spLightingPassShader->setFloat("shininess", m_OptsObj.shininess);
                m_spLightingPassShader->setFloat("emissionIntensity", m_OptsObj.emissionIntensity);
                m_spLightingPassShader->setFloat("emissionAttenuation.constant",
                    m_OptsObj.emissionAttenuation.x);
                m_spLightingPassShader->setFloat("emissionAttenuation.linear",
                    m_OptsObj.emissionAttenuation.y);
                m_spLightingPassShader->setFloat("emissionAttenuation.quadratic",
                    m_OptsObj.emissionAttenuation.z);

                m_spScreenQuad->unsetTexture();
                m_spScreenQuad->draw(*m_spLightingPassShader, m_spLightingTextureRegistry.get());

                m_spMainFb->deactivate();
            }

            // Step 3: forward render anything else on top.
            {
                Cme::DebugGroup debugGroup("Forward pass");

                // Before we do so, we have to blit the depth buffer.
                m_spGBuffer->blit(*m_spMainFb, GL_DEPTH_BUFFER_BIT);

                m_spMainFb->activate();

                if (m_OptsObj.drawNormals)
                {
                    // Draw the normals.
                    m_spNormalShader->updateUniforms();
                    m_upModel->draw(*m_spNormalShader);
                }

                // Draw light source.
                m_spLampShader->updateUniforms();
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->enableWireframe();
                }
                // TODO: Make point lights more part of the UI.
                // lightSphere.draw(lampShader);
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->disableWireframe();
                }

                // Draw skybox.
                m_spSkyboxShader->updateUniforms();
                m_spSkybox->draw(*m_spSkyboxShader);

                m_spMainFb->deactivate();
            }

            // Step 4: post processing.
            if (m_OptsObj.bloom)
            {
                Cme::DebugGroup debugGroup("Bloom pass");
                m_spBloomPass->multipassDraw(*m_spMainFb);
            }

            {
                Cme::DebugGroup debugGroup("Tonemap & gamma");
                m_spFinalFb->activate();
                m_spFinalFb->clear();

                // Draw to the final FB using the post process shader.
                m_spPostprocessShader->updateUniforms();
                m_spPostprocessShader->setBool("bloom", m_OptsObj.bloom);
                m_spPostprocessShader->setFloat("bloomMix", m_OptsObj.bloomMix);
                m_spPostprocessShader->setInt("toneMapping", static_cast<int>(m_OptsObj.toneMapping));
                m_spPostprocessShader->setBool("gammaCorrect", m_OptsObj.gammaCorrect);
                m_spPostprocessShader->setFloat("gamma", static_cast<int>(m_OptsObj.gamma));
                m_spScreenQuad->setTexture(m_MainColorAttachmentObj);
                m_spScreenQuad->draw(*m_spPostprocessShader, m_spPostprocessTextureRegistry.get());

                m_spFinalFb->deactivate();
            }

            m_pWindow->setViewport();

            // Finally draw to the screen via the FXAA shader.
            if (m_OptsObj.fxaa)
            {
                Cme::DebugGroup debugGroup("FXAA");
                m_spScreenQuad->setTexture(m_FinalColorAttachmentObj);
                m_spScreenQuad->draw(*m_spFxaaShader);
            }
            else
            {
                m_spFinalFb->blitToDefault(GL_COLOR_BUFFER_BIT);
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

        return true;
	}

	void App::Restart()
	{

	}

	void App::Close()
	{

	}


    void App::RenderImGuiUI(ModelRenderOptions& opts, Cme::Camera camera, Cme::ShadowMap shadowMap, Cme::SsaoBuffer ssaoBuffer)
    {
        ImGui::Begin("Model Render");

        constexpr float IMAGE_BASE_SIZE = 160.0f;

        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Perform some shenanigans so that the gizmo rotates along with the
            // camera while still representing the same model rotation.
            glm::quat rotViewSpace = glm::quat_cast(camera.getViewTransform()) * opts.modelRotation;
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

    std::unique_ptr<Cme::Model> App::LoadModelOrDefault()
    {
        // Default to the gltf DamagedHelmet.
        auto helmet = std::make_unique<Cme::Model>("assets//models//DamagedHelmet/DamagedHelmet.gltf");
        return helmet;
    }

    /** Loads a skybox image as a cubemap and generates IBL info. */
    void App::LoadSkyboxImage(
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

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    void App::imguiHelpMarker(const char* desc)
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

    // Helper for a float slider value.
    bool App::imguiFloatSlider(const char* desc, float* value, float min,
        float max, const char* fmt, Scale scale)
    {
        ImGuiSliderFlags flags = ImGuiSliderFlags_None;
        if (scale == Scale::LOG)
        {
            flags = ImGuiSliderFlags_Logarithmic;
        }
        return ImGui::SliderScalar(desc, ImGuiDataType_Float, value, &min, &max, fmt, flags);
    }

    // Helper for an image control.
    void App::imguiImage(const Cme::Texture& texture, glm::vec2 size)
    {
        ImTextureID texID = reinterpret_cast<void*>(texture.getId());
        // Flip the image.
        ImGui::Image(texID, ImVec2(size.x, size.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        //ImGui::Image(texID, size, /*uv0=*/glm::vec2(0.0f, 1.0f),
    }
}


