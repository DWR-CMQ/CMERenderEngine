#include "prefilter_map.h"

namespace Cme
{
    PrefilterMap::PrefilterMap(
        int width, int height, int maxNumMips)
        : m_BufferInstance(width, height), m_CubemapRenderHelperInstance(&m_BufferInstance)
    {
        TextureParams textureParams;
        textureParams.filtering = TextureFiltering::TRILINEAR;
        textureParams.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        textureParams.generateMips = MipGeneration::ALWAYS;
        textureParams.maxNumMips = maxNumMips;

        m_CubemapInstance = m_BufferInstance.AttachTexture2FB_i(BufferType::COLOR_CUBEMAP_HDR, textureParams);
    }

    void PrefilterMap::multipassDraw(Texture source)
    {
        // Set up the source.
        source.BindToUnit(0, TextureBindType::CUBEMAP);
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

    void PrefilterMap::updateUniforms(Shader& shader)
    {
        shader.setFloat("qrk_ggxPrefilteredEnvMapMaxLOD", static_cast<float>(m_CubemapInstance.m_iNumMips - 1.0));
    }

    unsigned int PrefilterMap::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_CubemapInstance.Transform2Texture().BindToUnit(nextTextureUnit, TextureBindType::CUBEMAP);
        // Bind sampler uniforms.
        shader.setInt("qrk_ggxPrefilteredEnvMap", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}
