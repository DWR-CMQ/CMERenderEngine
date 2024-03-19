#pragma once
#include "mesh.h"
#include "../framebuffer.h"

namespace Cme
{
    class ScreenQuadMesh : public Mesh
    {
    public:
        // Creates an unbound screen quad mesh.
        ScreenQuadMesh();
        // Creates a new screen quad mesh from a texture.
        explicit ScreenQuadMesh(Texture texture);

        // Sets a framebuffer attachment as the texture.
        void setTexture(Attachment attachment);
        // Sets the texture. This overrides previously set textures.
        void setTexture(Texture texture);
        // Unsets the texture.
        void unsetTexture();

    protected:
        void loadMesh();
        void initializeVertexAttributes() override;
        void bindTextures(Shader& shader, TextureRegistry* textureRegistry) override;
    };
}
