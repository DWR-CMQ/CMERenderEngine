#include "plane_mesh.h"
namespace Cme
{
    // clang-format off
    constexpr float planeVertices[] =
    {
        // positions           // normals            // tangents           // texture coords
        -0.5f,  0.0f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.0f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.0f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,

         0.5f,  0.0f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.0f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f,  0.0f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f
    };
    // clang-format on

    PlaneMesh::PlaneMesh(std::string texturePath)
    {
        std::vector<std::shared_ptr<TextureMap>> vecTextureMaps;
        if (!texturePath.empty())
        {
            Texture textureObj;
            textureObj.LoadTexture(texturePath.c_str());
            auto spTextureMap = std::make_shared<TextureMap>(textureObj, TextureMapType::DIFFUSE);
            vecTextureMaps.push_back(spTextureMap);
        }
        loadMeshAndTextures(vecTextureMaps);
    }

    PlaneMesh::PlaneMesh(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps)
    {
        loadMeshAndTextures(vecTextureMaps);
    }

    void PlaneMesh::loadMeshAndTextures(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps)
    {
        constexpr unsigned int planeVertexSizeBytes = 11 * sizeof(float);
        LoadMeshData(planeVertices, sizeof(planeVertices) / planeVertexSizeBytes, planeVertexSizeBytes, {}, vecTextureMaps);
    }

    void PlaneMesh::initializeVertexAttributes()
    {
        // Positions.
        m_VertexArrayObj.AddVertexAttrib(3, GL_FLOAT);
        // Normals.
        m_VertexArrayObj.AddVertexAttrib(3, GL_FLOAT);
        // Tangents.
        m_VertexArrayObj.AddVertexAttrib(3, GL_FLOAT);
        // Texture coordinates.
        m_VertexArrayObj.AddVertexAttrib(2, GL_FLOAT);

        m_VertexArrayObj.SetVertexAttribs();
    }
}