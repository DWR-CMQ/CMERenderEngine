#include "ssao_kernel.h"
#include "../common_helper.h"

namespace Cme
{
    SsaoKernel::SsaoKernel(float radius, float bias, int kernelSize, int noiseTextureSideLength)
    {
        m_fRadius = radius;
        m_fBias = bias;
        GenerateNoiseTexture(kernelSize, noiseTextureSideLength);
    }

    void SsaoKernel::GenerateNoiseTexture(int kernelSize, int noiseTextureSideLength)
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

        //m_NoiseTextureObj = Texture::createFromData(noiseTextureSideLength, noiseTextureSideLength, GL_RGB16F, vecNoiseData, params);
    }

    unsigned int SsaoKernel::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_NoiseTextureObj.BindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ssaoNoise", nextTextureUnit);

        return nextTextureUnit + 1;
    }
}

