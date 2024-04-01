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

        void RenderImGuiUI(ModelRenderOptions& opts, Cme::Camera camera, Cme::ShadowMap shadowMap);
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
        // ShadowMap 阴影映射

        // Model
        std::unique_ptr<Cme::Model> m_upModel;
        // Model

        std::shared_ptr<Cme::DirectionalLight> m_spDirectionalLight;
        
        // IBL
        std::shared_ptr<Cme::PrefilterMap> m_spPrefilterMap;                       // 预卷积贴图
        std::shared_ptr<Cme::EquirectCubemap> m_spEquirectCubeMap;                 // 立方体贴图
        std::shared_ptr<Cme::IrradianceMap> m_spIrradianceMap;                     // 辐照贴图
        // IBL

        // GBuffer 
        // 延迟渲染包含两个阶段(pass) 
        // 第一个几何处理阶段(GeometryPass)中 先渲染场景一次 获取对象的各种几何信息 并存储在叫做GBuffer的纹理中
        // 第二个光照处理阶段(LightingPass) 在这个阶段会渲染一个屏幕大小的方形 并用G缓冲中的几何数据对每一个片段计算场景光照
        std::shared_ptr<Cme::GBuffer> m_spGBuffer;                                  // 几何渲染阶段 初始化一个帧缓冲对象 也就是GBuffer
        std::shared_ptr<Cme::ScreenQuadMesh> m_spScreenQuad;                        // 渲染一个充满屏幕的四边形，并使用存储在G缓冲区中的几何信息计算每个片段的场景光照    
        std::shared_ptr<Cme::DeferredGeometryPassShader> m_spGeometryPassShader;    // Defer渲染的几何处理阶段
        std::shared_ptr<Cme::ScreenShader> m_spLightingPassShader;                  // Defer渲染的光照处理阶段-----pbr渲染(光照和纹理)都需要用到这个shader
        std::shared_ptr<Cme::ScreenShader> m_spGBufferVisualShader;                 // GBuffer可视化所需要的Shader
        // GBuffer

        // ***
        std::shared_ptr<Cme::TextureUniformSource> m_spLightingTextureUniformSource;

        // Skybox
        std::shared_ptr<Cme::SkyboxShader> m_spSkyboxShader;
        std::shared_ptr<Cme::Skybox> m_spSkybox;
        // Skybox

        // Opengl教程上所做的所有操作都是在默认帧缓冲的渲染缓冲上进行的。默认的帧缓冲是在你创建窗口的时候生成和配置的（GLFW帮我们做了这些
        // 这就是为什么Opengl教程8.1上明明是没有句柄为0的FBO,但却可以绑定,因为默认生成的FBO句柄是0 
        // 在本工程中 没有采用默认的帧缓冲 而是独立生成了一个FBO 也就是m_spMainFb 基于这个思想 所有的逻辑就能说清楚
        std::shared_ptr<Cme::Framebuffer> m_spMainFb;
        Attachment m_MainColorAttachmentObj;
        // 专门用于后处理的FBO 从GBuffer中Blit而来
        std::shared_ptr<Cme::Framebuffer> m_spFinalFb;
        Attachment m_FinalColorAttachmentObj;

        // FXAA
        std::shared_ptr<Cme::FXAAShader> m_spFxaaShader;
        // FXAA

        // PostProcess
        std::shared_ptr<Cme::TextureUniformSource> m_spPostprocessTextureUniformSource;
        std::shared_ptr<Cme::ScreenShader> m_spPostprocessShader;
        // PostProcess

        // Debug Shader
        std::shared_ptr<Cme::Shader> m_spNormalShader;
        std::shared_ptr<Cme::Shader> m_spLampShader;
	};
}

