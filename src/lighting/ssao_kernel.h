#pragma once
#include "../framebuffer.h"
#include "../shader/shader.h"
#include "../core/texture.h"
#include "../random.h"

namespace Cme
{
    // SsaoKernel本质是纹理
    // A sample kernel for use in screen space ambient occlusion. Uses a hemisphere
    // sampling method and a noise texture.
    class SsaoKernel
    {
    public:
        SsaoKernel(float radius = 0.5, float bias = 0.025, int kernelSize = 64, int noiseTextureSideLength = 4);
        int getSize() { return m_vecKernel.size(); }
        int getNoiseTextureSideLength() { return m_NoiseTextureObj.getWidth(); }
        float getRadius() { return m_fRadius; }
        void setRadius(float radius) { m_fRadius = radius; }
        float getBias() { return m_fBias; }
        void setBias(float bias) { m_fBias = bias; }

        // Binds the noise texture.
        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader);

    private:
        // TODO: Expose this after texture lifecycle is handled (currently
        // regenerating would cause orphaned textures).
        void GenerateNoiseTexture(int kernelSize, int noiseTextureSideLength);

        float m_fRadius;
        float m_fBias;
        std::vector<glm::vec3> m_vecKernel;
        Texture m_NoiseTextureObj;       // 随机核心转动作为纹理数据
        UniformRandom m_RandObj;         // 随机浮点数 范围0.0-1.0
    };
}
