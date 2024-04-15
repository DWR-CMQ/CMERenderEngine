#include "skybox.h"
#include <mutex>

namespace Cme
{
    const std::string Skybox::SAMPLER_KEY = "skybox";

    const int Skybox::POSITION_ATTRIBUTE_INDEX = 0;
    const int Skybox::TEXTURE_COORDINATE_ATTRIBUTE_INDEX = 1;
    const int Skybox::NORMAL_ATTRIBUTE_INDEX = 2;

    glm::vec3 vertices[36] =
    {
        // Front face
        glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),
        // Back face
        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f),
        // Left face
        glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
        // Right face
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
        // Top face
        glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
        // Bottom face
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
    };

    glm::vec2 skyTextureCoordinates[6] =
    {
        glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f)
    };

    glm::vec3 skyboxNormals[6] =
    {
        glm::vec3(0.0f, 0.0f, 1.0f), // Front face
        glm::vec3(0.0f, 0.0f, -1.0f), // Back face
        glm::vec3(-1.0f, 0.0f, 0.0f), // Left face
        glm::vec3(1.0f, 0.0f, 0.0f), // Right face
        glm::vec3(0.0f, 1.0f, 0.0f), // Top face
        glm::vec3(0.0f, -1.0f, 0.0f), // Bottom face
    };

    Skybox::Skybox(bool withPositions, bool withTextureCoordinates, bool withNormals)
    {
        //m_sBaseDirectory = baseDirectory;
        //m_sImageExtension = imageExtension;
        m_hasPositions = withPositions;
        m_hasTextureCoordinates = withTextureCoordinates;
        m_hasNormals = withNormals;

        InitializeData();
    }

    Skybox::Skybox() 
    { 
    }

    Skybox::~Skybox()
    {
        glDeleteBuffers(1, &m_VAO);
        //glDeleteBuffers(1, &skyboxVBO);
    }

    void Skybox::Render(Shader& shader, std::shared_ptr<Cme::Camera> spCamera)
    {
        shader.setInt("skybox", 0);
        shader.activate();
        shader.setMat4("view", spCamera->getViewTransform());
        shader.setMat4("projection", spCamera->getProjectionTransform());

        glBindVertexArray(m_VAO);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        shader.deactivate();
    }

    void Skybox::InitializeData()
    {
        if (m_bInitialized) 
        {
            return;
        }

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        const auto numVertices = 36;
        const auto vertexByteSize = GetVertexByteSize();
        m_VBO.CreateVBO(vertexByteSize * numVertices);
        m_VBO.BindVBO();

        if (HasPositions())
        {
            m_VBO.AddRawData(vertices, sizeof(glm::vec3) * numVertices);
        }

        if (HasTextureCoordinates())
        {
            for (auto i = 0; i < 6; i++)
            {
                m_VBO.AddRawData(skyTextureCoordinates, sizeof(glm::vec2) * 6);
            }
        }

        if (HasNormals())
        {
            for (auto i = 0; i < 6; i++)
            {
                m_VBO.AddRawData(&skyboxNormals[i], sizeof(glm::vec3), 6);
            }
        }

        m_VBO.UploadDataToGPU(GL_STATIC_DRAW);
        SetVertexAttributesPointers(numVertices);
        m_bInitialized = true;
    }

    void Skybox::LoadSkyboxImage(SkyboxImage eSkyboxImage)
    {
        std::string hdrPath;
        switch (eSkyboxImage)
        {
        case SkyboxImage::ALEXS_APT:
            hdrPath = "assets//models//ibl/AlexsApt.hdr";
            break;
        case SkyboxImage::FROZEN_WATERFALL:
            hdrPath = "assets//models//ibl//FrozenWaterfall.hdr";
            break;
        case SkyboxImage::KLOPPENHEIM:
            hdrPath = "assets//models//ibl//Kloppenheim.hdr";
            break;
        case SkyboxImage::MILKYWAY:
            hdrPath = "assets//models//ibl//Milkyway.hdr";
            break;
        case SkyboxImage::MON_VALLEY:
            hdrPath = "assets//models//ibl//MonValley.hdr";
            break;
        case SkyboxImage::UENO_SHRINE:
            hdrPath = "assets//models//ibl//UenoShrine.hdr";
            break;
        case SkyboxImage::WINTER_FOREST:
            hdrPath = "assets//models//ibl//WinterForest.hdr";
            break;
        case SkyboxImage::Six_Face:
            hdrPath = "assets//models//skybox//jajsundown1";
            break;
        }

        // IBL
        constexpr int CUBEMAP_SIZE = 1024;
        // 等距柱状投影图
        m_spEquirectCubeMap = std::make_shared<Cme::EquirectCubemap>(CUBEMAP_SIZE, CUBEMAP_SIZE, true);
        
        // 辐照贴图 Irradiance map averages radiance uniformly so it doesn't have a lot of high frequency details and can thus be small.
        m_spIrradianceMap = std::make_shared<Cme::IrradianceMap>(32, 32);

        // 预卷积贴图  Create prefiltered envmap for specular IBL. It doesn't have to be super large.
        m_spPrefilterMap = std::make_shared<Cme::PrefilterMap>(CUBEMAP_SIZE, CUBEMAP_SIZE);

        int iSkyboxImage = static_cast<int>(eSkyboxImage);
        if (iSkyboxImage <= 6)
        {
            //Cme::Texture hdr = Cme::Texture::LoadHDR(hdrPath.c_str());
            auto fuck = std::make_shared<Cme::Texture>();
            fuck->LoadHDR(hdrPath.c_str());
            m_spEquirectCubeMap->multipassDraw(fuck);

            // m_spIrradianceMap和m_spPrefilterMap好像没什么作用(因为没有在Rendder时用到 却照样可以渲染出天空盒)
            m_spTexture = m_spEquirectCubeMap->GetCubemap();
            m_spIrradianceMap->multipassDraw(m_spTexture);
            m_spPrefilterMap->multipassDraw(m_spTexture);
        }
        else
        {
            std::vector<std::string> vecFaces
            {
                "assets//models//skybox//jajsundown1//right.jpg",
                "assets//models//skybox//jajsundown1//left.jpg",
                "assets//models//skybox//jajsundown1//top.jpg",
                "assets//models//skybox//jajsundown1//bottom.jpg",
                "assets//models//skybox//jajsundown1//front.jpg",
                "assets//models//skybox//jajsundown1//back.jpg",
            };
            //m_Texture = Cme::Texture::loadCubemap(vecFaces);

            auto texturePtr = std::make_unique<Cme::Texture>();
            texturePtr->loadCubemap(vecFaces);
        }
    }

    bool Skybox::HasPositions() const
    {
        return m_hasPositions;
    }

    bool Skybox::HasTextureCoordinates() const
    {
        return m_hasTextureCoordinates;
    }

    bool Skybox::HasNormals() const
    {
        return m_hasNormals;
    }

    int Skybox::GetVertexByteSize() const
    {
        int result = 0;
        if (HasPositions()) 
        {
            // sizeof(glm::vec3) = 12
            result += sizeof(glm::vec3);
        }
        if (HasTextureCoordinates())
        {
            result += sizeof(glm::vec2);
        }
        if (HasNormals())
        {
            result += sizeof(glm::vec3);
        }

        return result;
    }

    void Skybox::SetVertexAttributesPointers(int numVertices)
    {
        uint64_t offset = 0;
        if (HasPositions())
        {
            glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
            glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<void*>(offset));

            offset += sizeof(glm::vec3) * numVertices;
        }

        if (HasTextureCoordinates())
        {
            glEnableVertexAttribArray(TEXTURE_COORDINATE_ATTRIBUTE_INDEX);
            glVertexAttribPointer(TEXTURE_COORDINATE_ATTRIBUTE_INDEX, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), reinterpret_cast<void*>(offset));

            offset += sizeof(glm::vec2) * numVertices;
        }

        if (HasNormals())
        {
            glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
            glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<void*>(offset));

            offset += sizeof(glm::vec3) * numVertices;
        }
    }
}
