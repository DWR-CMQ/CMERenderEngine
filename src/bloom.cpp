#include "bloom.h"

namespace Cme
{
    BloomBuffer::BloomBuffer(int width, int height) : Framebuffer(width, height) 
    {
        // Create and attach the bloom buffer. Don't need a depth buffer.
        Cme::TextureParams resampleParams;

        resampleParams.filtering = Cme::TextureFiltering::BILINEAR;
        resampleParams.wrapMode = Cme::TextureWrapMode::CLAMP_TO_EDGE;
        resampleParams.generateMips = Cme::MipGeneration::ALWAYS;

        m_BloomMipChainTextureInstance = attachTexture(Cme::BufferType::COLOR_HDR_ALPHA, resampleParams);
    }

    void BloomBuffer::selectMip(int mipLevel)
    {
        if (mipLevel < 0 || mipLevel >= m_BloomMipChainTextureInstance.m_iNumMips)
        {
            throw BloomException("ERROR::BLOOM::SOURCE_MIP_OUT_OF_RANGE");
        }
        m_BloomMipChainTextureInstance.asTexture().setSamplerMipRange(mipLevel, mipLevel);
    }

    void BloomBuffer::deselectMip()
    {
        m_BloomMipChainTextureInstance.asTexture().unsetSamplerMipRange();
    }

    unsigned int BloomBuffer::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_BloomMipChainTextureInstance.asTexture().bindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_bloomMipChain", nextTextureUnit);

        return nextTextureUnit + 1;
    }

    BloomDownsampleShader::BloomDownsampleShader(): ScreenShader(ShaderPath("assets/shaders/builtin/bloom_downsample.frag")) {}

    void BloomDownsampleShader::configureWith(BloomBuffer& buffer) 
    {
        buffer.getBloomMipChainTexture().bindToUnit(0);
        setInt("qrk_bloomMipChain", 0);
    }

    BloomUpsampleShader::BloomUpsampleShader() : ScreenShader(ShaderPath("assets/shaders/builtin/bloom_upsample.frag"))
    {
        setFilterRadius(m_fFilterRadius);
    }

    void BloomUpsampleShader::configureWith(BloomBuffer& buffer)
    {
        // The bloom shader only needs a single texture, so we just bind it
        // directly.
        buffer.getBloomMipChainTexture().bindToUnit(0);
        setInt("qrk_bloomMipChain", 0);
    }

    BloomPass::BloomPass(int width, int height) : m_BloomBufferInstance(width, height) {}

    void BloomPass::multipassDraw(Framebuffer& sourceFb) 
    {
        // Copy to mip level 0.
        m_BloomBufferInstance.activate(0);
        sourceFb.blit(m_BloomBufferInstance, GL_COLOR_BUFFER_BIT);

        int numMips = m_BloomBufferInstance.getNumMips();

        // Perform the downsampling across the mip chain.
        m_DownsampleShaderInstance.configureWith(m_BloomBufferInstance);
        for (int destMip = 1; destMip < numMips; ++destMip) 
        {
            m_BloomBufferInstance.activate(destMip);
            int sourceMip = destMip - 1;
            m_BloomBufferInstance.selectMip(sourceMip);
            m_ScreenQuadInstance.draw(m_DownsampleShaderInstance);
        }

        // Perform the upsampling, starting with the second-to-last mip. We enable
        // additive blending to avoid having to render into a separate texture.
        m_BloomBufferInstance.enableAdditiveBlending();
        m_UpsampleShaderInstance.configureWith(m_BloomBufferInstance);
        for (int destMip = numMips - 2; destMip >= 0; --destMip) 
        {
            m_BloomBufferInstance.activate(destMip);
            int sourceMip = destMip + 1;
            m_BloomBufferInstance.selectMip(sourceMip);
            m_ScreenQuadInstance.draw(m_UpsampleShaderInstance);
        }

        m_BloomBufferInstance.deselectMip();
        m_BloomBufferInstance.disableAdditiveBlending();
        m_BloomBufferInstance.deactivate();
    }

    unsigned int BloomPass::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        getOutput().bindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_bloom", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}  // namespace Cme