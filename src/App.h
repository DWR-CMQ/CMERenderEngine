#pragma once
#include "quarkgl.h"
#include <memory>
#include <iostream>

#include "scene/model_scene.h"

namespace Cme
{
    // Non-normative context for UI rendering. Used for accessing renderer info.
    struct UIContext
    {
        Cme::Camera& camera;
        Cme::ShadowMap& shadowMap;
        Cme::SsaoBuffer& ssaoBuffer;
    };

	class App
	{
	public:
		//static App* Instance();     

		void Init(bool bFullScreen = true);
		bool Run();
		void Restart();
		void Close();

        void RenderImGuiUI(ModelRenderOptions& opts, Cme::Camera camera);

    public:
        App();
        ~App();

	private:
		Cme::Window* m_pWindow;

        // ��App�п��Ʊ༭��
        ModelRenderOptions m_OptsObj;

        std::shared_ptr<Cme::Camera> m_spCamera;
        std::shared_ptr<Cme::CameraControls> m_spCameraControls;

        // Skybox ��պ���App�� Ҳ����Ĭ�Ͼ�������պ�
        std::shared_ptr<Cme::SkyboxShader> m_spSkyboxShader;
        std::shared_ptr<Cme::Skybox> m_spSkybox;
        // Skybox

        Cme::ModelScene m_ModelSceneObj;

        // IBL
        std::shared_ptr<Cme::BrdfMap> m_spBrdfMap;

        std::shared_ptr<Cme::DirectionalLight> m_spDirectionalLight;
        // GBuffer 
        // �ӳ���Ⱦ���������׶�(pass) 
        // ��һ�����δ���׶�(GeometryPass)�� ����Ⱦ����һ�� ��ȡ����ĸ��ּ�����Ϣ ���洢�ڽ���GBuffer��������
        // �ڶ������մ���׶�(LightingPass) ������׶λ���Ⱦһ����Ļ��С�ķ��� ����G�����еļ������ݶ�ÿһ��Ƭ�μ��㳡������
        std::shared_ptr<Cme::GBuffer> m_spGBuffer;                                  // ������Ⱦ�׶� ��ʼ��һ��֡������� Ҳ����GBuffer
        std::shared_ptr<Cme::ScreenQuadMesh> m_spScreenQuad;                        // ��Ⱦһ��������Ļ���ı��Σ���ʹ�ô洢��G�������еļ�����Ϣ����ÿ��Ƭ�εĳ�������    
        std::shared_ptr<Cme::DeferredGeometryPassShader> m_spGeometryPassShader;    // Defer��Ⱦ�ļ��δ���׶�
        std::shared_ptr<Cme::ScreenShader> m_spLightingPassShader;                  // Defer��Ⱦ�Ĺ��մ���׶�-----pbr��Ⱦ(���պ�����)����Ҫ�õ����shader
        std::shared_ptr<Cme::ScreenShader> m_spGBufferVisualShader;                 // GBuffer���ӻ�����Ҫ��Shader

        // Opengl�̳������������в���������Ĭ��֡�������Ⱦ�����Ͻ��еġ�Ĭ�ϵ�֡���������㴴�����ڵ�ʱ�����ɺ����õģ�GLFW������������Щ
        // �����ΪʲôOpengl�̳�8.1��������û�о��Ϊ0��FBO,��ȴ���԰�,��ΪĬ�����ɵ�FBO�����0 
        // �ڱ������� û�в���Ĭ�ϵ�֡���� ���Ƕ���������һ��FBO Ҳ����m_spMainFb �������˼�� ���е��߼�����˵���
        std::shared_ptr<Cme::Framebuffer> m_spMainFb;
        // ר�����ں����FBO ��GBuffer��Blit����
        std::shared_ptr<Cme::Framebuffer> m_spFinalFb;

        // FXAA
        std::shared_ptr<Cme::FXAAShader> m_spFxaaShader;
        // FXAA

        // PostProcess
        std::shared_ptr<Cme::ScreenShader> m_spPostprocessShader;
        // PostProcess

        std::shared_ptr<Cme::LightControl> m_spLightControl;
	};
}

