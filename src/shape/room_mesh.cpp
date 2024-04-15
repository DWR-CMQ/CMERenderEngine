#include "room_mesh.h"

namespace Cme
{
    constexpr float roomVertices[] =
    {
        // positions           // normals            // tangents           // texture coords

        // back
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

        // front
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

        // left
        -0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,

        // right
         0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,

         // bottom
         -0.5f, -0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

         // top
         -0.5f,  0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f
    };
    // clang-format on

    RoomMesh::RoomMesh(std::string texturePath)
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

    RoomMesh::RoomMesh(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps)
    {
        loadMeshAndTextures(vecTextureMaps);
    }

    void RoomMesh::loadMeshAndTextures(const std::vector<std::shared_ptr<TextureMap>>& vecTextureMaps)
    {
        constexpr unsigned int roomVertexSizeBytes = 11 * sizeof(float);
        LoadMeshData(roomVertices, sizeof(roomVertices) / roomVertexSizeBytes, roomVertexSizeBytes, {}, vecTextureMaps);
    }

    void RoomMesh::initializeVertexAttributes()
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
