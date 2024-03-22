#include <glad/glad.h>
#include "model.h"

#include <assimp/Importer.hpp>
#include <iostream>

namespace Cme 
{
    namespace 
    {
        constexpr TextureMapType loaderSupportedTextureMapTypes[] = 
        {
            TextureMapType::DIFFUSE,   TextureMapType::SPECULAR,
            TextureMapType::ROUGHNESS, TextureMapType::METALLIC,
            TextureMapType::AO,        TextureMapType::EMISSION,
            TextureMapType::NORMAL,
        };

        glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& m) 
        {
          return glm::mat4(
              m.a1, m.b1, m.c1, m.d1,
              m.a2, m.b2, m.c2, m.d2,
              m.a3, m.b3, m.c3, m.d3,
              m.a4, m.b4, m.c4, m.d4);
        }
    }  

    ModelMesh::ModelMesh(const std::vector<ModelVertex>& vertices,
                         const std::vector<unsigned int>& indices,
                         const std::vector<TextureMap>& textureMaps,
                         unsigned int instanceCount)
    {
        m_vecVertices = vertices;
        LoadMeshData(&m_vecVertices[0], m_vecVertices.size(), sizeof(ModelVertex), indices, textureMaps, instanceCount);
    }

    void ModelMesh::initializeVertexAttributes() 
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

    Model::Model(const char* path, unsigned int instanceCount)
        : m_uiInstanceCount(instanceCount)
    {
        std::string pathString(path);
        size_t i = pathString.find_last_of("/");
        // This will either be the model's directory, or empty string if the model is
        // at project root.
        m_sDirectory = i != std::string::npos ? pathString.substr(0, i) : "";

        loadModel(pathString);
    }

    void Model::LoadNodeMatrixByVectorInModel(const std::vector<glm::mat4>& vecModelMat)
    {
        m_RootNodeObj.visitRenderables([&](Renderable* renderable)
        {
            // All renderables in a Model are ModelMeshes.
            ModelMesh* pMesh = static_cast<ModelMesh*>(renderable);
            pMesh->LoadNodeMatrixByVectorInMesh(vecModelMat);
        });
    }

    void Model::LoadNodeMatrixByPointerInModel(const glm::mat4* pModelMat, unsigned int size)
    {
        m_RootNodeObj.visitRenderables([&](Renderable* renderable)
        {
            // All renderables in a Model are ModelMeshes.
            ModelMesh* pMesh = static_cast<ModelMesh*>(renderable);
            pMesh->LoadNodeMatrixByPointerInMesh(pModelMat, size);
        });
    }

    void Model::drawWithTransform(const glm::mat4& transform, Shader& shader,
                                  TextureRegistry* textureRegistry) 
    {
        m_RootNodeObj.drawWithTransform(transform * getModelTransform(), shader,
                                    textureRegistry);
    }

    void Model::loadModel(std::string path) 
    {
        Assimp::Importer importer;
        // Scene is freed by the importer.
        //const aiScene* scene = importer.ReadFile(path, DEFAULT_LOAD_FLAGS);
        const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
  
        if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
        {
            std::cout << "importer.GetErrorString(): " << importer.GetErrorString() << std::endl;
            throw ModelLoaderException("ERROR::MODEL::" + std::string(importer.GetErrorString()));
        }

        ProcessNode(m_RootNodeObj, pScene->mRootNode, pScene);
    }

    // 从Scene中获取根节点后，就可以从根节点开始，不断递归遍历其下的子节点，最终做到处理完所有节点拥有的数据
    void Model::ProcessNode(RenderableNode& target, aiNode* node, const aiScene* scene)
    {
        // mTransformation是相对于上一级node的变换矩阵
        target.setModelTransform(aiMatrix4x4ToGlm(node->mTransformation));

        // 处理节点下的每一个网格
        // 如果在该节点下找到挂载的Mesh，就直接处理该Mesh数据并将其添加至meshes容器当中，需要注意的是aiNode仅仅保存Mesh对应的索引，所以需要真正获得aiMesh还需要到Scene中去查找
        for (unsigned int i = 0; i < node->mNumMeshes; i++) 
        {
            // TODO: This might be creating meshes multiple times when they are
            // referenced by multiple nodes.
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            target.addRenderable(ProcessMesh(mesh, scene));
        }

        // Recurse for children. Recursion stops when no children left.
        for (unsigned int i = 0; i < node->mNumChildren; i++) 
        {
            auto childTarget = std::make_unique<RenderableNode>();
            ProcessNode(*childTarget, node->mChildren[i], scene);
            target.addChildNode(std::move(childTarget));
        }
    }

    std::unique_ptr<ModelMesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<ModelVertex> vecModelVertices;
        std::vector<unsigned int> vecIndices;
        std::vector<TextureMap> vecTextureMaps;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
        {
            ModelVertex vertex;

            // Process vertex positions, normals, tangents, and texture coordinates.
            auto inputPos = mesh->mVertices[i];
            glm::vec3 position(inputPos.x, inputPos.y, inputPos.z);
            vertex.position = position;

            if (mesh->HasNormals()) 
            {
                auto inputNorm = mesh->mNormals[i];
                vertex.normal = glm::vec3(inputNorm.x, inputNorm.y, inputNorm.z);
            } 
            else 
            {
                vertex.normal = glm::vec3(0.0f);
            }

            if (mesh->HasTangentsAndBitangents()) 
            {
                auto inputTangent = mesh->mTangents[i];
                vertex.tangent = glm::vec3(inputTangent.x, inputTangent.y, inputTangent.z);
            } 
            else 
            {
                vertex.tangent = glm::vec3(0.0f);
            }

            // TODO: This is only using the first texture coord set.
            if (mesh->HasTextureCoords(0)) 
            {
                auto inputTexCoords = mesh->mTextureCoords[0][i];
                vertex.texCoords = glm::vec2(inputTexCoords.x, inputTexCoords.y);
            } 
            else
            {
                vertex.texCoords = glm::vec2(0.0f);
            }

            vecModelVertices.push_back(vertex);
        }

        // Process indices.
        // 需要注意的是，索引并非直接存储在aiMesh中，而是存储在aiMesh下的aiFace中，由于在导入模型时使用了aiProcess_Triangulate，
        // 这里的Face也就基本是三角形图元，拥有3个Indices，将它们读取出来添加进总的索引即可
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) 
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) 
            {
                vecIndices.push_back(face.mIndices[j]);
            }
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        for (auto type : loaderSupportedTextureMapTypes)
        {
            auto loadedMaps = LoadMaterialTextureMaps(material, type);
            vecTextureMaps.insert(vecTextureMaps.end(), loadedMaps.begin(), loadedMaps.end());
        }

        return std::make_unique<ModelMesh>(vecModelVertices, vecIndices, vecTextureMaps, m_uiInstanceCount);
    }

    std::vector<TextureMap> Model::LoadMaterialTextureMaps(aiMaterial* material,
                                                           TextureMapType type)
    {
        std::vector<aiTextureType> aiTypes = textureMapTypeToAiTextureTypes(type);
        std::vector<TextureMap> textureMaps;

        for (aiTextureType aiType : aiTypes) 
        {
            for (unsigned int i = 0; i < material->GetTextureCount(aiType); i++)
            {
                aiString texturePath;
                material->GetTexture(aiType, i, &texturePath);
                // TODO: Pull the texture loading bits into a separate class.
                // Assume that the texture path is relative to model directory.
                std::string fullPath = m_sDirectory + "/" + texturePath.C_Str();

                // Don't re-load a texture if it's already been loaded.
                auto item = m_unmapLoadedTextureMaps.find(fullPath);
                if (item != m_unmapLoadedTextureMaps.end())
                {
                    // Texture has already been loaded, but likely of a different map type
                    // (for example, it could be a combined roughness / metallic map). If
                    // so, mark it as a packed texture.
                    TextureMap textureMap(item->second.getTexture(), type);
                    if (type != item->second.getType()) 
                    {
                        textureMap.setPacked(true);
                        item->second.setPacked(true);
                    }
                    textureMaps.push_back(textureMap);
                    continue;
                }

                // Assume that diffuse and emissive textures are in sRGB.
                // TODO: Allow for a way to override this if necessary.
                bool isSRGB = type == TextureMapType::DIFFUSE || type == TextureMapType::EMISSION;

                Texture texture = Texture::load(fullPath.c_str(), isSRGB);
                TextureMap textureMap(texture, type);
                m_unmapLoadedTextureMaps.insert(std::make_pair(fullPath, textureMap));
                textureMaps.push_back(textureMap);
            }
        }
        return textureMaps;
    }

}  // namespace Cme
