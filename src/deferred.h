#ifndef QUARKGL_DEFERRED_H_
#define QUARKGL_DEFERRED_H_

#include "exceptions.h"
#include "framebuffer.h"
#include "shader/shader.h"
#include "texture.h"
#include "texture_uniform_source.h"

#include <glm/glm.hpp>

namespace Cme
{
    class DeferredShadingException : public QuarkException
    {
        using QuarkException::QuarkException;
    };

    class DeferredGeometryPassShader : public Shader
    {
    public:
        DeferredGeometryPassShader();
    };

    class GBuffer : public Framebuffer, public TextureSource 
    {
    public:
        GBuffer(int width, int height);
        explicit GBuffer(ImageSize size) : GBuffer(size.width, size.height) {}
        virtual ~GBuffer() = default;

        Texture getPositionAOTexture() { return m_PositionAOBufferInstance.Transform2Texture(); }
        Texture getNormalRoughnessTexture() 
        {
            return m_NormalRoughnessBufferInstance.Transform2Texture();
        }
        Texture getAlbedoMetallicTexture()
        {
            return m_AlbedoMetallicBufferInstance.Transform2Texture();
        }
        Texture getEmissionTexture() { return m_EmissionBufferInstance.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader) override;

    private:
        // RGB used for position, alpha used for AO.
        Attachment m_PositionAOBufferInstance;
        // RGB used for normal, alpha used for roughness.
        Attachment m_NormalRoughnessBufferInstance;
        // RGB used for albedo, alpha used for metallic.
        Attachment m_AlbedoMetallicBufferInstance;
        // RGB used for emission color, alpha channel unused.
        Attachment m_EmissionBufferInstance;
    };

}  // namespace Cme

#endif
