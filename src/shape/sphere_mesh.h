#pragma once
#include "mesh.h"

namespace Cme
{
    class SphereMesh : public Mesh
    {
    public:
        static constexpr int DEFAULT_NUM_MERIDIANS = 64;
        static constexpr int DEFAULT_NUM_PARALLELS = 64;

        SphereMesh(std::string texturePath = "",
            int numMeridians = DEFAULT_NUM_MERIDIANS,
            int numParallels = DEFAULT_NUM_PARALLELS);

        SphereMesh(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps,
            int numMeridians = DEFAULT_NUM_MERIDIANS,
            int numParallels = DEFAULT_NUM_PARALLELS);

    protected:
        void loadMeshAndTextures(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps);
        void initializeVertexAttributes() override;

        int m_iNumMeridians;
        int m_iNumParallels;
    };
}
