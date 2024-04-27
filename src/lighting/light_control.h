#pragma once

#include "../shader/shader.h"
#include "light.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>
namespace Cme
{
    class LightControl
    {
    public:
        LightControl(glm::mat4 mat4View);
        virtual ~LightControl() = default;
        void AddLight(std::shared_ptr<Light> light);
        void updateUniforms(Shader& shader);

        // Sets whether the registered lights should transform their positions to view
        // space. If false, positions remain in world space when passed to the shader.
        void setUseViewTransform(bool useViewTransform);

    private:
        unsigned int m_uiDirectionalCount = 0;
        unsigned int m_uiPointCount = 0;
        unsigned int m_uiSpotCount = 0;
        glm::mat4 m_mat4View = glm::mat4(1.0f);

        // std::shared_ptr<ViewSource> m_spViewSource;
        std::vector<std::shared_ptr<Light>> m_vecLights;
    };
}

