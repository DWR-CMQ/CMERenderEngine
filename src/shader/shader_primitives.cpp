#include <glad/glad.h>
#include "shader_primitives.h"

#include <cstring>

namespace Cme 
{
    SkyboxShader::SkyboxShader()
        : Shader(ShaderPath("assets//shaders//builtin//skybox.vert"),
                 ShaderPath("assets//shaders//builtin//skybox.frag")) {}

    void SkyboxShader::activate()
    {
        // The shader always outputs a depth of 1.0 (the max depth) for skybox
        // fragments, so we have to switch the depth function to LEQUAL in order for
        // them to render at all.
        glDepthFunc(GL_LEQUAL);
        Shader::activate();
    }

    void SkyboxShader::deactivate()
    {
        Shader::deactivate();
        glDepthFunc(GL_LESS);
    }

    void SkyboxShader::setMat4(const char* name, const glm::mat4& matrix) 
    {
        // Special case the "view" transform.
        if (strcmp(name, "view") == 0)
        {
            // We drop the translation (by just converting to a mat3 and then back)
            // since the skybox needs to always follow the camera.
            Shader::setMat4(name, glm::mat4(glm::mat3(matrix)));
            return;
        }
        Shader::setMat4(name, matrix);
    }

    ScreenShader::ScreenShader()
        : Shader(ShaderPath("assets//shaders//builtin//screen_quad.vert"),
                 ShaderPath("assets//shaders//builtin//screen_quad.frag")) {}

    ScreenShader::ScreenShader(const ShaderSource& fragmentSource)
        : Shader(ShaderPath("assets//shaders//builtin//screen_quad.vert"),
                 fragmentSource) {}

    ScreenLodShader::ScreenLodShader()
        : ScreenShader(ShaderPath("assets//shaders//builtin//screen_quad_lod.frag")) {}

    ShadowMapShader::ShadowMapShader()
        : Shader(ShaderPath("assets//shaders//builtin//shadow_map.vert"),
                 ShaderPath("assets//shaders//builtin//shadow_map.frag")) {}


    CubemapIrradianceShader::CubemapIrradianceShader()
        : Shader(ShaderPath("assets//shaders//builtin//cubemap.vert"),
            ShaderPath("assets//shaders//builtin//irradiance_cubemap.frag"))
    {
        // Set defaults.
        setHemisphereSampleDelta(m_fHemisphereSampleDelta);
    }

    void CubemapIrradianceShader::setHemisphereSampleDelta(float delta)
    {
        m_fHemisphereSampleDelta = delta;
        setFloat("qrk_hemisphereSampleDelta", m_fHemisphereSampleDelta);
    }

    GGXPrefilterShader::GGXPrefilterShader()
        : Shader(ShaderPath("assets//shaders//builtin//cubemap.vert"),
            ShaderPath("assets//shaders//builtin//ggx_prefilter_cubemap.frag"))
    {
        setNumSamples(m_uiNumSamples);
    }

    void GGXPrefilterShader::setNumSamples(unsigned int samples)
    {
        m_uiNumSamples = samples;
        setUInt("qrk_numSamples", m_uiNumSamples);
    }

    void GGXPrefilterShader::setRoughness(float roughness)
    {
        setFloat("qrk_roughness", roughness);
    }

    GGXBrdfIntegrationShader::GGXBrdfIntegrationShader()
        : ScreenShader(ShaderPath("assets/shaders/builtin/ggx_brdf_integration.frag"))
    {
        setNumSamples(m_uiNumSamples);
    }

    void GGXBrdfIntegrationShader::setNumSamples(unsigned int samples)
    {
        m_uiNumSamples = samples;
        setUInt("qrk_numSamples", m_uiNumSamples);
    }

    EquirectCubemapShader::EquirectCubemapShader()
        : Shader(ShaderPath("assets//shaders//builtin//cubemap.vert"),
            ShaderPath("assets//shaders//builtin//equirect_cubemap.frag")) {}
}  
