#ifndef QUARKGL_SHADOWS_H_
#define QUARKGL_SHADOWS_H_

#include "exceptions.h"
#include "framebuffer.h"
#include "light.h"
#include "shader.h"
#include "texture.h"
#include "texture_registry.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Cme 
{

    class ShadowException : public QuarkException 
    {
        using QuarkException::QuarkException;
    };

    class ShadowCamera : public UniformSource 
    {
    public:
        // TODO: Currently only renders the origin. Make this more dynamic, and have
        // it automatically determine a best-fit frustum based on the scene.
        ShadowCamera(std::shared_ptr<DirectionalLight> light,
                    float cuboidExtents = 10.0f, float near = 0.1f,
                    float far = 15.0f, float shadowCameraDistanceFromOrigin = 7.0f,
                    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f));
        virtual ~ShadowCamera() = default;

        float getCuboidExtents() const { return m_fCuboidExtents; }
        void setCuboidExtents(float cuboidExtents) { m_fCuboidExtents = cuboidExtents; }
        float getNearPlane() const { return m_fNear; }
        void setNearPlane(float near) { m_fNear = near; }
        float getFarPlane() const { return m_fFar; }
        void setFarPlane(float far) { m_fFar = far; }
        float getDistanceFromOrigin() const 
        {
            return m_fShadowCameraDistanceFromOrigin;
        }
        void setDistanceFromOrigin(float dist) 
        {
            m_fShadowCameraDistanceFromOrigin = dist;
        }

        glm::mat4 getViewTransform();
        glm::mat4 getProjectionTransform();

        void updateUniforms(Shader& shader) override;

    private:
        std::shared_ptr<DirectionalLight> m_spLight;
        // The extents of the ortho camera's rendering cuboid.
        float m_fCuboidExtents;
        // The near plane of the ortho cuboid.
        float m_fNear;
        // The far plane of the ortho cuboid.
        float m_fFar;
        // The fake distance from the origin that the shadow camera is positioned at.
        float m_fShadowCameraDistanceFromOrigin;
        glm::vec3 m_vec3WorldUp;
    };

    class ShadowMap : public Framebuffer, public TextureSource 
    {
    public:
        explicit ShadowMap(int width = 1024, int height = 1024);
        explicit ShadowMap(ImageSize size) : ShadowMap(size.width, size.height) {}
        virtual ~ShadowMap() = default;

        Texture getDepthTexture() { return m_DepthAttachmentObj.asTexture(); }
        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        Attachment m_DepthAttachmentObj;
    };

}  // namespace Cme

#endif
