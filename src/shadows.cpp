#include "shadows.h"

namespace qrk
{

    ShadowCamera::ShadowCamera(std::shared_ptr<DirectionalLight> light,

                               float cuboidExtents, float near, float far,
                               float shadowCameraDistanceFromOrigin,
                               glm::vec3 worldUp)
        : light_(light),
          cuboidExtents_(cuboidExtents),
          near_(near),
          far_(far),
          shadowCameraDistanceFromOrigin_(shadowCameraDistanceFromOrigin),
          worldUp_(worldUp) {}

    glm::mat4 ShadowCamera::getViewTransform() 
    {
        return glm::lookAt(shadowCameraDistanceFromOrigin_ * -light_->getDirection(),
                            glm::vec3(0.0f), worldUp_);
    }

    glm::mat4 ShadowCamera::getProjectionTransform() 
    {
        // Directional lights cast orthographic shadows.
        return glm::ortho(-cuboidExtents_, cuboidExtents_, -cuboidExtents_,
                        cuboidExtents_, near_, far_);
    }

    void ShadowCamera::updateUniforms(Shader& shader) 
    {
        shader.setMat4("lightViewProjection",
                        getProjectionTransform() * getViewTransform());
    }

    ShadowMap::ShadowMap(int width, int height) : Framebuffer(width, height)
    {
        // Attach the depth texture used for the shadow map.
        // TODO: Support omnidirectional shadow maps.
        TextureParams params;
        params.filtering = TextureFiltering::NEAREST;
        params.wrapMode = TextureWrapMode::CLAMP_TO_BORDER;
        params.borderColor = glm::vec4(1.0f);
        depthAttachment_ = attachTexture(BufferType::DEPTH, params);
    }

    unsigned int ShadowMap::bindTexture(unsigned int nextTextureUnit,
                                        Shader& shader) 
    {
        depthAttachment_.asTexture().bindToUnit(nextTextureUnit);
        // TODO: Make this more generic.
        shader.setInt("shadowMap", nextTextureUnit);
        return nextTextureUnit + 1;
    }

}  // namespace qrk
