#ifndef QUARKGL_BLOOM_H_
#define QUARKGL_BLOOM_H_

#include "framebuffer.h"
#include "shape/screenquad_mesh.h"
#include "shader/shader.h"
#include "shader/shader_primitives.h"
#include "texture_uniform_source.h"

namespace Cme 
{
    class BloomException : public QuarkException 
    {
        using QuarkException::QuarkException;
    };

    class BloomBuffer : public Framebuffer, public TextureSource
    {
    public:
        BloomBuffer(int width, int height);
        explicit BloomBuffer(ImageSize size) : BloomBuffer(size.width, size.height) {}
        virtual ~BloomBuffer() = default;

        Texture getBloomMipChainTexture()
        {
            return m_BloomMipChainTextureInstance.Transform2Texture();
        }

        int getNumMips() { return m_BloomMipChainTextureInstance.m_iNumMips; }

        // Limits sampling from any other mip other than the given mip. This is
        // important to avoid undefined behavior when drawing to a mip level while
        // sampling from another.
        void selectMip(int mipLevel);
        void deselectMip();

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        Attachment m_BloomMipChainTextureInstance;
    };

    class BloomDownsampleShader : public ScreenShader
    {
    public:
        BloomDownsampleShader();

        void configureWith(BloomBuffer& buffer);
    };

    class BloomUpsampleShader : public ScreenShader
    {
    public:
        BloomUpsampleShader();

        void configureWith(BloomBuffer& buffer);
        // Sets the radius, in UV coordinates, for the upscaling kernel.
        void setFilterRadius(float filterRadius) 
        {
            m_fFilterRadius = filterRadius;
            setFloat("qrk_filterRadius", m_fFilterRadius);
        }
        float getFilterRadius() { return m_fFilterRadius; }

    private:
        static constexpr float DEFAULT_FILTER_RADIUS = 0.005f;
        float m_fFilterRadius = DEFAULT_FILTER_RADIUS;
    };

    // A self contained bloom pass that uses a series of down/upsamples to perform
    // the bloom effect.
    class BloomPass : public TextureSource
    {
    public:
        BloomPass(int width, int height);
        explicit BloomPass(ImageSize size) : BloomPass(size.width, size.height) {}
        virtual ~BloomPass() = default;

        // Sets the radius, in UV coordinates, for the upscaling kernel.
        void setFilterRadius(float filterRadius) { m_UpsampleShaderInstance.setFilterRadius(filterRadius); }
        float getFilterRadius() { return m_UpsampleShaderInstance.getFilterRadius(); }

        // Performs the bloom based on the image from a source framebuffer. The
        // currently configured color component is blitted into the bloom buffer,
        // after which the bloom downsample/upsample calls are made.
        void multipassDraw(Framebuffer& sourceFb);

        int getNumMips() { return m_BloomBufferInstance.getNumMips(); }
        void selectMip(int mipLevel) { m_BloomBufferInstance.selectMip(mipLevel); }
        void deselectMip() { m_BloomBufferInstance.deselectMip(); }

        Texture getOutput() { return m_BloomBufferInstance.getBloomMipChainTexture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        ScreenQuadMesh m_ScreenQuadInstance;
        BloomBuffer m_BloomBufferInstance;
        // 泛光的上下采样可参考 https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
        BloomDownsampleShader m_DownsampleShaderInstance;
        BloomUpsampleShader m_UpsampleShaderInstance;
    };

}  // namespace Cme

#endif