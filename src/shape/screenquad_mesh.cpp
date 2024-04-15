#include "screenquad_mesh.h"

namespace Cme
{
    constexpr float screenQuadVertices[] =
    {
        // positions   // texture coords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    // clang-format on

    ScreenQuadMesh::ScreenQuadMesh() { loadMesh(); }

    ScreenQuadMesh::ScreenQuadMesh(Texture texture)
    {
        loadMesh();
        setTexture(std::make_shared<Texture>(texture));
    }

    void ScreenQuadMesh::loadMesh()
    {
        constexpr unsigned int quadVertexSizeBytes = 4 * sizeof(float);
        LoadMeshData(screenQuadVertices, sizeof(screenQuadVertices) / quadVertexSizeBytes, quadVertexSizeBytes, {}, {});
    }

    //void ScreenQuadMesh::setTexture(Attachment attachment)
    //{
    //    setTexture(attachment.Transform2Texture());
    //}

    void ScreenQuadMesh::setTexture(std::shared_ptr<Texture> spTexture)
    {
        // TODO: This copies the texture info, meaning it won't see updates.
        m_vecTextureMaps.clear();
        auto temp = std::make_shared<TextureMap>(*(spTexture.get()), TextureMapType::DIFFUSE);
        m_vecTextureMaps.emplace_back(temp);
    }

    //void ScreenQuadMesh::setTexture(Texture texture)
    //{
    //    // TODO: This copies the texture info, meaning it won't see updates.
    //    m_vecTextureMaps.clear();
    //    m_vecTextureMaps.emplace_back(texture, TextureMapType::DIFFUSE);
    //}

    void ScreenQuadMesh::unsetTexture()
    {
        m_vecTextureMaps.clear();
    }

    void ScreenQuadMesh::initializeVertexAttributes()
    {
        // Screen positions.
        m_VertexArrayObj.AddVertexAttrib(2, GL_FLOAT);
        // Texture coordinates.
        m_VertexArrayObj.AddVertexAttrib(2, GL_FLOAT);
        m_VertexArrayObj.SetVertexAttribs();
    }

    void ScreenQuadMesh::bindTextures(Shader& shader,
        TextureUniformSource* TextureUniformSource)
    {
        if (m_vecTextureMaps.empty())
        {
            return;
        }

        // Bind textures, assuming a given uniform naming.
        // If a TextureUniformSource isn't provided, just start with texture unit 0.
        unsigned int textureUnit = 0;
        if (TextureUniformSource != nullptr)
        {
            TextureUniformSource->pushUsageBlock();
            textureUnit = TextureUniformSource->getNextTextureUnit();
        }

        Texture& texture = m_vecTextureMaps[0]->getTexture();
        texture.BindToUnit(textureUnit, TextureBindType::TEXTURE_2D);

        // Set the sampler to the correct texture unit.
        shader.setInt("qrk_screenTexture", textureUnit);
        if (TextureUniformSource != nullptr)
        {
            TextureUniformSource->popUsageBlock();
        }
    }
}
