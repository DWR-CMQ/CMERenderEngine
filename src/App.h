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

        // ShadowMap ��Ӱӳ��
        std::shared_ptr<Cme::ShadowMap> m_spShadowMap;
        std::shared_ptr<Cme::ShadowMapShader> m_spShadowMapShader;
        std::shared_ptr<Cme::ShadowMapCamera> m_spShadowMapCamera;
        // ShadowMap ��Ӱӳ��

        // Model
        std::unique_ptr<Cme::Model> m_upModel;
        // Model

        std::shared_ptr<Cme::DirectionalLight> m_spDirectionalLight;
        
        // IBL
        std::shared_ptr<Cme::PrefilterMap> m_spPrefilterMap;                       // Ԥ�����ͼ
        std::shared_ptr<Cme::EquirectCubemap> m_spEquirectCubeMap;                 // ��������ͼ
        std::shared_ptr<Cme::IrradianceMap> m_spIrradianceMap;                     // ������ͼ
        // IBL

        // GBuffer 
        // �ӳ���Ⱦ���������׶�(pass) 
        // ��һ�����δ���׶�(GeometryPass)�� ����Ⱦ����һ�� ��ȡ����ĸ��ּ�����Ϣ ���洢�ڽ���GBuffer��������
        // �ڶ������մ���׶�(LightingPass) ������׶λ���Ⱦһ����Ļ��С�ķ��� ����G�����еļ������ݶ�ÿһ��Ƭ�μ��㳡������
        std::shared_ptr<Cme::GBuffer> m_spGBuffer;                                  // ������Ⱦ�׶� ��ʼ��һ��֡������� Ҳ����GBuffer
        std::shared_ptr<Cme::ScreenQuadMesh> m_spScreenQuad;                        // ��Ⱦһ��������Ļ���ı��Σ���ʹ�ô洢��G�������еļ�����Ϣ����ÿ��Ƭ�εĳ�������    
        std::shared_ptr<Cme::DeferredGeometryPassShader> m_spGeometryPassShader;    // Defer��Ⱦ�ļ��δ���׶�
        std::shared_ptr<Cme::ScreenShader> m_spLightingPassShader;                  // Defer��Ⱦ�Ĺ��մ���׶�-----pbr��Ⱦ(���պ�����)����Ҫ�õ����shader
        std::shared_ptr<Cme::ScreenShader> m_spGBufferVisualShader;                 // GBuffer���ӻ�����Ҫ��Shader
        // GBuffer

        // ***
        std::shared_ptr<Cme::TextureUniformSource> m_spLightingTextureUniformSource;

        // Skybox
        std::shared_ptr<Cme::SkyboxShader> m_spSkyboxShader;
        std::shared_ptr<Cme::Skybox> m_spSkybox;
        // Skybox

        // Opengl�̳������������в���������Ĭ��֡�������Ⱦ�����Ͻ��еġ�Ĭ�ϵ�֡���������㴴�����ڵ�ʱ�����ɺ����õģ�GLFW������������Щ
        // �����ΪʲôOpengl�̳�8.1��������û�о��Ϊ0��FBO,��ȴ���԰�,��ΪĬ�����ɵ�FBO�����0 
        // �ڱ������� û�в���Ĭ�ϵ�֡���� ���Ƕ���������һ��FBO Ҳ����m_spMainFb �������˼�� ���е��߼�����˵���
        std::shared_ptr<Cme::Framebuffer> m_spMainFb;
        Attachment m_MainColorAttachmentObj;
        // ר�����ں����FBO ��GBuffer��Blit����
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

