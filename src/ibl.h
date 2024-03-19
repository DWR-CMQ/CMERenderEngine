#ifndef QUARKGL_IBL_H_
#define QUARKGL_IBL_H_

#include "cubemap.h"
#include "framebuffer.h"
#include "shape/screenquad_mesh.h"
#include "shader/shader.h"
#include "shader/shader_primitives.h"
#include "texture_registry.h"

namespace Cme
{

    class IblException : public QuarkException 
    {
        using QuarkException::QuarkException;
    };

    class CubemapIrradianceShader : public Shader
    {
    public:
        CubemapIrradianceShader();

        float getHemisphereSampleDelta() const { return m_fHemisphereSampleDelta; }
        void setHemisphereSampleDelta(float delta);

    private:
        float m_fHemisphereSampleDelta = 0.025f;
    };

    // Calculates a diffuse irradiance map based on a given HDR cubemap.
    // Note that since each pixel in the irradiance map is uniformly sampled from an
    // entire hemisphere, fine details are not preserved - thus the irradiance map
    // need not be very large (32x32 can suffice), and the texture's bilinear
    // filtering can fill in the blanks.
    class CubemapIrradianceCalculator : public TextureSource 
    {
    public:
        CubemapIrradianceCalculator(int width = 32, int height = 32);
        explicit CubemapIrradianceCalculator(ImageSize size)
            : CubemapIrradianceCalculator(size.width, size.height) {}
        virtual ~CubemapIrradianceCalculator() = default;

        float getHemisphereSampleDelta() const
        {
            return m_IrradianceShaderInstance.getHemisphereSampleDelta();
        }
        void setHemisphereSampleDelta(float delta)
        {
            m_IrradianceShaderInstance.setHemisphereSampleDelta(delta);
        }

        // Draw onto the allocated cubemap from the given cubemap as the source.
        void multipassDraw(Texture source);

        Texture getIrradianceMap() { return m_CubemapInstance.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        Framebuffer m_BufferInstance;
        Attachment m_CubemapInstance;
        CubemapIrradianceShader m_IrradianceShaderInstance;
        CubemapRenderHelper m_CubemapRenderHelperInstance;
    };

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

    // Calculates the prefiltered env map based on the GGX microfacet model. The map
    // contains multiple mip level, which each mip level representing a different
    // material roughness (mip0 -> roughness 0).
    class GGXPrefilteredEnvMapCalculator : public UniformSource,
                                           public TextureSource 
    {
    public:
        GGXPrefilteredEnvMapCalculator(int width, int height, int maxNumMips = -1);
        explicit GGXPrefilteredEnvMapCalculator(ImageSize size, int maxNumMips = -1)
            : GGXPrefilteredEnvMapCalculator(size.width, size.height, maxNumMips) {}
        virtual ~GGXPrefilteredEnvMapCalculator() = default;

        unsigned int getNumSamples() const { return m_ShaderInstance.getNumSamples(); }
        void setNumSamples(unsigned int samples) { m_ShaderInstance.setNumSamples(samples); }

        // Draw onto the allocated prefiltering cubemap from the given cubemap as the
        // source. The cubemap should ideally have mip levels in order to avoid
        // hotspot artifacts.
        void multipassDraw(Texture source);

        Texture getPrefilteredEnvMap() { return m_CubemapInstance.Transform2Texture(); }

        void updateUniforms(Shader& shader) override;
        unsigned int bindTexture(unsigned int nextTextureUnit,
                                Shader& shader) override;

    private:
        Framebuffer m_BufferInstance;
        Attachment m_CubemapInstance;
        GGXPrefilterShader m_ShaderInstance;
        CubemapRenderHelper m_CubemapRenderHelperInstance;
    };

    class GGXBrdfIntegrationShader : public ScreenShader
    {
    public:
        GGXBrdfIntegrationShader();

        unsigned int getNumSamples() const { return m_uiNumSamples; }
        void setNumSamples(unsigned int samples);

    private:
        unsigned int m_uiNumSamples = 1024;
    };

    // Calculates an integration map for the GGX BRDF, parameterized over roughness
    // and NdotV.
    class GGXBrdfIntegrationCalculator : public TextureSource
    {
    public:
        GGXBrdfIntegrationCalculator(int width, int height);
        explicit GGXBrdfIntegrationCalculator(ImageSize size)
            : GGXBrdfIntegrationCalculator(size.width, size.height) {}
        virtual ~GGXBrdfIntegrationCalculator() = default;

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

}  // namespace Cme

#endif