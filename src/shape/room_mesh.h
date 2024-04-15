#pragma once
#include "mesh.h"

namespace Cme
{
    class RoomMesh : public Mesh
    {
    public:
        explicit RoomMesh(std::string texturePath = "");
        explicit RoomMesh(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);

    protected:
        void loadMeshAndTextures(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);
        void initializeVertexAttributes() override;
    };
}