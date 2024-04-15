#pragma once
#include "mesh.h"

namespace Cme
{
    class PlaneMesh : public Mesh
    {
    public:
        explicit PlaneMesh(std::string texturePath = "");
        explicit PlaneMesh(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);

    protected:
        void loadMeshAndTextures(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);
        void initializeVertexAttributes() override;
    };
}

