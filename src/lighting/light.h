#pragma once

#include "../exceptions.h"
#include "../shader/shader.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

// ʹ�����
namespace Cme 
{
    class LightException : public QuarkException 
    {
        using QuarkException::QuarkException;
    };

    struct Attenuation 
    {
        float constant;
        float linear;
        float quadratic;
    };

    // TODO: Change this to [0, 0, 1].
    constexpr Attenuation DEFAULT_ATTENUATION = {1.0f, 0.09f, 0.032f};
    constexpr glm::vec3 DEFAULT_DIFFUSE = glm::vec3(0.5f, 0.5f, 0.5f);
    constexpr glm::vec3 DEFAULT_SPECULAR = glm::vec3(1.0f, 1.0f, 1.0f);

    constexpr float DEFAULT_INNER_ANGLE = glm::radians(10.5f);
    constexpr float DEFAULT_OUTER_ANGLE = glm::radians(19.5f);

    enum class LightType
    {
        DIRECTIONAL_LIGHT,
        POINT_LIGHT,
        SPOT_LIGHT,
    };

    class Light 
    {
    public:
        virtual ~Light() = default;
        virtual LightType getLightType() const = 0;

        void setUseViewTransform(bool useViewTransform)
        {
            m_bUseViewTransform = useViewTransform;
        }

        void setLightIdx(unsigned int lightIdx) 
        {
            m_uiLightIdx = lightIdx;
            m_sUniformName = getUniformName(lightIdx);
        }

        void checkState()
        {
            if (m_hasViewDependentChanged && !m_hasViewBeenApplied)
            {
                throw LightException(
                    "ERROR::LIGHT::VIEW_CHANGED\n"
                    "Light state changed without re-applying view transform.");
            }
        }

        void resetChangeDetection()
        {
            m_hasViewDependentChanged = false;
            m_hasLightChanged = false;
            m_hasViewBeenApplied = false;
        }

        virtual std::string getUniformName(unsigned int lightIdx) const = 0;
        virtual void updateUniforms(Shader& shader) = 0;
        virtual void applyViewTransform(const glm::mat4& view) = 0;

        unsigned int m_uiLightIdx;
        std::string m_sUniformName;

        // Whether the light's position uniforms should be in view space. If false,
        // the positions are instead in world space.
        bool m_bUseViewTransform = true;
        // Start as `true` so that initial uniform values get set.
        bool m_hasViewDependentChanged = true;
        bool m_hasLightChanged = true;

        bool m_hasViewBeenApplied = false;
    };

    class DirectionalLight : public Light
    {
    public:
        DirectionalLight(glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
                        glm::vec3 diffuse = DEFAULT_DIFFUSE,
                        // TODO: Change this to default to whatever the diffuse was.
                        glm::vec3 specular = DEFAULT_SPECULAR);

        LightType getLightType() const { return LightType::DIRECTIONAL_LIGHT; }

        glm::vec3 getDirection() const { return direction_; }
        void setDirection(glm::vec3 direction)
        {
            direction_ = direction;
            m_hasViewDependentChanged = true;
        }
        glm::vec3 getDiffuse() const { return diffuse_; }
        void setDiffuse(glm::vec3 diffuse)
        {
            diffuse_ = diffuse;
            m_hasLightChanged = true;
        }
        glm::vec3 getSpecular() const { return specular_; }
        void setSpecular(glm::vec3 specular) 
        {
            specular_ = specular;
            m_hasLightChanged = true;
        }

    protected:
        std::string getUniformName(unsigned int lightIdx) const 
        {
            return "qrk_directionalLights[" + std::to_string(lightIdx) + "]";
        }
        void updateUniforms(Shader& shader);
        void applyViewTransform(const glm::mat4& view);

        private:
        glm::vec3 direction_;
        glm::vec3 viewDirection_;

        glm::vec3 diffuse_;
        glm::vec3 specular_;
    };

    class PointLight : public Light
    {
    public:
        PointLight(glm::vec3 position = glm::vec3(0.0f),
                    glm::vec3 diffuse = DEFAULT_DIFFUSE,
                    glm::vec3 specular = DEFAULT_SPECULAR,
                    Attenuation attenuation = DEFAULT_ATTENUATION);

        LightType getLightType() const { return LightType::POINT_LIGHT; }

        glm::vec3 getPosition() const { return m_vec3Position; }
        void setPosition(glm::vec3 position)
        {
            m_vec3Position = position;
            m_hasViewDependentChanged = true;
        }
        glm::vec3 getDiffuse() const { return m_vec3Diffuse; }
        void setDiffuse(glm::vec3 diffuse)
        {
            m_vec3Diffuse = diffuse;
            m_hasLightChanged = true;
        }

        glm::vec3 getSpecular() const { return m_vec3Specular; }
        void setSpecular(glm::vec3 specular)
        {
            m_vec3Specular = specular;
            m_hasLightChanged = true;
        }
        Attenuation getAttenuation() const { return m_stAttenuation; }
        void setAttenuation(Attenuation attenuation) 
        {
            m_stAttenuation = attenuation;
            m_hasLightChanged = true;
        }

    protected:
        std::string getUniformName(unsigned int lightIdx) const 
        {
            return "qrk_pointLights[" + std::to_string(lightIdx) + "]";
        }
        void updateUniforms(Shader& shader);
        void applyViewTransform(const glm::mat4& view);

    private:
        glm::vec3 m_vec3Position;
        glm::vec3 m_vec3ViewPosition;

        glm::vec3 m_vec3Diffuse;
        glm::vec3 m_vec3Specular;

        Attenuation m_stAttenuation;
    };

    class SpotLight : public Light
    {
    public:
        SpotLight(glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
                float innerAngle = DEFAULT_INNER_ANGLE,
                float outerAngle = DEFAULT_OUTER_ANGLE,
                glm::vec3 diffuse = DEFAULT_DIFFUSE,
                glm::vec3 specular = DEFAULT_SPECULAR,
                Attenuation attenuation = DEFAULT_ATTENUATION);

        LightType getLightType() const { return LightType::SPOT_LIGHT; }

        glm::vec3 getPosition() const { return m_vec3Position; }
        void setPosition(glm::vec3 position)
        {
            m_vec3Position = position;
            m_hasViewDependentChanged = true;
        }
        glm::vec3 getDirection() const { return m_vec3Direction; }
        void setDirection(glm::vec3 direction)
        {
            m_vec3Direction = direction;
            m_hasViewDependentChanged = true;
        }
        glm::vec3 getDiffuse() const { return m_vec3Diffuse; }
        void setDiffuse(glm::vec3 diffuse) 
        {
            m_vec3Diffuse = diffuse;
            m_hasViewBeenApplied = true;
        }
        glm::vec3 getSpecular() const { return m_vec3Specular; }
        void setSpecular(glm::vec3 specular)
        {
            m_vec3Specular = specular;
            m_hasViewBeenApplied = true;
        }
        Attenuation getAttenuation() const { return m_stAttenuation; }
        void setAttenuation(Attenuation attenuation)
        {
            m_stAttenuation = attenuation;
            m_hasViewBeenApplied = true;
        }

    protected:
        std::string getUniformName(unsigned int lightIdx) const
        {
            return "qrk_spotLights[" + std::to_string(lightIdx) + "]";
        }
        void updateUniforms(Shader& shader);
        void applyViewTransform(const glm::mat4& view);

    private:
        glm::vec3 m_vec3Position;
        glm::vec3 m_vec3ViewPosition;

        glm::vec3 m_vec3Direction;
        glm::vec3 m_vec3ViewDirection;

        float m_fInnerAngle;
        float m_fOuterAngle;

        glm::vec3 m_vec3Diffuse;
        glm::vec3 m_vec3Specular;

        Attenuation m_stAttenuation;
    };

} 

