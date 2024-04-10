#pragma once
#include "../framebuffer.h"
#include "../core/vertex_buffer_object.h"
#include "../cme_defs.h"

#include "../cubemap.h"
#include "../ibl/irradiance_map.h"
#include "../ibl/prefilter_map.h"

namespace Cme
{
    const int CUBE_FRONT_FACE = 1; // Bitmask to render cube front face
    const int CUBE_BACK_FACE = 1 << 1; // Bitmask to render cube back face
    const int CUBE_LEFT_FACE = 1 << 2; // Bitmask to render cube left face
    const int CUBE_RIGHT_FACE = 1 << 3; // Bitmask to render cube right face
    const int CUBE_TOP_FACE = 1 << 4; // Bitmask to render cube top face
    const int CUBE_BOTTOM_FACE = 1 << 5; // Bitmask to render cube bottom face
    const int CUBE_ALLFACES = (1 << 6) - 1; // Bitmask to render cube bottom face

    //class SkyboxMesh : public Mesh
    class Skybox
    {
    public:
        enum class SkyboxType
        {
            HDR = 0,
            Normal
        };

    public:
        static const std::string SAMPLER_KEY; // Key to store skybox sampler with

        static const int POSITION_ATTRIBUTE_INDEX; // Vertex attribute index of vertex position (0)
        static const int TEXTURE_COORDINATE_ATTRIBUTE_INDEX; // Vertex attribute index of texture coordinate (1)
        static const int NORMAL_ATTRIBUTE_INDEX; // Vertex attribute index of vertex normal (2)

    public:
        // Creates an unbound skybox mesh.
        Skybox();
        Skybox(bool withPositions = true, bool withTextureCoordinates = true, bool withNormals = true);
        ~Skybox();

        void Render(Shader& shader, std::shared_ptr<Cme::Camera> spCamera);

        void InitializeData();

        void LoadSkyboxImage(SkyboxImage eSkyboxImage);

        bool HasPositions() const;

        bool HasTextureCoordinates() const;

        bool HasNormals() const;

        int GetVertexByteSize() const;

        void SetVertexAttributesPointers(int numVertices);

    public:
        Texture m_Texture;

    private:
        GLuint m_VAO = 0;
        VertexBufferObject m_VBO;

        bool m_bInitialized = false;
        bool m_hasPositions = false;
        bool m_hasTextureCoordinates = false; 
        bool m_hasNormals = false; 

        std::string m_sBaseDirectory;
        std::string m_sImageExtension; 

        SkyboxType m_SkyboxType = SkyboxType::Normal;

        // µÈ¾àÖù×´Í¶Ó°Í¼
        std::shared_ptr<Cme::EquirectCubemap> m_spEquirectCubeMap;                 // Á¢·½ÌåÌùÍ¼
        std::shared_ptr<Cme::PrefilterMap> m_spPrefilterMap;                       // Ô¤¾í»ýÌùÍ¼
        std::shared_ptr<Cme::IrradianceMap> m_spIrradianceMap;                     // ·øÕÕÌùÍ¼
        
    };
}

