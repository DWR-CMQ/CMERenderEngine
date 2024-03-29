#ifndef QUARKGL_CUBEMAP_H_
#define QUARKGL_CUBEMAP_H_

#include "framebuffer.h"
#include "shape/room_mesh.h"
#include "shader/shader.h"
#include "shader/shader_primitives.h"
#include "texture_uniform_source.h"

namespace Cme
{
    class CubemapException : public QuarkException 
    {
        using QuarkException::QuarkException;
    };

    // A helper for rendering to a cubemap texture in a framebuffer. The passed-in
    // framebuffer must outlive the life of this helper.
    class CubemapRenderHelper 
    {
    public:
        CubemapRenderHelper(Framebuffer* buffer) : m_pBuffer(buffer) {}

        void setTargetMip(int mip) { m_iTargetMip = mip; }

        // Draws with the given shader to each face of the cubemap. This results in 6
        // different draw calls. Shader should be prepared (i.e. necessary textures
        // should either be bound or be in the registry, uniforms should be set, etc).
        void multipassDraw(Shader& shader, TextureUniformSource* TextureUniformSource = nullptr);

    private:
        Framebuffer* m_pBuffer;
        RoomMesh m_RoomInstance;
        int m_iTargetMip = 0;
    };

    // Converts an equirect texture to a cubemap.
    class EquirectCubemap : public TextureSource 
    {
    public:
        EquirectCubemap(int width, int height, bool generateMips = false);
        explicit EquirectCubemap(ImageSize size, bool generateMips = false)
            : EquirectCubemap(size.width, size.height, generateMips) {}
        virtual ~EquirectCubemap() = default;

        // Draw onto the allocated cubemap from the given texture as the source.
        void multipassDraw(Texture source);

        Texture getCubemap() { return m_CubemapInstance.Transform2Texture(); }

        unsigned int bindTexture(unsigned int nextTextureUnit,
                                Shader& shader) override;

    private:
        Framebuffer m_BufferInstance;
        Attachment m_CubemapInstance;
        EquirectCubemapShader m_EquirectCubemapShaderInstance;
        CubemapRenderHelper m_CubemapRenderHelperInstance;
        bool m_bGenerateMips;
    };

}  // namespace Cme

#endif