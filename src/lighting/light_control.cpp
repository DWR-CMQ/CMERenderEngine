#include "light_control.h"

namespace Cme
{
    LightControl::LightControl(glm::mat4 mat4View)
    {
        m_mat4View = mat4View;
    }

    void LightControl::AddLight(std::shared_ptr<Light> light)
    {
        // TODO: Throw an error if this exceeds the max light count supported in the
        // shader.
        m_vecLights.push_back(light);

        switch (light->getLightType())
        {
        case LightType::DIRECTIONAL_LIGHT:
            light->setLightIdx(m_uiDirectionalCount);
            m_uiDirectionalCount++;
            break;
        case LightType::POINT_LIGHT:
            light->setLightIdx(m_uiPointCount);
            m_uiPointCount++;
            break;
        case LightType::SPOT_LIGHT:
            light->setLightIdx(m_uiPointCount);
            m_uiSpotCount++;
            break;
        }
    }

    void LightControl::updateUniforms(Shader& shader)
    {
        for (auto item : m_vecLights)
        {
            item->applyViewTransform(m_mat4View);
        }

        shader.setInt("qrk_directionalLightCount", m_uiDirectionalCount);
        shader.setInt("qrk_pointLightCount", m_uiPointCount);
        shader.setInt("qrk_spotLightCount", m_uiSpotCount);

        // 由于可以设置不同的光源 所以目前光源的更新暂时放到light_control中
        for (auto light : m_vecLights)
        {
            light->updateUniforms(shader);
        }
    }

    void LightControl::setUseViewTransform(bool useViewTransform)
    {
        for (auto light : m_vecLights)
        {
            light->setUseViewTransform(useViewTransform);
        }
    }
}
