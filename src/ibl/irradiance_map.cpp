#include "irradiance_map.h"

namespace Cme
{
    IrradianceMap::IrradianceMap(int width, int height)
        : m_BufferInstance(width, height), m_CubemapRenderHelperInstance(&m_BufferInstance)
    {
        m_CubemapInstance = m_BufferInstance.AttachTexture2FB(BufferType::COLOR_CUBEMAP_HDR);
    }

    void IrradianceMap::multipassDraw(Texture source)
    {
        // Set up the source.
        source.BindToUnit(0, TextureBindType::CUBEMAP);
        m_IrradianceShaderInstance.setInt("qrk_environmentMap", 0);

        m_CubemapRenderHelperInstance.multipassDraw(m_IrradianceShaderInstance);
    }

    unsigned int IrradianceMap::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_CubemapInstance.Transform2Texture().BindToUnit(nextTextureUnit, TextureBindType::CUBEMAP);
        // Bind sampler uniforms.
        shader.setInt("qrk_irradianceMap", nextTextureUnit);

        return nextTextureUnit + 1;
    }
}