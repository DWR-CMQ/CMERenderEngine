#include "texture_registry.h"

namespace qrk 
{
    void TextureRegistry::updateUniforms(Shader& shader) 
    {
        m_uiNextTextureUnit = 0;
        m_vecLastAvailableUnits.clear();

        for (auto source : m_vecTextureSources)
        {
            m_uiNextTextureUnit = source->bindTexture(m_uiNextTextureUnit, shader);
        }
    }

    void TextureRegistry::pushUsageBlock() 
    {
        m_vecLastAvailableUnits.push_back(m_uiNextTextureUnit);
    }

    void TextureRegistry::popUsageBlock() 
    {
        if (m_vecLastAvailableUnits.empty())
        {
            throw TextureRegistryException("ERROR::TEXTURE_REGISTRY::POP");
        }
        m_uiNextTextureUnit = m_vecLastAvailableUnits.back();
        m_vecLastAvailableUnits.pop_back();
    }

}  // namespace qrk
