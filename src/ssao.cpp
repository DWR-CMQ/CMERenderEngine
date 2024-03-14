#include "ssao.h"
#include "utils.h"

#include <glm/gtx/norm.hpp>
#include <random>

namespace qrk 
{

    SsaoShader::SsaoShader()
        : Shader(ShaderPath("assets//shaders//builtin//screen_quad.vert"),
                 ShaderPath("assets//shaders//builtin//ssao.frag")) {}

    SsaoKernel::SsaoKernel(float radius, float bias, int kernelSize,
                           int noiseTextureSideLength)
        : m_fRadius(radius), m_fBias(bias)
    {
        regenerate(kernelSize, noiseTextureSideLength);
    }

    void SsaoKernel::regenerate(int kernelSize, int noiseTextureSideLength) 
    {
        // Generate the kernel.
        m_vecKernel.resize(kernelSize);
        for (int i = 0; i < kernelSize; ++i) 
        {
            // Generate a hemisphere sample, with the normal vector pointing in the
            // positive Z direction.

            // First we generate a vector along the sample space.
            glm::vec3 sample(m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next());
            // Reject samples outside the sphere, to avoid over-sampling the corners.
            if (glm::length2(sample) >= 1.0f) 
            {
                --i;
                continue;
            }

            // Use the vector to generate a point in the hemisphere.
            sample = glm::normalize(sample) * m_RandObj.next();

            // At this point we have a random point sampled in the hemisphere, but we
            // want to sample more points closer to the actual fragment, so we scale the
            // result.
            float scale = static_cast<float>(i) / kernelSize;
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            m_vecKernel[i] = sample;
        }

        // Generate the noise texture used to rotate the kernel.
        std::vector<glm::vec3> noiseData;
        int noiseDataSize = noiseTextureSideLength * noiseTextureSideLength;
        noiseData.resize(noiseDataSize);
        for (int i = 0; i < noiseDataSize; ++i) 
        {
            // Generate a vector on the XY normal plane which we'll use to randomly
            // "tilt" the sample hemisphere in the shader.
            glm::vec3 noise(m_RandObj.next() * 2.0f - 1.0f, m_RandObj.next() * 2.0f - 1.0f,
                            0.0f);
            noiseData[i] = noise;
        }

        // Generate the texture. We don't want texture filtering, and we must enable
        // repeat wrap mode so that it properly tiles over the screen.
        TextureParams params;
        params.filtering = TextureFiltering::NEAREST;
        params.wrapMode = TextureWrapMode::REPEAT;

        m_NoiseTextureObj = Texture::createFromData(noiseTextureSideLength, noiseTextureSideLength, GL_RGB16F, noiseData, params);
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
        m_NoiseTextureObj.bindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ssaoNoise", nextTextureUnit);

        return nextTextureUnit + 1;
    }

    SsaoBuffer::SsaoBuffer(int width, int height) : Framebuffer(width, height) 
    {
        // Make sure we're clearing properly.
        setClearColor(glm::vec4(0.0f));
        // Create and attach the SSAO buffer. Don't need a depth buffer.
        m_SsaoBufferObj = attachTexture(qrk::BufferType::GRAYSCALE);
    }

    unsigned int SsaoBuffer::bindTexture(unsigned int nextTextureUnit, Shader& shader) 
    {
        m_SsaoBufferObj.asTexture().bindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ssao", nextTextureUnit);

        return nextTextureUnit + 1;
    }

    SsaoBlurShader::SsaoBlurShader()
        : Shader(ShaderPath("assets/shaders/builtin/screen_quad.vert"),
                 ShaderPath("assets/shaders/builtin/ssao_blur.frag")) {}

    void SsaoBlurShader::configureWith(SsaoKernel& kernel, SsaoBuffer& buffer)
    {
        setInt("qrk_ssaoNoiseTextureSideLength", kernel.getNoiseTextureSideLength());

        // The blur shader only needs a single texture, so we just bind it directly.
        buffer.getSsaoTexture().bindToUnit(0);
        setInt("qrk_ssao", 0);
    }

}  // namespace qrk
