#pragma once
#include "../framebuffer.h"
#include "../core/vertex_buffer_object.h"

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
        static const std::string SAMPLER_KEY; // Key to store skybox sampler with

        static const int POSITION_ATTRIBUTE_INDEX; // Vertex attribute index of vertex position (0)
        static const int TEXTURE_COORDINATE_ATTRIBUTE_INDEX; // Vertex attribute index of texture coordinate (1)
        static const int NORMAL_ATTRIBUTE_INDEX; // Vertex attribute index of vertex normal (2)

    public:
        // Creates an unbound skybox mesh.
        Skybox();
        Skybox(const std::string& baseDirectory, const std::string& imageExtension, bool withPositions = true, bool withTextureCoordinates = true, bool withNormals = true);
        ~Skybox();

        void Render(Shader& shader);


        void InitializeData();

        bool HasPositions() const;

        bool HasTextureCoordinates() const;

        bool HasNormals() const;

        int GetVertexByteSize() const;

        void SetVertexAttributesPointers(int numVertices);

    public:
        Texture m_Texture;

    protected:
        GLuint m_VAO = 0;
        VertexBufferObject m_VBO;

        bool m_bInitialized = false;
        bool m_hasPositions = false;
        bool m_hasTextureCoordinates = false; 
        bool m_hasNormals = false; 

    private:
        std::string m_sBaseDirectory;
        std::string m_sImageExtension; 
    };
}

