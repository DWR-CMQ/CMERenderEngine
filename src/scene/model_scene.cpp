#include "model_scene.h"

namespace Cme
{
	ModelScene::ModelScene()
	{

	}

	void ModelScene::Init(std::shared_ptr<Cme::Camera> spCamera, ImageSize windowSize)
	{
        m_Size = windowSize;

        const char* lampShaderSource = R"SHADER(
        #version 460 core
        out vec4 fragColor;

        void main() { fragColor = vec4(1.0); }
        )SHADER";

        const char* normalShaderSource = R"SHADER(
        #version 460 core
        out vec4 fragColor;

        void main() { fragColor = vec4(1.0, 1.0, 0.0, 1.0); }
        )SHADER";

        // Prepare some debug shaders.
        m_spNormalShader = std::make_shared<Cme::Shader>(Cme::ShaderPath("assets//model_shaders//model.vert"),
            Cme::ShaderInline(normalShaderSource),
            Cme::ShaderPath("assets//model_shaders//model_normals.geom"));
        m_spNormalShader->addUniformSource(spCamera);

        m_spLampShader = std::make_shared<Cme::Shader>(Cme::ShaderPath("assets//model_shaders//model.vert"), Cme::ShaderInline(lampShaderSource));
        m_spLampShader->addUniformSource(spCamera);

        // Load primary model.
        m_upModel = LoadModelOrDefault();
	}

	/// @brief 由App中传递编辑器参数选项
	/// @param spCamera 
	/// @param stModelRenderOptions 
	void ModelScene::Render(ModelRenderOptions stModelRenderOptions,
                            std::shared_ptr<Cme::DeferredGeometryPassShader> spGeometryPassShader,
                            bool bShowNormal)
	{
        // Post-process options. Some option values are used later during rendering.
        m_upModel->setModelTransform(glm::scale(glm::mat4_cast(stModelRenderOptions.modelRotation), glm::vec3(stModelRenderOptions.modelScale)));

        // 正常绘制
        if (!bShowNormal)
        {
            m_upModel->draw(*spGeometryPassShader);
        }
        else
        {
            m_spNormalShader->updateUniforms();
            m_upModel->draw(*m_spNormalShader);
        }
	}

	void ModelScene::Update()
	{

	}

    std::unique_ptr<Cme::Model> ModelScene::LoadModelOrDefault()
    {
        // Default to the gltf DamagedHelmet.
        auto helmet = std::make_unique<Cme::Model>("assets//models//DamagedHelmet/DamagedHelmet.gltf");
        return helmet;
    }
}
