#include "ibl.h"

namespace Cme 
{
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

    CubemapIrradianceCalculator::CubemapIrradianceCalculator(int width, int height)
        : m_BufferInstance(width, height), m_CubemapRenderHelperInstance(&m_BufferInstance)
    {
        m_CubemapInstance = m_BufferInstance.attachTexture(BufferType::COLOR_CUBEMAP_HDR);
    }

    void CubemapIrradianceCalculator::multipassDraw(Texture source)
    {
        // Set up the source.
        source.bindToUnit(0, TextureBindType::CUBEMAP);
        m_IrradianceShaderInstance.setInt("qrk_environmentMap", 0);

        m_CubemapRenderHelperInstance.multipassDraw(m_IrradianceShaderInstance);
    }

    unsigned int CubemapIrradianceCalculator::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_CubemapInstance.asTexture().bindToUnit(nextTextureUnit, TextureBindType::CUBEMAP);
        // Bind sampler uniforms.
        shader.setInt("qrk_irradianceMap", nextTextureUnit);

        return nextTextureUnit + 1;
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

    Cme::GGXPrefilteredEnvMapCalculator::GGXPrefilteredEnvMapCalculator(
        int width, int height, int maxNumMips)
        : m_BufferInstance(width, height), m_CubemapRenderHelperInstance(&m_BufferInstance)
    {
        TextureParams textureParams;
        textureParams.filtering = TextureFiltering::TRILINEAR;
        textureParams.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        textureParams.generateMips = MipGeneration::ALWAYS;
        textureParams.maxNumMips = maxNumMips;

        m_CubemapInstance = m_BufferInstance.attachTexture(BufferType::COLOR_CUBEMAP_HDR, textureParams);
    }

    void GGXPrefilteredEnvMapCalculator::multipassDraw(Texture source) 
    {
        // Set up the source.
        source.bindToUnit(0, TextureBindType::CUBEMAP);
        m_ShaderInstance.setInt("qrk_environmentMap", 0);

        for (int mip = 0; mip < m_CubemapInstance.m_iNumMips; ++mip)
        {
            m_CubemapRenderHelperInstance.setTargetMip(mip);
            // Go through roughness from [0..1].
            float roughness = static_cast<float>(mip) / (m_CubemapInstance.m_iNumMips - 1);
            m_ShaderInstance.setRoughness(roughness);
            m_CubemapRenderHelperInstance.multipassDraw(m_ShaderInstance);
        }
    }

    void GGXPrefilteredEnvMapCalculator::updateUniforms(Shader& shader) 
    {
      shader.setFloat("qrk_ggxPrefilteredEnvMapMaxLOD", static_cast<float>(m_CubemapInstance.m_iNumMips - 1.0));
    }

    unsigned int GGXPrefilteredEnvMapCalculator::bindTexture(unsigned int nextTextureUnit, Shader& shader) 
    {
        m_CubemapInstance.asTexture().bindToUnit(nextTextureUnit, TextureBindType::CUBEMAP);
        // Bind sampler uniforms.
        shader.setInt("qrk_ggxPrefilteredEnvMap", nextTextureUnit);

        return nextTextureUnit + 1;
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

    GGXBrdfIntegrationCalculator::GGXBrdfIntegrationCalculator(int width, int height) : m_BufferInstance(width, height)
    {
        // The BRDF integration map contains values from [0..1] so we can use an SNORM
        // for greater precision.
        // TODO: Use a 2-channel SNORM texture instead.
        m_IntegrationMapInstance = m_BufferInstance.attachTexture(BufferType::COLOR_SNORM);
    }

    void GGXBrdfIntegrationCalculator::draw() 
    {
        m_BufferInstance.activate();
        m_ScreenQuadInstance.draw(m_shaderInstance);
        m_BufferInstance.deactivate();
    }

    unsigned int GGXBrdfIntegrationCalculator::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_IntegrationMapInstance.asTexture().bindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ggxIntegrationMap", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}  // namespace Cme