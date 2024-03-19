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
        std::vector<TextureMap> textureMaps;
        if (!texturePath.empty())
        {
            TextureMap textureMap(Texture::load(texturePath.c_str()), TextureMapType::DIFFUSE);
            textureMaps.push_back(textureMap);
        }
        loadMeshAndTextures(textureMaps);
    }

    RoomMesh::RoomMesh(const std::vector<TextureMap>& textureMaps)
    {
        loadMeshAndTextures(textureMaps);
    }

    void RoomMesh::loadMeshAndTextures(const std::vector<TextureMap>& textureMaps)
    {
        constexpr unsigned int roomVertexSizeBytes = 11 * sizeof(float);
        loadMeshData(roomVertices, sizeof(roomVertices) / roomVertexSizeBytes,
            roomVertexSizeBytes, /*indices=*/{}, textureMaps);
    }

    void RoomMesh::initializeVertexAttributes()
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
