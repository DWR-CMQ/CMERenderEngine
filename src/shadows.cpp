#include "shadows.h"

namespace Cme
{

    ShadowMapCamera::ShadowMapCamera(std::shared_ptr<DirectionalLight> light,

                               float cuboidExtents, float near, float far,
                               float shadowCameraDistanceFromOrigin,
                               glm::vec3 worldUp)
        : m_spLight(light),
        m_fCuboidExtents(cuboidExtents),
        m_fNear(near),
        m_fFar(far),
        m_fShadowCameraDistanceFromOrigin(shadowCameraDistanceFromOrigin),
        m_vec3WorldUp(worldUp) {}

    glm::mat4 ShadowMapCamera::getViewTransform()
    {
        return glm::lookAt(m_fShadowCameraDistanceFromOrigin * -m_spLight->getDirection(),
                            glm::vec3(0.0f), m_vec3WorldUp);
    }

    glm::mat4 ShadowMapCamera::getProjectionTransform()
    {
        // Directional lights cast orthographic shadows.
        return glm::ortho(-m_fCuboidExtents, m_fCuboidExtents, -m_fCuboidExtents,
            m_fCuboidExtents, m_fNear, m_fFar);
    }

    void ShadowMapCamera::updateUniforms(Shader& shader)
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
        m_DepthAttachmentObj = attachTexture(BufferType::DEPTH, params);
    }

    unsigned int ShadowMap::bindTexture(unsigned int nextTextureUnit,
                                        Shader& shader) 
    {
        m_DepthAttachmentObj.Transform2Texture().bindToUnit(nextTextureUnit);
        // TODO: Make this more generic.
        shader.setInt("shadowMap", nextTextureUnit);
        return nextTextureUnit + 1;
    }

}  // namespace Cme
