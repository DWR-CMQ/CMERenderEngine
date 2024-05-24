#pragma once

// ���ͷ�ļ�
#include "fxaa.h"
#include "blur.h"
#include "camera.h"
#include "cubemap.h"
#include "debug.h"
#include "deferred.h"
#include "exceptions.h"
#include "framebuffer.h"

#include "ibl/brdf_map.h"
#include "ibl/irradiance_map.h"
#include "ibl/prefilter_map.h"

#include "lighting/light.h"
#include "lighting/light_control.h"

#include "shape/mesh.h"
#include "shape/screenquad_mesh.h"
#include "shape/sphere_mesh.h"
#include "shape/cylinder.h"
#include "shape/pipe.h"

#include "model.h"
#include "random.h"
#include "screen.h"
#include "shader/shader.h"
#include "shader/shader_defs.h"
#include "shader/shader_loader.h"
#include "shader/shader_primitives.h"
#include "shadows.h"
#include "lighting/ssao.h"
#include "lighting/ssao_kernel.h"
#include "core/texture.h"
#include "texture_map.h"
#include "common_helper.h"
#include "vertex_array.h"
#include "window.h"
#include "cme_defs.h"
#include "core/texture_manager.h"
#include "UI/ui.h"

#include "particle/water_fountain_particle_system.h"

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

        // �²�ר�����ں����FBO(����֪��m_spFinalFb�����õ�����ʲô ��Ϊɾ���󹤳̻��ǿ������е�) ��GBuffer��Blit���� m_spFinalFb�ƺ�û��ר�ŵ���Ⱦ�������
        // m_spFinalFbͨ��Blit�ܷ����Ⱦ�����������ȥ
        // std::shared_ptr<Cme::Framebuffer> m_spFinalFb;

        // FXAA
        std::shared_ptr<Cme::FXAAShader> m_spFxaaShader;
        // FXAA

        // PostProcess
        std::shared_ptr<Cme::ScreenShader> m_spPostprocessShader;
        // PostProcess

        // ���տ���
        std::shared_ptr<Cme::LightControl> m_spLightControl;

        // ����ϵͳ
        WaterFountainParticleSystem* m_pWaterFountainPS;

        // Բ����
        std::shared_ptr<Cylinder> m_spCylinder;

        // �ܵ�
        std::shared_ptr<Pipe> m_spPipeFirst;
        std::shared_ptr<Pipe> m_spPipeSecond;
	};
}

