#pragma once
#include "mesh.h"

namespace Cme
{
    class PlaneMesh : public Mesh
    {
    public:
        explicit PlaneMesh(std::string texturePath = "");
        explicit PlaneMesh(const std::vector<TextureMap>& textureMaps);

    protected:
        void loadMeshAndTextures(const std::vector<TextureMap>& textureMaps);
        void initializeVertexAttributes() override;
    };
}

