#pragma once
#include "../cubemap.h"
#include "../framebuffer.h"
#include "../shape/screenquad_mesh.h"

namespace Cme
{
    // Calculates the prefiltered env map based on the GGX microfacet model. The map
    // contains multiple mip level, which each mip level representing a different
    // material roughness (mip0 -> roughness 0).
	class PrefilterMap
	{
    public:
        PrefilterMap(int width, int height, int maxNumMips = -1);
        explicit PrefilterMap(ImageSize size, int maxNumMips = -1)
            : PrefilterMap(size.width, size.height, maxNumMips) {}
        virtual ~PrefilterMap() = default;

        unsigned int getNumSamples() const { return m_ShaderInstance.getNumSamples(); }
        void setNumSamples(unsigned int samples) { m_ShaderInstance.setNumSamples(samples); }

        // Draw onto the allocated prefiltering cubemap from the given cubemap as the
        // source. The cubemap should ideally have mip levels in order to avoid
        // hotspot artifacts.
        void multipassDraw(std::shared_ptr<Texture> spSource);

        std::shared_ptr<Texture> getPrefilteredEnvMap() { return m_CubemapInstance.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader & shader);

    private:
        Framebuffer m_BufferInstance;
        Attachment m_CubemapInstance;
        GGXPrefilterShader m_ShaderInstance;
        CubemapRenderHelper m_CubemapRenderHelperInstance;
	};
}


