#ifndef QUARKGL_SSAO_H_
#define QUARKGL_SSAO_H_

#include "exceptions.h"
#include "framebuffer.h"
#include "random.h"
#include "shader.h"
#include "texture.h"
#include "texture_registry.h"

#include <glm/glm.hpp>
#include <vector>

namespace Cme
{

    class SsaoException : public QuarkException
    {
        using QuarkException::QuarkException;
    };

    // A shader that calculates SSAO based on a G-Buffer and SSAO kernel.
    class SsaoShader : public Shader 
    {
    public:
        SsaoShader();
    };

    // A sample kernel for use in screen space ambient occlusion. Uses a hemisphere
    // sampling method and a noise texture.
    class SsaoKernel : public UniformSource, public TextureSource 
    {
    public:
        SsaoKernel(float radius = 0.5, float bias = 0.025, int kernelSize = 64,
                    int noiseTextureSideLength = 4);
        int getSize() { return m_vecKernel.size(); }
        int getNoiseTextureSideLength() { return m_NoiseTextureObj.getWidth(); }
        float getRadius() { return m_fRadius; }
        void setRadius(float radius) { m_fRadius = radius; }
        float getBias() { return m_fBias; }
        void setBias(float bias) { m_fBias = bias; }

        // Binds kernel uniforms.
        void updateUniforms(Shader& shader) override;

        // Binds the noise texture.
        unsigned int bindTexture(unsigned int nextTextureUnit,
                                Shader& shader) override;

    private:
        // TODO: Expose this after texture lifecycle is handled (currently
        // regenerating would cause orphaned textures).
        void regenerate(int kernelSize, int noiseTextureSideLength);

        float m_fRadius;
        float m_fBias;
        std::vector<glm::vec3> m_vecKernel;
        Texture m_NoiseTextureObj;
        UniformRandom m_RandObj;
    };

    class SsaoBuffer : public Framebuffer, public TextureSource 
    {
    public:
        SsaoBuffer(int width, int height);
        explicit SsaoBuffer(ImageSize size) : SsaoBuffer(size.width, size.height) {}
        virtual ~SsaoBuffer() = default;

        Texture getSsaoTexture() { return m_SsaoBufferObj.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit,
                                Shader& shader) override;

    private:
        Attachment m_SsaoBufferObj;
    };

    // A simple shader that blurs a precomputed SSAO buffer.
    class SsaoBlurShader : public Shader
    {
    public:
        SsaoBlurShader();

        // Configures uniforms for the blur to run correctly for the given kernel,
        // using the given buffer as the source image. After this, you can draw onto
        // another separate SsaoBuffer to get the blurred result.
        void configureWith(SsaoKernel& kernel, SsaoBuffer& buffer);
    };

}  // namespace Cme

#endif