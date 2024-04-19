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

    // 重新给Mesh设置Texture之前 是需要把之前的纹理清除掉吗?
    // 如果是 则可以直接绑定
    // 而且从代码逻辑来看 m_vecTextureMaps的size = 1恒成立
    void ScreenQuadMesh::setTexture(std::shared_ptr<Texture> spTexture)
    {
        // TODO: This copies the texture info, meaning it won't see updates.
        m_vecTextureMaps.clear();
        auto temp = std::make_shared<TextureMap>(*(spTexture.get()), TextureMapType::DIFFUSE);
        m_vecTextureMaps.emplace_back(temp);
    }

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

    /// @brief 其实这段代码本质就是利用Shader来更新纹理 如果之前已经有代码用Shader更新了纹理 此步骤根本就不用执行
    /// @param shader 
    /// @param TextureUniformSource 
    void ScreenQuadMesh::bindTextures(Shader& shader)
    {
        if (m_vecTextureMaps.empty())
        {
            return;
        }

        // Bind textures, assuming a given uniform naming.
        // If a TextureUniformSource isn't provided, just start with texture unit 0.
        unsigned int textureUnit = 0;

        Texture& texture = m_vecTextureMaps[0]->getTexture();
        texture.BindToUnit(textureUnit, TextureBindType::TEXTURE_2D);

        // Set the sampler to the correct texture unit.
        shader.setInt("qrk_screenTexture", textureUnit);
    }
}
