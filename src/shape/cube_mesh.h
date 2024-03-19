#pragma once
#include "mesh.h"

namespace Cme
{
    class CubeMesh : public Mesh
    {
    public:
        explicit CubeMesh(std::string texturePath = "");
        explicit CubeMesh(const std::vector<TextureMap>& textureMaps);

    protected:
        void loadMeshAndTextures(const std::vector<TextureMap>& textureMaps);
        void initializeVertexAttributes() override;
    };

}
