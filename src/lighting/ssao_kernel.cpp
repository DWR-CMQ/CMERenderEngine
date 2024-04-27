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
        // ������� ��ȡһ��ӵ�����64����ֵ�Ĳ�������
        for (int i = 0; i < kernelSize; ++i)
        {
            // �����߿ռ�����-1.0��1.0�ķ�Χ�任x��y���� ����0.0��1.0Ϊ��Χ�任z�����ȡ����
            glm::vec3 sample(m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next());
            // ģ����1����ɾ�� �������ϲ������������
            if (glm::length2(sample) >= 1.0f)
            {
                --i;
                continue;
            }

            sample = glm::normalize(sample) * m_RandObj.next();
            float scale = static_cast<float>(i) / kernelSize;
            // �������е�sample����ƽ���ֲ��ڲ��������� ����ssao��Ҫ��������������ԭ��ֲ� �����Ϳ��Խ������ע�������ڿ�������Ƭ�ε��ڱ���
            // ����ʹ�ò�ֵ����ʵ��
            scale = CommonHelper::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            m_vecKernel[i] = sample;
        }

        // �������ת�� ����һ��С�������ת��������ƽ������Ļ��
        std::vector<glm::vec3> vecNoiseData;
        int noiseDataSize = noiseTextureSideLength * noiseTextureSideLength;
        vecNoiseData.resize(noiseDataSize);
        for (int i = 0; i < noiseDataSize; ++i)
        {
            // ���ֺͲ�������һ�������ɷ�ʽ
            glm::vec3 noise(m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next() * 2.0f - 1.0f, 0.0f);
            vecNoiseData[i] = noise;
        }

        // Generate the texture. We don't want texture filtering, and we must enable
        // repeat wrap mode so that it properly tiles over the screen
        // ����һ�����������ת������4x4���� �ǵ��趨���ķ�װ����ΪGL_REPEAT �Ӷ���֤�����ʵ�ƽ������Ļ��
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

