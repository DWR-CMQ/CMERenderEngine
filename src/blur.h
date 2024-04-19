#ifndef QUARKGL_BLUR_H_
#define QUARKGL_BLUR_H_

#include "framebuffer.h"
#include "shader/shader.h"
#include "shader/shader_primitives.h"

namespace Cme 
{
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