#include "light.h"

namespace Cme
{
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

}  // namespace Cme
