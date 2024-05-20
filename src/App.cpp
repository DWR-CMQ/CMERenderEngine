#include "App.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


#include "../imguizmo_quat/imGuIZMOquat.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

/// @brief ��ԭ�������� ԭ����texture_uniform_source���þ��ǹ�����Texture(ʵ������󶨺���������) ��texture_uniform_source����ֻ��һ������ʱ ����ļ����Ѿ�
/// û�������� ��ΪĬ�ϵ�������������0 ������ǰ��������� �Ͳ�������ļ���
/// ��TextureUniformSource����������ļ��ϼ��� Ȼ���ټӵ�ĳһ��Shader�� ��Shader��������
/// ��һ����Ҫ��TextureUniformSourceȫ����� ֻ��texturemanager��ͳһ����
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
        delete m_pWaterFountainPS;
    }

    void App::Init(bool bFullScreen)
    {
        m_pWindow = new Cme::Window(1920, 1080, "Model Render", false, 0);
        m_pWindow->setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
        m_pWindow->setEscBehavior(Cme::EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE);

        // Setup Dear ImGui.
        UI::SetupUI(m_pWindow->GetWindow());

        // ����Manager
        auto& tm = TextureManager::GetInstance();

        // Setup the camera.
        m_spCamera = std::make_shared<Cme::Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
        m_spCameraControls = std::make_shared<Cme::OrbitCameraControls>(*m_spCamera);

        m_pWindow->bindCamera(m_spCamera);
        m_pWindow->bindCameraControls(m_spCameraControls);

        // Create light registry and add lights.
        // ��Ϊ�ж��ֹ�Դ �����ö�����ʾ
        m_spLightControl = std::make_shared<Cme::LightControl>(m_spCamera->getViewTransform());
        m_spDirectionalLight = std::make_shared<Cme::DirectionalLight>();
        m_spLightControl->AddLight(m_spDirectionalLight);

        TextureParams params;
        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        m_spMainFb = std::make_shared<Cme::Framebuffer>(m_pWindow->getSize(), Cme::BufferType::COLOR_HDR_ALPHA, params);
        m_spMainFb->attachRenderbuffer(Cme::BufferType::DEPTH_AND_STENCIL);

        // Build the G-Buffer and prepare deferred shading.
        m_spGeometryPassShader = std::make_shared<Cme::DeferredGeometryPassShader>();           

        // GBuffer
        m_spGBuffer = std::make_shared<Cme::GBuffer>(m_pWindow->getSize());
        tm.AddTexture("gbuffer", m_spGBuffer->GetAllTexture());

        // Screen
        m_spScreenQuad = std::make_shared<Cme::ScreenQuadMesh>();
        m_spGBufferVisualShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//gbuffer_visual.frag"));
        m_spLightingPassShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//lighting_pass.frag"));

        // ֻ��Shader���ܸ���Uniform���� 
        // �����shader����������Զ����Ķ��� ֻ��ͨ��glBindTexture�����߽����һ����� ������ʵ���������ʱ���Ƕ�����
        m_spPostprocessShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//post_processing.frag"));

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
        // brdf�ͷǳ���׼�İ���opengl�̳������� ��loopѭ����û��ִ��bind������޷���Ч
        tm.AddTexture("brdf", std::vector<std::shared_ptr<Texture>>{m_spBrdfMap->getBrdfIntegrationMap()});

        // ��պ�
        m_spSkybox = std::make_shared<Cme::Skybox>(true, false, false);
        m_spSkybox->LoadSkyboxImage(m_OptsObj.skyboxImage);
        m_spSkyboxShader = std::make_shared<Cme::SkyboxShader>();

        // ģ��
        m_ModelSceneObj.Init(m_pWindow->getSize());

        // ����
        m_pWaterFountainPS = new WaterFountainParticleSystem;
        m_pWaterFountainPS->max_particles = WATER_FOUNTAIN_MAX_PARTICLES;
        m_pWaterFountainPS->birth_rate = m_pWaterFountainPS->max_particles * 1000;
        m_pWaterFountainPS->InitPS(nullptr, 0);

        // Ŀǰ����δ�����Ϻ� ����ʼ����Ⱦ������ �ݲ�֪��ԭ��
        // m_pWindow->enableFaceCull();

        // Բ����
        m_spCylinder = std::make_shared<Cylinder>(1, 1, 2, 36, 1, true, 2);

        // �ܵ�
        m_spPipe = std::make_shared<Pipe>();
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

            m_pWaterFountainPS->Upload();

            ModelRenderOptions prevOpts = m_OptsObj;

            // ��ʼ���༭��
            m_OptsObj.speed = m_spCameraControls->getSpeed();
            m_OptsObj.sensitivity = m_spCameraControls->getSensitivity();
            m_OptsObj.fov = m_spCamera->getFov();
            m_OptsObj.near = m_spCamera->getNearPlane();
            m_OptsObj.far = m_spCamera->getFarPlane();

            m_OptsObj.frameDeltas = m_pWindow->getFrameDeltas();
            m_OptsObj.numFrameDeltas = m_pWindow->getNumFrameDeltas();
            m_OptsObj.frameDeltasOffset = m_pWindow->getFrameDeltasOffset();
            m_OptsObj.avgFPS = m_pWindow->getAvgFPS();

            // ��Ⱦ�༭��
            UI::RenderUI(m_OptsObj, *m_spCamera);

            auto& tm = TextureManager::GetInstance();

            // ��Ⱦ���� ����͹���ǿ�����Ա༭��
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

            // ���������ģʽ
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

            // ���Ӹ���
            m_pWaterFountainPS->Update(deltaTime);

            // �ܵ�����
            m_spPipe->SetThickness(m_OptsObj.fLoveThickness);
            m_spPipe->Update(static_cast<float>(glfwGetTime()));

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

            // ������պ�
            if (m_OptsObj.skyboxImage != prevOpts.skyboxImage)
            {
                m_spSkybox->LoadSkyboxImage(m_OptsObj.skyboxImage);
            }

            // G-Buffer����1 Geometry Pass. ����Opengl�̵̳��߼�
            {
                Cme::DebugGroup debugGroup("Geometry pass");
                m_spGBuffer->activate();
                m_spGBuffer->clear();

                // m_spGeometryPassShader��Ҫ���������
                m_spGeometryPassShader->setMat4("view", m_spCamera->getViewTransform());
                m_spGeometryPassShader->setMat4("projection", m_spCamera->getProjectionTransform());

                // �߿�ģʽ 
                // �߿�ģʽ�����ã�ֻ���ڱ�д��shaderprogram������趨�����򱨴�
                // �����߿�ģʽһ��Ҫ����Ⱦ֮ǰ ������Ч ������Ҫ�ٴ�����Fillģʽ ����ģ�;���Ⱦ������
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->enableWireframe();
                }
                // ��Ⱦģ�� 

                m_ModelSceneObj.Render(m_OptsObj, m_spCamera, m_spGeometryPassShader);
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->disableWireframe();
                }

                m_spGBuffer->deactivate();
            }

            // ������GBuffer�еĸ�������
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
                    // GBuffer���������
                    m_spScreenQuad->draw(*m_spGBufferVisualShader);
                }

                // TODO: Refactor avoid needing to copy this.
                {
                    Cme::DebugGroup debugGroup("Imgui pass");
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                }
                return;
            }

            // G-Buffer����2 Lighting Pass. Draw to the main framebuffer.
            {
                Cme::DebugGroup debugGroup("Deferred lighting pass");
                // m_spMainFb��idΪ1
                m_spMainFb->activate();
                m_spMainFb->clear();

                // TODO: Set up environment mapping with the skybox.
                // ��ʱ����д ���ڸ���m_spLightingPassShader�����һЩ���� ���Ų�˳�� ������Ϊ�˰�texture_uniform_source���ɵ�
                // ���ʹ��m_spGBuffer->bindTexture ��ôm_spScreenQuad->drawҲҪ�仯 ��ʱ����ҪŪһ��manager���� ר�Ź���TextureUniformSource
                m_spGBuffer->bindTexture(tm.GetTextureUnit("gbuffer"), *m_spLightingPassShader);              // ����Shader�õ�GBuffer
                m_spBrdfMap->bindTexture(tm.GetTextureUnit("brdf"), *m_spLightingPassShader);                 // ����Shader�õ�brdf
                m_spLightControl->updateUniforms(*m_spLightingPassShader);                                    // ����Shader����

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

                m_spScreenQuad->unsetTexture();
                m_spScreenQuad->draw(*m_spLightingPassShader);

                m_spMainFb->deactivate();
            }

            // ������ǰ����Ⱦ
            // ��ͨ������ϵͳ�Ϳ�������������Ⱦ
            {
                Cme::DebugGroup debugGroup("Forward pass");

                // Before we do so, we have to blit the depth buffer.
                m_spGBuffer->blit(*m_spMainFb, GL_DEPTH_BUFFER_BIT);

                m_spMainFb->activate();

                if (m_OptsObj.drawNormals)
                {
                    // Draw the normals.
                    m_ModelSceneObj.Render(m_OptsObj, m_spCamera, nullptr, true);
                }

                //// Draw light source.
                //m_spLampShader->updateUniforms();
                // ������պ� ��պ���������Ⱦ��
                m_spSkybox->Render(*m_spSkyboxShader, m_spCamera);
                m_spMainFb->deactivate();
            }

            // ����
            {
                Cme::DebugGroup debugGroup("Tonemap & gamma");
                m_spMainFb->activate();

                // Draw to the final FB using the post process shader.
                m_spPostprocessShader->setBool("bloom", m_OptsObj.bloom);
                m_spPostprocessShader->setFloat("bloomMix", m_OptsObj.bloomMix);
                m_spPostprocessShader->setInt("toneMapping", static_cast<int>(m_OptsObj.toneMapping));
                m_spPostprocessShader->setBool("gammaCorrect", m_OptsObj.gammaCorrect);
                m_spPostprocessShader->setFloat("gamma", static_cast<int>(m_OptsObj.gamma));
                m_spScreenQuad->setTexture(m_spMainFb->GetTexture());

                // ������������� 
                m_spScreenQuad->draw(*m_spPostprocessShader);

                m_spMainFb->deactivate();
            }

            m_pWindow->setViewport();

            //// Finally draw to the screen via the FXAA shader.
            //if (m_OptsObj.fxaa)
            //{
            //    Cme::DebugGroup debugGroup("FXAA");
            //    m_spScreenQuad->setTexture(m_FinalColorAttachmentObj);
            //    // fxaa���������
            //    m_spScreenQuad->draw(*m_spFxaaShader);
            //}
            //else
            //{
            m_spMainFb->blitToDefault(GL_COLOR_BUFFER_BIT);

            //}

            // ��Ⱦ���� ����������Ⱦ
            // ����Opengl�̳����ӳ���ɫ�����½��е�----����ӳ���Ⱦ��������Ⱦ
            // ��֪������Ⱦһ��Ҫ��glBlitFramebuffer֮��
            // ���о���Ҫע��Init�е�enableFaceCull()
            m_pWaterFountainPS->SetCamera(m_spCamera);
            m_pWaterFountainPS->SetParticleColor(m_OptsObj.vec3ParticleColor);
            m_pWaterFountainPS->Render();

            m_spCylinder->Render(m_spCamera);

            m_spPipe->SetColor(m_OptsObj.vec3LoveColor);
            m_spPipe->Render(m_spCamera);

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
}


