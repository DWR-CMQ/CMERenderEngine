#include "mesh.h"

namespace Cme
{
    void RenderableNode::drawWithTransform(const glm::mat4& transform,
                                        Shader& shader,
                                        TextureUniformSource* TextureUniformSource) 
    {
        // Combined incoming transform with the node's.
        const glm::mat4 mat = transform * getModelTransform();
        for (auto& renderable : m_vecRenderables)
        {
            renderable->drawWithTransform(mat, shader, TextureUniformSource);
        }

        // Render children.
        for (auto& childNode : m_vecChildNodes)
        {
            childNode->drawWithTransform(mat, shader, TextureUniformSource);
        }
    }

    void RenderableNode::visitRenderables(std::function<void(Renderable*)> visitor)
    {
        for (auto& renderable : m_vecRenderables)
        {
            visitor(renderable.get());
        }
        for (auto& childNode : m_vecChildNodes)
        {
            childNode->visitRenderables(visitor);
        }
    }

    void Mesh::LoadMeshData(const void* vertexData, unsigned int numVertices,
                        unsigned int vertexSizeBytes,
                        const std::vector<unsigned int>& indices,
                        const std::vector<TextureMap>& textureMaps,
                        unsigned int instanceCount)
    {
        m_vecIndices = indices;
        m_vecTextureMaps = textureMaps;
        m_uiNumVertices = numVertices;
        m_uiVertexSizeBytes = vertexSizeBytes;
        m_uiInstanceCount = instanceCount;

        // Load VBO.
        m_VertexArrayObj.loadVertexData(vertexData, m_uiNumVertices * vertexSizeBytes);

        initializeVertexAttributes();
        initializeVertexArrayInstanceData();

        // Load EBO if this is an indexed mesh.
        if (!m_vecIndices.empty())
        {
            m_VertexArrayObj.loadElementData(&m_vecIndices[0], indices.size() * sizeof(unsigned int));
        }
    }

    void Mesh::LoadNodeMatrixByVectorInMesh(const std::vector<glm::mat4>& models)
    {
        m_VertexArrayObj.loadInstanceVertexData(&models[0], models.size() * sizeof(glm::mat4));
    }

    void Mesh::LoadNodeMatrixByPointerInMesh(const glm::mat4* models, unsigned int size)
    {
        m_VertexArrayObj.loadInstanceVertexData(&models[0], size * sizeof(glm::mat4));
    }

    void Mesh::drawWithTransform(const glm::mat4& transform, Shader& shader,
                                TextureUniformSource* TextureUniformSource)
    {
        // First we set the model transform, combining with the incoming transform.
        shader.setMat4("model", transform * getModelTransform());

        bindTextures(shader, TextureUniformSource);

        // Draw using the VAO.
        shader.activate();
        m_VertexArrayObj.activate();

        glDraw();

        m_VertexArrayObj.deactivate();

        // Reset.
        shader.deactivate();
    }

    void Mesh::initializeVertexArrayInstanceData()
    {
        if (m_uiInstanceCount)
        {
            // Allocate space for mat4 model transforms for the instancing.
            m_VertexArrayObj.allocateInstanceVertexData(m_uiInstanceCount * sizeof(glm::mat4));
            // Add vertex attributes (max attribute size is vec4, so we need 4 of them).
            m_VertexArrayObj.AddVertexAttrib(4, GL_FLOAT, /*instanceDivisor=*/1);
            m_VertexArrayObj.AddVertexAttrib(4, GL_FLOAT, /*instanceDivisor=*/1);
            m_VertexArrayObj.AddVertexAttrib(4, GL_FLOAT, /*instanceDivisor=*/1);
            m_VertexArrayObj.AddVertexAttrib(4, GL_FLOAT, /*instanceDivisor=*/1);
            m_VertexArrayObj.SetVertexAttribs();
        }
    }

    void Mesh::bindTextures(Shader& shader, TextureUniformSource* TextureUniformSource)
    {
        // Bind textures. Assumes uniform naming is "material.textureMapType[idx]".
        unsigned int diffuseIdx = 0;
        unsigned int specularIdx = 0;
        unsigned int roughnessIdx = 0;
        unsigned int metallicIdx = 0;
        unsigned int aoIdx = 0;
        unsigned int emissionIdx = 0;
        bool hasNormalMap = false;

        // If a TextureUniformSource isn't provided, just start with texture unit 0.
        unsigned int textureUnit = 0;
        if (TextureUniformSource != nullptr) 
        {
            TextureUniformSource->pushUsageBlock();
            textureUnit = TextureUniformSource->getNextTextureUnit();
        }

        // 加载模型才会执行for循环
        for (TextureMap& textureMap : m_vecTextureMaps)
        {
            std::string samplerName;
            TextureMapType type = textureMap.getType();
            Texture& texture = textureMap.getTexture();
            if (type == TextureMapType::CUBEMAP) 
            {
                texture.BindToUnit(textureUnit, TextureBindType::CUBEMAP);
                samplerName = "skybox";
            } 
            else 
            {
                texture.BindToUnit(textureUnit, TextureBindType::TEXTURE_2D);
                static std::string materialName = "material";
                std::ostringstream ss;
                // TODO: Make this more configurable / less generic?
                ss << "material.";

                // A subset of texture types can be packed into a single texture, which we
                // set a uniform for.
                switch (type) 
                {
                case TextureMapType::DIFFUSE:
                    ss << "diffuseMaps[" << diffuseIdx << "]";
                    diffuseIdx++;
                    break;
                case TextureMapType::SPECULAR:
                    ss << "specularMaps[" << specularIdx << "]";
                    specularIdx++;
                    break;
                case TextureMapType::ROUGHNESS:
                    ss << "roughnessMaps[" << roughnessIdx << "]";
                    shader.setBool(materialName + ".roughnessIsPacked[" +
                                        std::to_string(roughnessIdx) + "]",
                                    textureMap.isPacked());
                    roughnessIdx++;
                    break;
                case TextureMapType::METALLIC:
                    ss << "metallicMaps[" << metallicIdx << "]";
                    shader.setBool(materialName + ".metallicIsPacked[" +
                                        std::to_string(metallicIdx) + "]",
                                    textureMap.isPacked());
                    metallicIdx++;
                    break;
                case TextureMapType::AO:
                    ss << "aoMaps[" << aoIdx << "]";
                    shader.setBool(
                        materialName + ".aoIsPacked[" + std::to_string(aoIdx) + "]",
                        textureMap.isPacked());
                    aoIdx++;
                    break;
                case TextureMapType::EMISSION:
                    ss << "emissionMaps[" << emissionIdx << "]";
                    emissionIdx++;
                    break;
                case TextureMapType::NORMAL:
                    // Only a single normal map supported.
                    ss << "normalMap";
                    hasNormalMap = true;
                    break;
                case TextureMapType::CUBEMAP:
                    // Handled earlier.
                    abort();
                    break;
                }
                samplerName = ss.str();
            }
            // Set the sampler to the correct texture unit.
            shader.setInt(samplerName, textureUnit);

            if (TextureUniformSource != nullptr)
            {
                textureUnit = TextureUniformSource->getNextTextureUnit();
            } 
            else 
            {
                textureUnit++;
            }
        }
        if (TextureUniformSource != nullptr) 
        {
            TextureUniformSource->popUsageBlock();
        }
        shader.setInt("material.diffuseCount", diffuseIdx);
        shader.setInt("material.specularCount", specularIdx);
        shader.setInt("material.roughnessCount", roughnessIdx);
        shader.setInt("material.metallicCount", metallicIdx);
        shader.setInt("material.aoCount", aoIdx);
        shader.setInt("material.emissionCount", emissionIdx);
        shader.setInt("material.hasNormalMap", hasNormalMap);
    }

    void Mesh::glDraw() 
    {
        // Handle instancing.
        if (m_uiInstanceCount)
        {
            // Handle indexed arrays.
            if (!m_vecIndices.empty())
            {
                glDrawElementsInstanced(GL_TRIANGLES, m_vecIndices.size(), GL_UNSIGNED_INT,
                                        nullptr, m_uiInstanceCount);
            } 
            else 
            {
                glDrawArraysInstanced(GL_TRIANGLES, 0, m_uiNumVertices, m_uiInstanceCount);
            }
        } 
        else
        {
            // Handle indexed arrays.
            if (!m_vecIndices.empty())
            {
                glDrawElements(GL_TRIANGLES, m_vecIndices.size(), GL_UNSIGNED_INT, nullptr);
            }
            else 
            {
                glDrawArrays(GL_TRIANGLES, 0, m_uiNumVertices);
            }
        }
    }
}  // namespace Cme
