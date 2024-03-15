#ifndef QUARKGL_BLUR_H_
#define QUARKGL_BLUR_H_

#include "framebuffer.h"
#include "mesh_primitives.h"
#include "shader.h"
#include "shader_primitives.h"
#include "texture_registry.h"

namespace Cme 
{
    class PingPongPass 
    {
    public:
        PingPongPass(int width, int height);
        explicit PingPongPass(ImageSize size)
            : PingPongPass(size.width, size.height) {}
        virtual ~PingPongPass() = default;

        // Draws using the given shader multiple times, using the configured source
        // first and then ping-ponging between internal framebuffers. Passes represent
        // a single ping-pong cycle (so N passes means N*2 draw calls). The callback
        // should configure or update any needed shader uniforms, etc. After drawing,
        // you can call getOutput() to get the resulting texture.
        void multipassDraw(Texture source, Shader& shader, int passes,
                            std::function<void()> callback,
                            TextureRegistry* textureRegistry = nullptr);

        Texture getOutput()
        {
            // The final result is always stored in the 2nd FBO.
            return m_Attachments[1].asTexture();
        }

    private:
        ScreenQuadMesh m_ScreenQuadInstance;
        // Ping-pong buffers and attachments.
        Framebuffer m_Buffers[2];
        Attachment m_Attachments[2];
    };

    // A shader that performs a single-pass gaussian blur.
    class GaussianBlurShader : public ScreenShader
    {
    public:
        GaussianBlurShader();

        // Whether to perform a horizontal blur. If false, performs a vertical blur.
        void setHorizontal(bool horizontal)
        {
            m_bHorizontal = horizontal;
            setBool("horizontal", m_bHorizontal);
        }
        bool getHorizontal() { return m_bHorizontal; }

    private:
        bool m_bHorizontal = false;
    };

}  // namespace Cme

#endif