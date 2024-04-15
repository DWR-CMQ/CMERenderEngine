#pragma once
#include "mesh.h"

namespace Cme
{
    class CubeMesh : public Mesh
    {
    public:
        explicit CubeMesh(std::string texturePath = "");
        explicit CubeMesh(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);

    protected:
        void loadMeshAndTextures(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);
        void initializeVertexAttributes() override;
    };

}
