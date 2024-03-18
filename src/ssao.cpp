#include "ssao.h"
#include "common_helper.h"

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

    void SsaoBlurShader::configureWith(SsaoKernel& kernel, SsaoBuffer& buffer)
    {
        setInt("qrk_ssaoNoiseTextureSideLength", kernel.getNoiseTextureSideLength());

        // The blur shader only needs a single texture, so we just bind it directly.
        buffer.getSsaoTexture().BindToUnit(0);
        setInt("qrk_ssao", 0);
    }

    SsaoKernel::SsaoKernel(float radius, float bias, int kernelSize,
                           int noiseTextureSideLength)
        : m_fRadius(radius), m_fBias(bias)
    {
        regenerate(kernelSize, noiseTextureSideLength);
    }

    void SsaoKernel::regenerate(int kernelSize, int noiseTextureSideLength) 
    {
        m_vecKernel.resize(kernelSize);
        // 法向半球 获取一个拥有最大64样本值的采样核心
        for (int i = 0; i < kernelSize; ++i) 
        {

            // 在切线空间中以-1.0到1.0的范围变换x和y方向 并以0.0和1.0为范围变换z方向获取样本
            glm::vec3 sample(m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next());
            // 模大于1的则删除 但理论上不会有这种情况
            if (glm::length2(sample) >= 1.0f) 
            {
                --i;
                continue;
            }

            sample = glm::normalize(sample) * m_RandObj.next();
            float scale = static_cast<float>(i) / kernelSize;
            // 以上所有的sample都是平均分布在采样核心里 但是ssao需要将核心样本靠近原点分布 这样就可以将更多的注意力放在靠近真正片段的遮蔽上
            // 所以使用插值函数实现
            scale = CommonHelper::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            m_vecKernel[i] = sample;
        }

        // 随机核心转动 创建一个小的随机旋转向量纹理平铺在屏幕上
        std::vector<glm::vec3> vecNoiseData;
        int noiseDataSize = noiseTextureSideLength * noiseTextureSideLength;
        vecNoiseData.resize(noiseDataSize);
        for (int i = 0; i < noiseDataSize; ++i) 
        {
            // 保持和采样核心一样的生成方式
            glm::vec3 noise(m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next() * 2.0f - 1.0f, 0.0f);
            vecNoiseData[i] = noise;
        }

        // Generate the texture. We don't want texture filtering, and we must enable
        // repeat wrap mode so that it properly tiles over the screen
        // 创建一个包含随机旋转向量的4x4纹理 记得设定它的封装方法为GL_REPEAT 从而保证它合适地平铺在屏幕上
        TextureParams params;
        params.filtering = TextureFiltering::NEAREST;
        params.wrapMode = TextureWrapMode::REPEAT;

        m_NoiseTextureObj = Texture::createFromData(noiseTextureSideLength, noiseTextureSideLength, GL_RGB16F, vecNoiseData, params);
    }

    void SsaoKernel::updateUniforms(Shader& shader) 
    {
        shader.setFloat("qrk_ssaoSampleRadius", m_fRadius);
        shader.setFloat("qrk_ssaoSampleBias", m_fBias);
        shader.setInt("qrk_ssaoKernelSize", m_vecKernel.size());
        for (unsigned int i = 0; i < m_vecKernel.size(); ++i)
        {
            shader.setVec3("qrk_ssaoKernel[" + std::to_string(i) + "]", m_vecKernel[i]);
        }
    }

    unsigned int SsaoKernel::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_NoiseTextureObj.BindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ssaoNoise", nextTextureUnit);

        return nextTextureUnit + 1;
    }

    SsaoBuffer::SsaoBuffer(int width, int height) : Framebuffer(width, height) 
    {
        // Make sure we're clearing properly.
        setClearColor(glm::vec4(0.0f));
        // Create and attach the SSAO buffer. Don't need a depth buffer.
        m_SsaoBufferAttachmentObj = AttachTexture2FB(Cme::BufferType::GRAYSCALE);
    }

    unsigned int SsaoBuffer::bindTexture(unsigned int nextTextureUnit, Shader& shader) 
    {
        m_SsaoBufferAttachmentObj.Transform2Texture().BindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ssao", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}  // namespace Cme
