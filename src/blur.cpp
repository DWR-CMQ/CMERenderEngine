#include "blur.h"

namespace Cme
{
    PingPongPass::PingPongPass(int width, int height)
        : m_Buffers{Framebuffer(width, height), Framebuffer(width, height)}
    {
        m_Attachments[0] = m_Buffers[0].attachTexture(BufferType::COLOR_HDR_ALPHA);
        m_Attachments[1] = m_Buffers[1].attachTexture(BufferType::COLOR_HDR_ALPHA);
    }

    void PingPongPass::multipassDraw(Texture source, Shader& shader, int passes,
                                     std::function<void()> callback,
                                     TextureRegistry* textureRegistry) 
    {
        // For the very first iteration, we render from the source to buffer0.
        Framebuffer* currentFb = &m_Buffers[0];
        Texture currentTexture = source;

        const int totalIterations = passes * 2;
        for (int i = 0; i < totalIterations; i++) 
        {
            currentFb->activate();

            callback();
            m_ScreenQuadInstance.setTexture(currentTexture);
            m_ScreenQuadInstance.draw(shader, textureRegistry);

            // For every subsequent iteration, we switch our target buffer, and then
            // render from the opposing attachment.
            const int nextFbIdx = (i + 1) % 2;
            currentFb = &m_Buffers[nextFbIdx];
            currentTexture = m_Attachments[nextFbIdx == 0 ? 1 : 0].asTexture();

            currentFb->deactivate();
        }
    }

    GaussianBlurShader::GaussianBlurShader() : ScreenShader(ShaderPath("assets/shaders/builtin/gaussian_blur.frag")) {}

}  