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
            TextureMap textureMap(Texture::load(texturePath.c_str()), TextureMapType::DIFFUSE);
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
        loadMeshData(planeVertices, sizeof(planeVertices) / planeVertexSizeBytes,
            planeVertexSizeBytes, /*indices=*/{}, textureMaps);
    }

    void PlaneMesh::initializeVertexAttributes()
    {
        // Positions.
        m_VertexArrayObj.addVertexAttrib(3, GL_FLOAT);
        // Normals.
        m_VertexArrayObj.addVertexAttrib(3, GL_FLOAT);
        // Tangents.
        m_VertexArrayObj.addVertexAttrib(3, GL_FLOAT);
        // Texture coordinates.
        m_VertexArrayObj.addVertexAttrib(2, GL_FLOAT);

        m_VertexArrayObj.finalizeVertexAttribs();
    }
}