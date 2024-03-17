#pragma once
#include "quarkgl.h"
#include <memory>
#include <iostream>

//const char* lampShaderSource = R"SHADER(
//    #version 460 core
//    out vec4 fragColor;
//
//    void main() { fragColor = vec4(1.0); }
//    )SHADER";
//
//const char* normalShaderSource = R"SHADER(
//    #version 460 core
//    out vec4 fragColor;
//
//    void main() { fragColor = vec4(1.0, 1.0, 0.0, 1.0); }
//    )SHADER";

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

        // ShadowMap “ı”∞”≥…‰
        std::shared_ptr<Cme::ShadowMap> m_spShadowMap;
        std::shared_ptr<Cme::ShadowMapShader> m_spShadowShader;

        // Model
        std::unique_ptr<Cme::Model> m_upModel;

        std::shared_ptr<Cme::DirectionalLight> m_spDirectionalLight;

        std::shared_ptr<Cme::SkyboxMesh> m_spSkybox;
        std::shared_ptr<Cme::ShadowCamera> m_spShadowCamera;

        std::shared_ptr<Cme::GGXPrefilteredEnvMapCalculator> m_spPrefilteredEnvMapCalculator;

        std::shared_ptr<Cme::EquirectCubemapConverter> m_spEquirectCubemapConverter;
        std::shared_ptr<Cme::CubemapIrradianceCalculator> m_spIrradianceCalculator;

        std::shared_ptr<Cme::GBuffer> m_spGBuffer;

        std::shared_ptr<Cme::DeferredGeometryPassShader> m_spGeometryPassShader;

        std::shared_ptr<Cme::ScreenQuadMesh> m_spScreenQuad;
        std::shared_ptr<Cme::ScreenShader> m_spGBufferVisShader;
        std::shared_ptr<Cme::ScreenShader> m_spLightingPassShader;

        // SSAO
        //Cme::SsaoShader m_SsaoShaderObj;
        std::shared_ptr<Cme::SsaoShader> m_spSsaoShader;
        std::shared_ptr<Cme::SsaoKernel> m_spSsaoKernel;
        std::shared_ptr<Cme::SsaoBuffer> m_spSsaoBuffer;
        std::shared_ptr<Cme::SsaoBuffer> m_spSsaoBlurredBuffer;
        //Cme::SsaoBlurShader m_SsaoBlurShaderObj;
        std::shared_ptr<Cme::SsaoBlurShader> m_spSsaoBlurShader;

        std::shared_ptr<Cme::TextureRegistry> m_spSsaoTextureRegistry;
        std::shared_ptr<Cme::TextureRegistry> m_spLightingTextureRegistry;

        // Skybox
        // Cme::SkyboxShader m_SkyboxShaderObj;
        std::shared_ptr<Cme::SkyboxShader> m_spSkyboxShader;

        // Debug Shader
        std::shared_ptr<Cme::Shader> m_spNormalShader;
        std::shared_ptr<Cme::Shader> m_spLampShader;

        //
        std::shared_ptr<Cme::Framebuffer> m_spMainFb;
        Attachment m_MainColorAttachmentObj;
        std::shared_ptr<Cme::Framebuffer> m_spFinalFb;
        Attachment m_FinalColorAttachmentObj;

        std::shared_ptr<Cme::BloomPass> m_spBloomPass;

        // fxaa
        // Cme::FXAAShader m_FxaaShaderObj;
        std::shared_ptr<Cme::FXAAShader> m_spFxaaShader;

        std::shared_ptr<Cme::TextureRegistry> m_spPostprocessTextureRegistry;
        std::shared_ptr<Cme::ScreenShader> m_spPostprocessShader;
	};
}

