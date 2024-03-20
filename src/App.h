#pragma once
#include "quarkgl.h"
#include <memory>
#include <iostream>

namespace Cme
{
    // Non-normative context for UI rendering. Used for accessing renderer info.
    struct UIContext
    {
        Cme::Camera& camera;
        Cme::ShadowMap& shadowMap;
        Cme::SsaoBuffer& ssaoBuffer;
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
        glm::vec3 directionalDirection = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));

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

	class App
	{
	public:
		//static App* Instance();     

		void Init(bool bFullScreen = true);
		bool Run();
		void Restart();
		void Close();

        void RenderImGuiUI(ModelRenderOptions& opts, Cme::Camera camera, Cme::ShadowMap shadowMap, Cme::SsaoBuffer ssaoBuffer);
        std::unique_ptr<Cme::Model> LoadModelOrDefault();

    public:
        App();
        ~App();

	private:
		Cme::Window* m_pWindow;
        ModelRenderOptions m_OptsObj;
        std::shared_ptr<Cme::Camera> m_spCamera;
        std::shared_ptr<Cme::CameraControls> m_spCameraControls;

        // ShadowMap 阴影映射
        std::shared_ptr<Cme::ShadowMap> m_spShadowMap;
        std::shared_ptr<Cme::ShadowMapShader> m_spShadowMapShader;
        std::shared_ptr<Cme::ShadowMapCamera> m_spShadowMapCamera;

        // Model
        std::unique_ptr<Cme::Model> m_upModel;
        // Model

        std::shared_ptr<Cme::DirectionalLight> m_spDirectionalLight;
        
        // IBL
        std::shared_ptr<Cme::PrefilterMap> m_spPrefilterMap;                       // 预卷积贴图
        std::shared_ptr<Cme::EquirectCubemap> m_spEquirectCubeMap;                 // 立方体贴图
        std::shared_ptr<Cme::IrradianceMap> m_spIrradianceMap;                     // 辐照贴图
        // IBL

        std::shared_ptr<Cme::GBuffer> m_spGBuffer;                                  // 几何渲染阶段 初始化一个帧缓冲对象 也就是GBuffer
        std::shared_ptr<Cme::DeferredGeometryPassShader> m_spGeometryPassShader;    // GBuffer几何渲染阶段

        std::shared_ptr<Cme::ScreenQuadMesh> m_spScreenQuad;
        std::shared_ptr<Cme::ScreenShader> m_spGBufferVisShader;
        std::shared_ptr<Cme::ScreenShader> m_spLightingPassShader;

        // SSAO
        std::shared_ptr<Cme::SsaoShader> m_spSsaoShader;              
        std::shared_ptr<Cme::SsaoBlurShader> m_spSsaoBlurShader;      
        
        std::shared_ptr<Cme::SsaoBuffer> m_spSsaoBuffer;                // 环境遮蔽
        std::shared_ptr<Cme::SsaoBuffer> m_spSsaoBlurredBuffer;         // 环境遮蔽模糊

        std::shared_ptr<Cme::SsaoKernel> m_spSsaoKernel;
        std::shared_ptr<Cme::TextureRegistry> m_spSsaoTextureRegistry;  // Render的时候更新Uniform变量
        // SSAO 

        // ***
        std::shared_ptr<Cme::TextureRegistry> m_spLightingTextureRegistry;

        // Skybox
        std::shared_ptr<Cme::SkyboxShader> m_spSkyboxShader;
        std::shared_ptr<Cme::SkyboxMesh> m_spSkybox;
        // Skybox

        std::shared_ptr<Cme::Framebuffer> m_spMainFb;
        Attachment m_MainColorAttachmentObj;
        std::shared_ptr<Cme::Framebuffer> m_spFinalFb;
        Attachment m_FinalColorAttachmentObj;

        // Bloom
        std::shared_ptr<Cme::BloomPass> m_spBloomPass;
        // Bloom

        // FXAA
        std::shared_ptr<Cme::FXAAShader> m_spFxaaShader;
        // FXAA

        // PostProcess
        std::shared_ptr<Cme::TextureRegistry> m_spPostprocessTextureRegistry;
        std::shared_ptr<Cme::ScreenShader> m_spPostprocessShader;
        // PostProcess

        // Debug Shader
        std::shared_ptr<Cme::Shader> m_spNormalShader;
        std::shared_ptr<Cme::Shader> m_spLampShader;
	};
}

