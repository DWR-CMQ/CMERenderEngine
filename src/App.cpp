#include "App.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


#include "../imguizmo_quat/imGuIZMOquat.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

/// @brief 从原理上来看 原本的texture_uniform_source作用就是管理多个Texture(实现纹理绑定和纹理索引) 当texture_uniform_source里面只有一个纹理时 这个文件就已经
/// 没有作用了 因为默认的纹理索引就是0 所以提前做好纹理绑定 就不用这个文件了
/// 把TextureUniformSource看作是纹理的集合即可 然后再加到某一个Shader中 由Shader更新纹理
/// 下一步需要把TextureUniformSource全部清除 只用texturemanager做统一管理
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

        // 各个Manager
        auto& tm = TextureManager::GetInstance();

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

        // Build the G-Buffer and prepare deferred shading.
        m_spGeometryPassShader = std::make_shared<Cme::DeferredGeometryPassShader>();           

        // GBuffer
        m_spGBuffer = std::make_shared<Cme::GBuffer>(m_pWindow->getSize());
        tm.AddTexture("gbuffer", m_spGBuffer->GetAllTexture());

        // Screen
        m_spScreenQuad = std::make_shared<Cme::ScreenQuadMesh>();
        m_spGBufferVisualShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//gbuffer_visual.frag"));
        m_spLightingPassShader = std::make_shared<Cme::ScreenShader>(Cme::ShaderPath("assets//model_shaders//lighting_pass.frag"));

        // 只有Shader才能更新Uniform变量 
        // 纹理和shader本来就是相对独立的东西 只是通过glBindTexture将两者结合在一起而已 在两者实例化对象的时候都是独立的
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
        // brdf就非常标准的按照opengl教程里来的 在loop循环中没有执行bind纹理就无法生效
        tm.AddTexture("brdf", std::vector<std::shared_ptr<Texture>>{m_spBrdfMap->getBrdfIntegrationMap()});

        // 天空盒
        m_spSkybox = std::make_shared<Cme::Skybox>(true, false, false);
        m_spSkybox->LoadSkyboxImage(m_OptsObj.skyboxImage);
        m_spSkyboxShader = std::make_shared<Cme::SkyboxShader>();

        // 模型
        m_ModelSceneObj.Init(m_pWindow->getSize());

        // 粒子
        m_pWaterFountainPS = new WaterFountainParticleSystem;
        m_pWaterFountainPS->max_particles = WATER_FOUNTAIN_MAX_PARTICLES;
        m_pWaterFountainPS->birth_rate = m_pWaterFountainPS->max_particles * 1000;
        m_pWaterFountainPS->InitPS(nullptr, 0);

        // 目前把这段代码加上后 粒子始终渲染不出来 暂不知道原因
        // m_pWindow->enableFaceCull();

        // 圆柱体
        m_spCylinder = std::make_shared<Cylinder>(1, 1, 2, 36, 1, true, 2);

        // 管道
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
            UI::RenderUI(m_OptsObj, *m_spCamera);

            auto& tm = TextureManager::GetInstance();

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

            // 粒子更新
            m_pWaterFountainPS->Update(deltaTime);

            // 管道更新
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

                // m_spGeometryPassShader需要相机的数据
                m_spGeometryPassShader->setMat4("view", m_spCamera->getViewTransform());
                m_spGeometryPassShader->setMat4("projection", m_spCamera->getProjectionTransform());

                // 线框模式 
                // 线框模式的设置，只能在编写完shaderprogram后才能设定，否则报错
                // 设置线框模式一定要在渲染之前 否则不生效 而且需要再次设置Fill模式 否则模型就渲染不出来
                if (m_OptsObj.wireframe)
                {
                    m_pWindow->enableWireframe();
                }
                // 渲染模型 

                m_ModelSceneObj.Render(m_OptsObj, m_spCamera, m_spGeometryPassShader);
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
                // 暂时这样写 用于更新m_spLightingPassShader里面的一些变量 看着不顺眼 但是是为了把texture_uniform_source给干掉
                // 如果使用m_spGBuffer->bindTexture 那么m_spScreenQuad->draw也要变化 是时候需要弄一个manager出来 专门管理TextureUniformSource
                m_spGBuffer->bindTexture(tm.GetTextureUnit("gbuffer"), *m_spLightingPassShader);              // 光照Shader用到GBuffer
                m_spBrdfMap->bindTexture(tm.GetTextureUnit("brdf"), *m_spLightingPassShader);                 // 光照Shader用到brdf
                m_spLightControl->updateUniforms(*m_spLightingPassShader);                                    // 光照Shader控制

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
                    m_ModelSceneObj.Render(m_OptsObj, m_spCamera, nullptr, true);
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
                m_spMainFb->activate();

                // Draw to the final FB using the post process shader.
                m_spPostprocessShader->setBool("bloom", m_OptsObj.bloom);
                m_spPostprocessShader->setFloat("bloomMix", m_OptsObj.bloomMix);
                m_spPostprocessShader->setInt("toneMapping", static_cast<int>(m_OptsObj.toneMapping));
                m_spPostprocessShader->setBool("gammaCorrect", m_OptsObj.gammaCorrect);
                m_spPostprocessShader->setFloat("gamma", static_cast<int>(m_OptsObj.gamma));
                m_spScreenQuad->setTexture(m_spMainFb->GetTexture());

                // 后处理有纹理更新 
                m_spScreenQuad->draw(*m_spPostprocessShader);

                m_spMainFb->deactivate();
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
            m_spMainFb->blitToDefault(GL_COLOR_BUFFER_BIT);

            //}

            // 渲染粒子 输入正向渲染
            // 按照Opengl教程中延迟着色法的章节中的----结合延迟渲染和正向渲染
            // 可知粒子渲染一定要在glBlitFramebuffer之后
            // 还有就是要注释Init中的enableFaceCull()
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


