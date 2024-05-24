#pragma once

// 相关头文件
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

        // 由App中控制编辑器
        ModelRenderOptions m_OptsObj;

        std::shared_ptr<Cme::Camera> m_spCamera;
        std::shared_ptr<Cme::CameraControls> m_spCameraControls;

        // Skybox 天空盒在App中 也就是默认就是有天空盒
        std::shared_ptr<Cme::SkyboxShader> m_spSkyboxShader;
        std::shared_ptr<Cme::Skybox> m_spSkybox;
        // Skybox

        Cme::ModelScene m_ModelSceneObj;

        // IBL
        std::shared_ptr<Cme::BrdfMap> m_spBrdfMap;

        std::shared_ptr<Cme::DirectionalLight> m_spDirectionalLight;
        // GBuffer 
        // 延迟渲染包含两个阶段(pass) 
        // 第一个几何处理阶段(GeometryPass)中 先渲染场景一次 获取对象的各种几何信息 并存储在叫做GBuffer的纹理中
        // 第二个光照处理阶段(LightingPass) 在这个阶段会渲染一个屏幕大小的方形 并用G缓冲中的几何数据对每一个片段计算场景光照
        std::shared_ptr<Cme::GBuffer> m_spGBuffer;                                  // 几何渲染阶段 初始化一个帧缓冲对象 也就是GBuffer
        std::shared_ptr<Cme::ScreenQuadMesh> m_spScreenQuad;                        // 渲染一个充满屏幕的四边形，并使用存储在G缓冲区中的几何信息计算每个片段的场景光照    
        std::shared_ptr<Cme::DeferredGeometryPassShader> m_spGeometryPassShader;    // Defer渲染的几何处理阶段
        std::shared_ptr<Cme::ScreenShader> m_spLightingPassShader;                  // Defer渲染的光照处理阶段-----pbr渲染(光照和纹理)都需要用到这个shader
        std::shared_ptr<Cme::ScreenShader> m_spGBufferVisualShader;                 // GBuffer可视化所需要的Shader

        // Opengl教程上所做的所有操作都是在默认帧缓冲的渲染缓冲上进行的。默认的帧缓冲是在你创建窗口的时候生成和配置的（GLFW帮我们做了这些
        // 这就是为什么Opengl教程8.1上明明是没有句柄为0的FBO,但却可以绑定,因为默认生成的FBO句柄是0 
        // 在本工程中 没有采用默认的帧缓冲 而是独立生成了一个FBO 也就是m_spMainFb 基于这个思想 所有的逻辑就能说清楚
        std::shared_ptr<Cme::Framebuffer> m_spMainFb;

        // 猜测专门用于后处理的FBO(但不知道m_spFinalFb的作用到底是什么 因为删除后工程还是可以运行的) 从GBuffer中Blit而来 m_spFinalFb似乎没有专门的渲染缓冲对象
        // m_spFinalFb通过Blit能否把渲染缓冲对象分离出去
        // std::shared_ptr<Cme::Framebuffer> m_spFinalFb;

        // FXAA
        std::shared_ptr<Cme::FXAAShader> m_spFxaaShader;
        // FXAA

        // PostProcess
        std::shared_ptr<Cme::ScreenShader> m_spPostprocessShader;
        // PostProcess

        // 光照控制
        std::shared_ptr<Cme::LightControl> m_spLightControl;

        // 粒子系统
        WaterFountainParticleSystem* m_pWaterFountainPS;

        // 圆柱体
        std::shared_ptr<Cylinder> m_spCylinder;

        // 管道
        std::shared_ptr<Pipe> m_spPipeFirst;
        std::shared_ptr<Pipe> m_spPipeSecond;
	};
}

