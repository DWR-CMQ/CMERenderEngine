#pragma once
#include "../cubemap.h"
#include "../framebuffer.h"
#include "../shape/screenquad_mesh.h"

namespace Cme
{
    // Calculates an integration map for the GGX BRDF, parameterized over roughness
    // and NdotV.
    class BrdfMap : public TextureSource
    {
    public:
        BrdfMap(int width, int height);
        explicit BrdfMap(ImageSize size) : BrdfMap(size.width, size.height) {}
        virtual ~BrdfMap() = default;

        unsigned int getNumSamples() const { return m_shaderInstance.getNumSamples(); }
        void setNumSamples(unsigned int samples) { m_shaderInstance.setNumSamples(samples); }

        // Draw onto the allocated BRDF integration texture. This is solely a function
        // of the BRDF and does not require any source data.
        void draw();

        Texture getBrdfIntegrationMap() { return m_IntegrationMapInstance.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        Framebuffer m_BufferInstance;
        Attachment m_IntegrationMapInstance;
        ScreenQuadMesh m_ScreenQuadInstance;
        GGXBrdfIntegrationShader m_shaderInstance;
    };
}


