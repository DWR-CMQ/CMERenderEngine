#pragma once

#include "../exceptions.h"
#include "../framebuffer.h"
#include "../random.h"
#include "../shader/shader.h"
#include "../texture.h"
#include "../texture_uniform_source.h"
#include "ssao_kernel.h"

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

    // SsaoBuffer的本质是FBO
    class SsaoBuffer : public Framebuffer, public TextureSource 
    {
    public:
        SsaoBuffer(int width, int height);
        explicit SsaoBuffer(ImageSize size) : SsaoBuffer(size.width, size.height) {}
        virtual ~SsaoBuffer() = default;

        Texture getSsaoTexture()
        { 
            return m_SsaoBufferAttachmentObj.Transform2Texture(); 
        }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        Attachment m_SsaoBufferAttachmentObj;
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
