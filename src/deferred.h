#ifndef QUARKGL_DEFERRED_H_
#define QUARKGL_DEFERRED_H_

#include "exceptions.h"
#include "framebuffer.h"
#include "shader/shader.h"
#include "core/texture.h"

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

    class GBuffer : public Framebuffer 
    {
    public:
        GBuffer(int width, int height);
        explicit GBuffer(ImageSize size) : GBuffer(size.width, size.height) {}
        virtual ~GBuffer() = default;

        std::shared_ptr<Texture> getPositionAOTexture() { return GetTexture(1); }
        std::shared_ptr<Texture> getNormalRoughnessTexture() { return GetTexture(2); }
        std::shared_ptr<Texture> getAlbedoMetallicTexture() { return GetTexture(3); }
        std::shared_ptr<Texture> getEmissionTexture() { return GetTexture(4); }

        unsigned int bindTexture(unsigned int nextTextureUnit, Shader& shader);
    };

}  // namespace Cme

#endif
