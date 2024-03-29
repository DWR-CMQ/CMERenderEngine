#include "texture_uniform_source.h"

namespace Cme 
{
    void TextureUniformSource::updateUniforms(Shader& shader)
    {
        m_uiNextTextureUnit = 0;
        m_vecLastAvailableUnits.clear();

        for (auto source : m_vecTextureSources)
        {
            m_uiNextTextureUnit = source->bindTexture(m_uiNextTextureUnit, shader);
        }
    }

    void TextureUniformSource::pushUsageBlock()
    {
        m_vecLastAvailableUnits.push_back(m_uiNextTextureUnit);
    }

    void TextureUniformSource::popUsageBlock()
    {
        if (m_vecLastAvailableUnits.empty())
        {
            throw TextureUniformSourceException("ERROR::TEXTURE_REGISTRY::POP");
        }
        m_uiNextTextureUnit = m_vecLastAvailableUnits.back();
        m_vecLastAvailableUnits.pop_back();
    }

}  // namespace Cme
