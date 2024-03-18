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
