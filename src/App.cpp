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
        // glfwDestroyWindow(m_pWindow);
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

        // Setup the camera.
        m_spCamera = std::make_shared<Cme::Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
        m_spCameraControls = std::make_shared<Cme::OrbitCameraControls>(*m_spCamera);

        m_pWindow->bindCamera(m_spCamera);
        m_pWindow->bindCameraControls(m_spCameraControls);

        // Create light registry and add lights.
        // 因为有多种光源 所以用多个类表示
        m_spLightControl = std::make_shared<Cme::LightControl>(m_spCamera->getViewTransform());
        m_spDirectionalLight = std::make_shared<Cme::DirectionalLight>();
        m_spLightControl->AddLight(m_spDirectionalLight);

        TextureParams params;
        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        m_spMainFb = std::make_shared<Cme::Framebuffer>(m_pWindow->getSize(), Cme::BufferType::COLOR_HDR_ALPHA, params);
        m_spMainFb->attachRenderbuffer(Cme::BufferType::DEPTH_AND_STENCIL);

        m_spFinalFb = std::make_shared<Cme::Framebuffer>(m_pWindow->getSize(), Cme::BufferType::COLOR_ALPHA, params);

        // Build the G-Buffer and prepare deferred shading.
        m_spGeometryPassShader = std::make_shared<Cme::DeferredGeometryPassShader>();
        m_spGeometryPassShader->addUniformSource(m_spCamera);                // 说明m_spGeometryPassShader需要相机的数据

        // GBuffer
        m_spGBuffer = std::make_shared<Cme::GBuffer>(m_pWindow->getSize());
        m_spLightingTextureUniformSource = std::make_shared<Cme::TextureUniformSource>();
        m_spLightingTextureUniformSource->addTextureSource(m_spGBuffer);

        // Screen
        m_spScreenQuad = std::make_shared<Cme::ScreenQuadMesh>();
        m_spGBufferVisualShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//gbuffer_visual.frag"));
        m_spLightingPassShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//lighting_pass.frag"));

        // 光照Shader用到GBuffer
        m_spLightingPassShader->addUniformSource(m_spLightingTextureUniformSource);

        // 把TextureUniformSource看作是纹理的集合即可 然后再加到某一个Shader中 由Shader更新纹理
        m_spPostprocessTextureUniformSource = std::make_shared<Cme::TextureUniformSource>();

        // 只有Shader才能更新Uniform变量 
        // 纹理和shader本来就是相对独立的东西 只是通过glBindTexture将两者结合在一起而已 在两者实例化对象的时候都是独立的
        m_spPostprocessShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//post_processing.frag"));
        m_spPostprocessShader->addUniformSource(m_spPostprocessTextureUniformSource);

        // FXAA
        m_spFxaaShader = std::make_shared<Cme::FXAAShader>();

        // IBL
        constexpr int CUBEMAP_SIZE = 1024;

        // BRDF
        m_spBrdfMap = std::make_shared<Cme::BrdfMap>(CUBEMAP_SIZE, CUBEMAP_SIZE);
        {
            // Only needs to be calculated once up front.
            Cme::DebugGroup debugGroup("BRDF LUT calculation");
            m_spBrdfMap->draw();
        }
        // brdf就非常标准的按照opengl教程里来的 在loop循环中没有执行bind纹理就无法生效
        m_spLightingTextureUniformSource->addTextureSource(m_spBrdfMap);                          /// ----------------------------------

        // 天空盒
        m_spSkybox = std::make_shared<Cme::Skybox>(true, false, false);
        m_spSkybox->LoadSkyboxImage(m_OptsObj.skyboxImage);
        m_spSkyboxShader = std::make_shared<Cme::SkyboxShader>();

        m_ModelSceneObj.Init(m_spCamera, m_pWindow->getSize());

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

            // 初始化编辑器
            m_OptsObj.speed = m_spCameraControls->getSpeed();
            m_OptsObj.sensitivity = m_spCameraControls->getSensitivity();
            m_OptsObj.fov = m_spCamera->getFov();
            m_OptsObj.near = m_spCamera->getNearPlane();
            m_OptsObj.far = m_spCamera->getFarPlane();

            m_OptsObj.frameDeltas = m_pWindow->getFrameDeltas();
            m_OptsObj.numFrameDeltas = m_pWindow->getNumFrameDeltas();
            m_OptsObj.frameDeltasOffset = m_pWindow->getFrameDeltasOffset();
            m_OptsObj.avgFPS = m_pWindow->getAvgFPS();

            // 渲染编辑器
            RenderImGuiUI(m_OptsObj, *m_spCamera);

            // 渲染光照 方向和光照强度来自编辑器
            m_spDirectionalLight->setDiffuse(m_OptsObj.directionalDiffuse * m_OptsObj.directionalIntensity);
            m_spDirectionalLight->setSpecular(m_OptsObj.directionalSpecular * m_OptsObj.directionalIntensity);
            m_spDirectionalLight->setDirection(m_OptsObj.directionalDirection);

            m_spCameraControls->setSpeed(m_OptsObj.speed);
            m_spCameraControls->setSensitivity(m_OptsObj.sensitivity);
            m_spCamera->setFov(m_OptsObj.fov);
            m_spCamera->setNearPlane(m_OptsObj.near);
            m_spCamera->setFarPlane(m_OptsObj.far);

            m_pWindow->setMouseButtonBehavior(m_OptsObj.captureMouse
                ? Cme::MouseButtonBehavior::CAPTURE_MOUSE
                : Cme::MouseButtonBehavior::NONE);

            // 摄像机控制模式
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

            // 更新天空盒
            if (m_OptsObj.skyboxImage != prevOpts.skyboxImage)
            {
                m_spSkybox->LoadSkyboxImage(m_OptsObj.skyboxImage);
            }

            // G-Buffer步骤1 Geometry Pass. 符合Opengl教程的逻辑
            {
                Cme::DebugGroup debugGroup("Geometry pass");
                m_spGBuffer->activate();
                m_spGBuffer->clear();

                m_spGeometryPassShader->updateUniforms();

                // 线框模式 
                // 线框模式的设置，只能在编写完shaderprogram后才能设定，否则报错
                // 设置线框模式一定要在渲染之前 否则不生效 而且需要再次设置Fill模式 否则模型就渲染不出来
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->enableWireframe();
                }
                // 渲染模型 
                m_ModelSceneObj.Render(m_OptsObj, m_spGeometryPassShader);
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->disableWireframe();
                }

                m_spGBuffer->deactivate();
            }

            // 存在于GBuffer中的各个纹理
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
                    m_spGBufferVisualShader->setInt("gBufferVis", static_cast<int>(m_OptsObj.gBufferVis));
                    // GBuffer无纹理更新
                    m_spScreenQuad->draw(*m_spGBufferVisualShader);
                }

                // TODO: Refactor avoid needing to copy this.
                {
                    Cme::DebugGroup debugGroup("Imgui pass");
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                }
                return;
            }

            // G-Buffer步骤2 Lighting Pass. Draw to the main framebuffer.
            {
                Cme::DebugGroup debugGroup("Deferred lighting pass");
                // m_spMainFb的id为1
                m_spMainFb->activate();
                m_spMainFb->clear();

                // TODO: Set up environment mapping with the skybox.
                m_spLightingPassShader->updateUniforms();

                // 暂时这样写 用于更新m_spLightingPassShader里面的一些变量 看着不顺眼 但是是为了把texture_uniform_source给干掉
                m_spLightControl->updateUniforms(*m_spLightingPassShader);  

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
                m_spLightingPassShader->setFloat("emissionAttenuation.constant", m_OptsObj.emissionAttenuation.x);
                m_spLightingPassShader->setFloat("emissionAttenuation.linear", m_OptsObj.emissionAttenuation.y);
                m_spLightingPassShader->setFloat("emissionAttenuation.quadratic", m_OptsObj.emissionAttenuation.z);

                m_spLightingPassShader->setMat4("view", m_spCamera->getViewTransform());
                m_spLightingPassShader->setMat4("projection", m_spCamera->getProjectionTransform());

                //m_spBrdfMap->bindTexture();

                //m_spScreenQuad->unsetTexture();
                m_spScreenQuad->draw(*m_spLightingPassShader, m_spLightingTextureUniformSource.get());

                m_spMainFb->deactivate();
            }

            // 正常的前向渲染
            // 普通的粒子系统就可以在这里面渲染
            {
                Cme::DebugGroup debugGroup("Forward pass");

                // Before we do so, we have to blit the depth buffer.
                m_spGBuffer->blit(*m_spMainFb, GL_DEPTH_BUFFER_BIT);

                m_spMainFb->activate();

                if (m_OptsObj.drawNormals)
                {
                    // Draw the normals.
                    m_ModelSceneObj.Render(m_OptsObj, nullptr, true);
                }

                //// Draw light source.
                //m_spLampShader->updateUniforms();

                // 绘制天空盒 天空盒在正向渲染里
                m_spSkybox->Render(*m_spSkyboxShader, m_spCamera);

                m_spMainFb->deactivate();
            }

            // 后处理
            {
                Cme::DebugGroup debugGroup("Tonemap & gamma");
                // m_spFinalFb的id为2
                m_spFinalFb->activate();
                m_spFinalFb->clear();

                // Draw to the final FB using the post process shader.
                m_spPostprocessShader->updateUniforms();
                m_spPostprocessShader->setBool("bloom", m_OptsObj.bloom);
                m_spPostprocessShader->setFloat("bloomMix", m_OptsObj.bloomMix);
                m_spPostprocessShader->setInt("toneMapping", static_cast<int>(m_OptsObj.toneMapping));
                m_spPostprocessShader->setBool("gammaCorrect", m_OptsObj.gammaCorrect);
                m_spPostprocessShader->setFloat("gamma", static_cast<int>(m_OptsObj.gamma));
                //m_spScreenQuad->setTexture(m_MainColorAttachmentObj);
                m_spScreenQuad->setTexture(m_spMainFb->GetTexture());

                // 后处理有纹理更新
                m_spScreenQuad->draw(*m_spPostprocessShader, m_spPostprocessTextureUniformSource.get());

                m_spFinalFb->deactivate();
            }

            m_pWindow->setViewport();

            //// Finally draw to the screen via the FXAA shader.
            //if (m_OptsObj.fxaa)
            //{
            //    Cme::DebugGroup debugGroup("FXAA");
            //    m_spScreenQuad->setTexture(m_FinalColorAttachmentObj);
            //    // fxaa无纹理更新
            //    m_spScreenQuad->draw(*m_spFxaaShader);
            //}
            //else
            //{
            m_spFinalFb->blitToDefault(GL_COLOR_BUFFER_BIT);
            //}

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

    void App::RenderImGuiUI(ModelRenderOptions& opts, Cme::Camera camera)
    {
        ImGui::Begin("Model Render");

        constexpr float IMAGE_BASE_SIZE = 160.0f;

        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Perform some shenanigans so that the gizmo rotates along with the
            // camera while still representing the same model rotation.
            glm::quat rotViewSpace = glm::quat_cast(camera.getViewTransform()) * opts.modelRotation;
            ImGui::gizmo3D("Model rotation", rotViewSpace, IMAGE_BASE_SIZE);
            opts.modelRotation = glm::quat_cast(glm::inverse(camera.getViewTransform())) * glm::normalize(rotViewSpace);

            ImGui::SameLine();

            // Perform some shenanigans so that the gizmo rotates along with the
            // camera while still representing the same light dir..
            glm::vec3 dirViewSpace = glm::vec3(camera.getViewTransform() * glm::vec4(opts.directionalDirection, 0.0f));
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
            CommonHelper::imguiFloatSlider("Model scale", &opts.modelScale, 0.0001f, 100.0f, "%.04f",
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
            CommonHelper::imguiHelpMarker("Which lighting model to use for shading.");

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
                CommonHelper::imguiHelpMarker("Whether to lock the specular light color to the diffuse. Usually "
                    "desired for PBR.");
                if (lockSpecular)
                {
                    opts.directionalSpecular = opts.directionalDiffuse;
                }
                CommonHelper::imguiFloatSlider("Intensity", &opts.directionalIntensity, 0.0f, 50.0f, nullptr, Scale::LINEAR);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Emission lights"))
            {
                CommonHelper::imguiFloatSlider("Emission intensity", &opts.emissionIntensity, 0.0f,
                    1000.0f, nullptr, Scale::LOG);
                ImGui::DragFloat3("Emission attenuation",
                    reinterpret_cast<float*>(&opts.emissionAttenuation),
                    /*v_speed=*/0.01f, 0.0f, 10.0f);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("Constant, linear, and quadratic attenuation of emission lights.");

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Environment"))
            {
                ImGui::Combo("Skybox image", reinterpret_cast<int*>(&opts.skyboxImage),
                    "Alex's apt\0Frozen waterfall\0Kloppenheim\0Milkyway\0Mon "
                    "Valley\0Ueno shrine\0Winter forest\0Six_Face\0");

                ImGui::BeginDisabled(opts.lightingModel == LightingModel::BLINN_PHONG);
                ImGui::Checkbox("Use IBL", &opts.useIBL);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(opts.lightingModel != LightingModel::BLINN_PHONG &&
                    opts.useIBL);
                ImGui::ColorEdit3("Ambient color",
                    reinterpret_cast<float*>(&opts.ambientColor),
                    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("The color of the fixed ambient component.");
                ImGui::EndDisabled();

                ImGui::BeginDisabled(opts.lightingModel != LightingModel::BLINN_PHONG);
                CommonHelper::imguiFloatSlider("Shininess", &opts.shininess, 1.0f, 1000.0f, nullptr,
                    Scale::LOG);
                ImGui::SameLine();
                CommonHelper::imguiHelpMarker("Shininess of specular highlights. Only applies to Phong.");
                ImGui::EndDisabled();

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Post-processing"))
            {
                ImGui::Combo(
                    "Tone mapping", reinterpret_cast<int*>(&opts.toneMapping),
                    "None\0Reinhard\0Reinhard luminance\0ACES (approx)\0AMD\0\0");
                ImGui::Checkbox("Gamma correct", &opts.gammaCorrect);
                ImGui::BeginDisabled(!opts.gammaCorrect);
                CommonHelper::imguiFloatSlider("Gamma", &opts.gamma, 0.01f, 8.0f, nullptr, Scale::LOG);

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

            CommonHelper::imguiFloatSlider("Speed", &opts.speed, 0.1, 50.0);
            CommonHelper::imguiFloatSlider("Sensitivity", &opts.sensitivity, 0.01, 1.0, nullptr,
                Scale::LOG);
            CommonHelper::imguiFloatSlider("FoV", &opts.fov, Cme::MIN_FOV, Cme::MAX_FOV, "%.1f°");
            if (CommonHelper::imguiFloatSlider("Near plane", &opts.near, 0.01, 1000.0, nullptr,
                Scale::LOG))
            {
                if (opts.near > opts.far)
                {
                    opts.far = opts.near;
                }
            }
            if (CommonHelper::imguiFloatSlider("Far plane", &opts.far, 0.01, 1000.0, nullptr, Scale::LOG))
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
            CommonHelper::imguiHelpMarker("What component of the G-Buffer to visualize.");

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

}


