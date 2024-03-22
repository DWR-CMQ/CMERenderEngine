#include "skybox_mesh.h"

namespace Cme
{
    constexpr float skyboxVertices[] =
    {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    SkyboxMesh::SkyboxMesh() 
    { 
        loadMesh(); 
    }

    SkyboxMesh::SkyboxMesh(std::vector<std::string> faces)
        : SkyboxMesh(Texture::loadCubemap(faces)) {}

    SkyboxMesh::SkyboxMesh(Texture texture)
    {
        loadMesh();
        setTexture(texture);
    }

    void SkyboxMesh::setTexture(Attachment attachment)
    {
        setTexture(attachment.Transform2Texture());
    }

    void SkyboxMesh::setTexture(Texture texture)
    {
        if (texture.getType() != TextureType::CUBEMAP)
        {
            throw QuarkException::QuarkException(
                "ERROR::MESH_PRIMITIVE::INVALID_TEXTURE_TYPE\n" +
                std::to_string(static_cast<int>(texture.getType())));
        }
        // TODO: This copies the texture info, meaning it won't see updates.
        m_vecTextureMaps.clear();
        m_vecTextureMaps.emplace_back(texture, TextureMapType::CUBEMAP);
    }

    void SkyboxMesh::loadMesh()
    {
        constexpr unsigned int skyboxVertexSizeBytes = 3 * sizeof(float);
        LoadMeshData(skyboxVertices, sizeof(skyboxVertices) / skyboxVertexSizeBytes,
            skyboxVertexSizeBytes, /*indices=*/{}, /*textureMaps=*/{});
    }

    void SkyboxMesh::initializeVertexAttributes()
    {
        // Skybox only has vertex positions.
        m_VertexArrayObj.AddVertexAttrib(3, GL_FLOAT);
        m_VertexArrayObj.SetVertexAttribs();
    }
}
