#pragma once
#include "shader.h"

#include <glm/glm.hpp>

namespace Cme
{
    // A builtin shader for skyboxes. Should be used at the end of a frame to render
    // the skybox behind all other geometry.
    // TODO: Move this to a skybox header.
    class SkyboxShader : public Shader
    {
    public:
        SkyboxShader();

        virtual void activate() override;
        virtual void deactivate() override;

        virtual void setMat4(const char* name, const glm::mat4& matrix) override;
    };

    class ScreenShader : public Shader 
    {
    public:
        ScreenShader();
        explicit ScreenShader(const ShaderSource& fragmentSource);
    };

    class ScreenLodShader : public ScreenShader
    {
    public:
        ScreenLodShader();
        void setMipLevel(int mipLevel)
        {
            setFloat("lod", static_cast<float>(mipLevel));
        }
    };

    // TODO: Move this to the shadows header.
    class ShadowMapShader : public Shader
    {
    public:
        ShadowMapShader();
    };

    // HDR---立方体贴图Shader
    class EquirectCubemapShader : public Shader
    {
    public:
        EquirectCubemapShader();
    };

    // IBL---辐照Shader
    class CubemapIrradianceShader : public Shader
    {
    public:
        CubemapIrradianceShader();

        float getHemisphereSampleDelta() const { return m_fHemisphereSampleDelta; }
        void setHemisphereSampleDelta(float delta);

    private:
        float m_fHemisphereSampleDelta = 0.025f;
    };

    // IBL---预卷积Shader
    class GGXPrefilterShader : public Shader
    {
    public:
        GGXPrefilterShader();

        unsigned int getNumSamples() const { return m_uiNumSamples; }
        void setNumSamples(unsigned int samples);

        void setRoughness(float roughness);

    private:
        unsigned int m_uiNumSamples = 1024;
    };

    // IBL---双向反射分布函数Shader
    class GGXBrdfIntegrationShader : public ScreenShader
    {
    public:
        GGXBrdfIntegrationShader();

        unsigned int getNumSamples() const { return m_uiNumSamples; }
        void setNumSamples(unsigned int samples);

    private:
        unsigned int m_uiNumSamples = 1024;
    };
}  


