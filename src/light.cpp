#include "light.h"

namespace qrk
{
    void LightRegistry::addLight(std::shared_ptr<Light> light) 
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

    void LightRegistry::updateUniforms(Shader& shader)
    {
        if (m_spViewSource != nullptr)
        {
            applyViewTransform(m_spViewSource->getViewTransform());
        }
        shader.setInt("qrk_directionalLightCount", m_uiDirectionalCount);
        shader.setInt("qrk_pointLightCount", m_uiPointCount);
        shader.setInt("qrk_spotLightCount", m_uiSpotCount);

        for (auto light : m_vecLights)
        {
            light->updateUniforms(shader);
        }
    }

    void LightRegistry::applyViewTransform(const glm::mat4& view)
    {
        // TODO: Only do this when we need to.
        for (auto light : m_vecLights)
        {
            light->applyViewTransform(view);
        }
    }

    void LightRegistry::setUseViewTransform(bool useViewTransform)
    {
        for (auto light : m_vecLights)
        {
            light->setUseViewTransform(useViewTransform);
        }
    }

    DirectionalLight::DirectionalLight(glm::vec3 direction, glm::vec3 diffuse,
                                       glm::vec3 specular)
        : direction_(glm::normalize(direction)),
          diffuse_(diffuse),
          specular_(specular) {}

    void DirectionalLight::updateUniforms(Shader& shader) 
    {
        checkState();

        if (m_hasViewBeenApplied) 
        {
            shader.setVec3(m_sUniformName + ".direction", m_bUseViewTransform ? viewDirection_ : direction_);
        }
        if (m_hasLightChanged) 
        {
            shader.setVec3(m_sUniformName + ".diffuse", diffuse_);
            shader.setVec3(m_sUniformName + ".specular", specular_);
        }

        // TODO: Fix change detection to work with >1 shaders.
        // resetChangeDetection();
    }

    void DirectionalLight::applyViewTransform(const glm::mat4& view) 
    {
        viewDirection_ = glm::vec3(view * glm::vec4(direction_, 0.0f));
        m_hasViewBeenApplied = true;
    }

    PointLight::PointLight(glm::vec3 position, glm::vec3 diffuse,
                           glm::vec3 specular, Attenuation attenuation)
        : m_vec3Position(position),
        m_vec3Diffuse(diffuse),
        m_vec3Specular(specular),
        m_stAttenuation(attenuation) {}

    void PointLight::updateUniforms(Shader& shader)
    {
        checkState();

        if (m_hasViewBeenApplied)
        {
            shader.setVec3(m_sUniformName + ".position", m_bUseViewTransform ? m_vec3ViewPosition : m_vec3Position);
        }
        if (m_hasLightChanged)
        {
            shader.setVec3(m_sUniformName + ".diffuse", m_vec3Diffuse);
            shader.setVec3(m_sUniformName + ".specular", m_vec3Specular);
            shader.setFloat(m_sUniformName + ".attenuation.constant",
                m_stAttenuation.constant);
            shader.setFloat(m_sUniformName + ".attenuation.linear", m_stAttenuation.linear);
            shader.setFloat(m_sUniformName + ".attenuation.quadratic",
                m_stAttenuation.quadratic);
        }

        // resetChangeDetection();
    }

    void PointLight::applyViewTransform(const glm::mat4& view)
    {
        m_vec3ViewPosition = glm::vec3(view * glm::vec4(m_vec3Position, 1.0f));
        m_hasViewBeenApplied = true;
    }

    SpotLight::SpotLight(glm::vec3 position, glm::vec3 direction, float innerAngle,
                         float outerAngle, glm::vec3 diffuse, glm::vec3 specular,
                         Attenuation attenuation)
        : m_vec3Position(position),
        m_vec3Direction(direction),
        m_fInnerAngle(innerAngle),
        m_fOuterAngle(outerAngle),
        m_vec3Diffuse(diffuse),
        m_vec3Specular(specular),
        m_stAttenuation(attenuation) {}

    void SpotLight::updateUniforms(Shader& shader)
    {
        checkState();

        if (m_hasViewBeenApplied)
        {
            shader.setVec3(m_sUniformName + ".position", m_bUseViewTransform ? m_vec3ViewPosition : m_vec3Position);
            shader.setVec3(m_sUniformName + ".direction", m_bUseViewTransform ? m_vec3ViewDirection : m_vec3Direction);
        }
        if (m_hasLightChanged)
        {
            shader.setFloat(m_sUniformName + ".innerAngle", m_fInnerAngle);
            shader.setFloat(m_sUniformName + ".outerAngle", m_fOuterAngle);
            shader.setVec3(m_sUniformName + ".diffuse", m_vec3Diffuse);
            shader.setVec3(m_sUniformName + ".specular", m_vec3Specular);
            shader.setFloat(m_sUniformName + ".attenuation.constant",
                m_stAttenuation.constant);
            shader.setFloat(m_sUniformName + ".attenuation.linear", m_stAttenuation.linear);
            shader.setFloat(m_sUniformName + ".attenuation.quadratic",
                m_stAttenuation.quadratic);
        }

        // resetChangeDetection();
    }

    void SpotLight::applyViewTransform(const glm::mat4& view) 
    {
        m_vec3ViewPosition = glm::vec3(view * glm::vec4(m_vec3Position, 1.0f));
        m_vec3ViewDirection = glm::vec3(view * glm::vec4(m_vec3Direction, 0.0f));
        m_hasViewBeenApplied = true;
    }

}  // namespace qrk
