#include "deferred.h"

namespace Cme
{
    DeferredGeometryPassShader::DeferredGeometryPassShader()
        : Shader(ShaderPath("assets//shaders//builtin//deferred.vert"),
                 ShaderPath("assets//shaders//builtin//deferred.frag")) {}

    GBuffer::GBuffer(int width, int height) : Framebuffer(width, height) 
    {
        // Need to use a zero clear color, or else the G-Buffer won't work properly.
        setClearColor(glm::vec4(0.0f));
        // Create and attach all components of the G-Buffer.
        // Don't need to read from depth, so we attach as a renderbuffer.
        attachRenderbuffer(Cme::BufferType::DEPTH_AND_STENCIL);
        // Position and normal are stored as "HDR" colors for higher precision.
        // TODO: Positions can be un-projected from depth without storing them.
        m_PositionAOBufferInstance = AttachTexture2FB(Cme::BufferType::COLOR_HDR_ALPHA);
        m_NormalRoughnessBufferInstance = AttachTexture2FB(Cme::BufferType::COLOR_SNORM_ALPHA);
        m_AlbedoMetallicBufferInstance = AttachTexture2FB(Cme::BufferType::COLOR_ALPHA);
        m_EmissionBufferInstance = AttachTexture2FB(Cme::BufferType::COLOR_ALPHA);
    }

    unsigned int GBuffer::bindTexture(unsigned int nextTextureUnit, Shader& shader) 
    {
        m_PositionAOBufferInstance.Transform2Texture().BindToUnit(nextTextureUnit + 0);
        m_NormalRoughnessBufferInstance.Transform2Texture().BindToUnit(nextTextureUnit + 1);
        m_AlbedoMetallicBufferInstance.Transform2Texture().BindToUnit(nextTextureUnit + 2);
        m_EmissionBufferInstance.Transform2Texture().BindToUnit(nextTextureUnit + 3);
        // Bind sampler uniforms.
        shader.setInt("gPositionAO", nextTextureUnit + 0);
        shader.setInt("gNormalRoughness", nextTextureUnit + 1);
        shader.setInt("gAlbedoMetallic", nextTextureUnit + 2);
        shader.setInt("gEmission", nextTextureUnit + 3);

        return nextTextureUnit + 4;
    }

}  // namespace Cme
