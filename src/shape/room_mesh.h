#pragma once
#include "mesh.h"

namespace Cme
{
    class RoomMesh : public Mesh
    {
    public:
        explicit RoomMesh(std::string texturePath = "");
        explicit RoomMesh(const std::vector<TextureMap>& textureMaps);

    protected:
        void loadMeshAndTextures(const std::vector<TextureMap>& textureMaps);
        void initializeVertexAttributes() override;
    };
}