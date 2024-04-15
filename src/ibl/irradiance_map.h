#pragma once
#include "../cubemap.h"
#include "../framebuffer.h"
#include "../shape/screenquad_mesh.h"

namespace Cme
{
    // Calculates the prefiltered env map based on the GGX microfacet model. The map
    // contains multiple mip level, which each mip level representing a different
    // material roughness (mip0 -> roughness 0).
    class IrradianceMap : public TextureSource
    {
    public:
        IrradianceMap(int width = 32, int height = 32);
        explicit IrradianceMap(ImageSize size)
            : IrradianceMap(size.width, size.height) {}
        virtual ~IrradianceMap() = default;

        float getHemisphereSampleDelta() const
        {
            return m_IrradianceShaderInstance.getHemisphereSampleDelta();
        }
        void setHemisphereSampleDelta(float delta)
        {
            m_IrradianceShaderInstance.setHemisphereSampleDelta(delta);
        }

        // Draw onto the allocated cubemap from the given cubemap as the source.
        void multipassDraw(std::shared_ptr<Texture> spSource);

        std::shared_ptr<Texture> getIrradianceMap() { return m_CubemapInstance.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        Framebuffer m_BufferInstance;
        Attachment m_CubemapInstance;
        CubemapIrradianceShader m_IrradianceShaderInstance;
        CubemapRenderHelper m_CubemapRenderHelperInstance;
    };
}

