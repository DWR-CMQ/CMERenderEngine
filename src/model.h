#pragma once

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "exceptions.h"
#include "shape/mesh.h"
#include "shader/shader.h"
#include "texture_map.h"
#include "texture_map.h"

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace Cme 
{
    class ModelLoaderException : public QuarkException
    {
        using QuarkException::QuarkException;
    };

    struct ModelVertex
    {
        glm::vec3 position;          // 顶点位置
        glm::vec3 normal;            // 法向量
        glm::vec3 tangent;           // 切线
        glm::vec2 texCoords;         // 纹理坐标
    };

    class ModelMesh : public Mesh
    {
    public:
        ModelMesh(const std::vector<ModelVertex>& vertices,
                const std::vector<unsigned int>& indices,
                const std::vector<TextureMap>& textureMaps,
                unsigned int instanceCount = 0);

        virtual ~ModelMesh() = default;

    private:
        void initializeVertexAttributes() override;
        std::vector<ModelVertex> m_vecVertices;
    };

    constexpr auto DEFAULT_LOAD_FLAGS =
        // Ensure that all non-triangular polygon are converted to triangles.
        aiProcess_Triangulate |
        // Generate normals if the model doesn't have them.
        aiProcess_GenSmoothNormals |
        // Calculates tangent space if the model doesn't have them.
        aiProcess_CalcTangentSpace |
        // Generate UV coords if they aren't present.
        aiProcess_GenUVCoords |
        // Reduce vertex duplication.
        aiProcess_JoinIdenticalVertices |
        // Sort the result by primitive type.
        aiProcess_SortByPType;

    class Model : public Renderable 
    {
    public:
        explicit Model(const char* path, unsigned int instanceCount = 0);
        virtual ~Model() = default;

        // 这两个函数是专门做动画的
        void LoadNodeMatrixByVectorInModel(const std::vector<glm::mat4>& vecModelMat);
        void LoadNodeMatrixByPointerInModel(const glm::mat4* pModelMat, unsigned int size);

        void drawWithTransform(const glm::mat4& transform, Shader& shader,
                                TextureRegistry* textureRegistry = nullptr) override;

    private:
        void loadModel(std::string path);
        void ProcessNode(RenderableNode& target, aiNode* node, const aiScene* scene);
        std::unique_ptr<ModelMesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<TextureMap> LoadMaterialTextureMaps(aiMaterial* material,
                                                        TextureMapType type);

        unsigned int m_uiInstanceCount;
        RenderableNode m_RootNodeObj;
        std::string m_sDirectory;
        std::unordered_map<std::string, TextureMap> m_unmapLoadedTextureMaps;
    };

}  // namespace Cme

