#include "ssao.h"
#include "../common_helper.h"
#include "ssao_kernel.h"

#include <glm/gtx/norm.hpp>
#include <random>

namespace Cme 
{
    SsaoShader::SsaoShader()
        : Shader(ShaderPath("assets//shaders//builtin//screen_quad.vert"),
                 ShaderPath("assets//shaders//builtin//ssao.frag")) {}

    SsaoBlurShader::SsaoBlurShader()
        : Shader(ShaderPath("assets/shaders/builtin/screen_quad.vert"),
            ShaderPath("assets/shaders/builtin/ssao_blur.frag")) {}

    SsaoBuffer::SsaoBuffer(int width, int height) : Framebuffer(width, height)
    {
        // Make sure we're clearing properly.
        setClearColor(glm::vec4(0.0f));
        // Create and attach the SSAO buffer. Don't need a depth buffer.
        m_SsaoBufferAttachmentObj = AttachTexture2FB(Cme::BufferType::GRAYSCALE);
    }

    void SsaoBlurShader::configureWith(SsaoKernel& kernel, SsaoBuffer& buffer)
    {
        setInt("qrk_ssaoNoiseTextureSideLength", kernel.getNoiseTextureSideLength());

        // The blur shader only needs a single texture, so we just bind it directly.
        buffer.getSsaoTexture()->BindToUnit(0);
        setInt("qrk_ssao", 0);
    }

    unsigned int SsaoBuffer::bindTexture(unsigned int nextTextureUnit, Shader& shader) 
    {
        m_SsaoBufferAttachmentObj.Transform2Texture()->BindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ssao", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}  // namespace Cme
