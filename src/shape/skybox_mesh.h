#pragma once
#include "mesh.h"
#include "../framebuffer.h"

namespace Cme
{
    class SkyboxMesh : public Mesh
    {
    public:
        // Creates an unbound skybox mesh.
        SkyboxMesh();
        // Creates a new skybox mesh from a set of 6 textures for the faces. Textures
        // must be passed in order starting with GL_TEXTURE_CUBE_MAP_POSITIVE_X and
        // incrementing from there; namely, in the order right, left, top, bottom,
        // front, and back.
        explicit SkyboxMesh(std::vector<std::string> faces);
        explicit SkyboxMesh(Texture texture);

        // Sets a framebuffer attachment as the texture.
        void setTexture(Attachment attachment);
        // Sets the texture. This overrides previously set textures.
        void setTexture(Texture texture);

    protected:
        void loadMesh();
        void initializeVertexAttributes() override;
    };
}

