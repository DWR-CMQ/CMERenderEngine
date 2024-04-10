#pragma once
#include "../quarkgl.h"
#include "../framebuffer.h"

namespace Cme
{
    class ModelScene
    {
    public:
        ModelScene();
        ~ModelScene() {};

        void Init(std::shared_ptr<Cme::Camera> spCamera, ImageSize windowSize);
        void Render(ModelRenderOptions stModelRenderOptions, std::shared_ptr<Cme::DeferredGeometryPassShader> spGeometryPassShader = nullptr, bool bShowNormal = false);
        void Update();

        std::unique_ptr<Cme::Model> LoadModelOrDefault();

    private:
        ImageSize m_Size;

    private:
        // Model
        std::unique_ptr<Cme::Model> m_upModel;
        // Model

        // Normal And Lamp Shader
        std::shared_ptr<Cme::Shader> m_spNormalShader;
        std::shared_ptr<Cme::Shader> m_spLampShader;
    };
}

