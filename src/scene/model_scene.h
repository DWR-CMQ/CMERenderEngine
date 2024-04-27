#pragma once
#include "../framebuffer.h"
#include "../model.h"
#include "../deferred.h"
#include "../cme_defs.h"

namespace Cme
{
    class ModelScene
    {
    public:
        ModelScene();
        ~ModelScene() {};

        void Init(ImageSize windowSize);
        void Render(ModelRenderOptions stModelRenderOptions, std::shared_ptr<Cme::Camera> spCamera, std::shared_ptr<Cme::DeferredGeometryPassShader> spGeometryPassShader = nullptr, bool bShowNormal = false);
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

