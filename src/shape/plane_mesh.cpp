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
        std::vector<TextureMap> textureMaps;
        if (!texturePath.empty())
        {
            TextureMap textureMap(Texture::LoadTexture(texturePath.c_str()), TextureMapType::DIFFUSE);
            textureMaps.push_back(textureMap);
        }
        loadMeshAndTextures(textureMaps);
    }

    PlaneMesh::PlaneMesh(const std::vector<TextureMap>& textureMaps)
    {
        loadMeshAndTextures(textureMaps);
    }

    void PlaneMesh::loadMeshAndTextures(const std::vector<TextureMap>& textureMaps)
    {
        constexpr unsigned int planeVertexSizeBytes = 11 * sizeof(float);
        LoadMeshData(planeVertices, sizeof(planeVertices) / planeVertexSizeBytes,
            planeVertexSizeBytes, /*indices=*/{}, textureMaps);
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