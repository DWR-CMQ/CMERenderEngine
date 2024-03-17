#include "blur.h"

namespace Cme
{
    GaussianBlurShader::GaussianBlurShader() : ScreenShader(ShaderPath("assets/shaders/builtin/gaussian_blur.frag")) {}

}  