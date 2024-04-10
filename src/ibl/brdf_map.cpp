#include "brdf_map.h"

namespace Cme
{
    BrdfMap::BrdfMap(int width, int height) : m_BufferInstance(width, height)
    {
        // The BRDF integration map contains values from [0..1] so we can use an SNORM
        // for greater precision.
        // TODO: Use a 2-channel SNORM texture instead.
        m_IntegrationMapInstance = m_BufferInstance.AttachTexture2FB(BufferType::COLOR_SNORM);
        // BrdfMap本质就是一个Texture 新建好FBO
        unsigned int uiFBO;
        glGenFramebuffers(1, &uiFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, uiFBO);

    }

    void BrdfMap::draw()
    {
        m_BufferInstance.activate();
        m_ScreenQuadInstance.draw(m_shaderInstance);
        m_BufferInstance.deactivate();
    }

    unsigned int BrdfMap::bindTexture(unsigned int nextTextureUnit, Shader& shader)
    {
        m_IntegrationMapInstance.Transform2Texture().BindToUnit(nextTextureUnit);
        // Bind sampler uniforms.
        shader.setInt("qrk_ggxIntegrationMap", nextTextureUnit);

        return nextTextureUnit + 1;
    }
}