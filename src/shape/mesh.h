#ifndef QUARKGL_MESH_H_
#define QUARKGL_MESH_H_

#include <glad/glad.h>

#include "../shader/shader.h"
#include "../texture_map.h"
#include "../texture_registry.h"
#include "../vertex_array.h"

#include <functional>
#include <glm/glm.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace Cme 
{
    class Renderable 
    {
    public:
        virtual ~Renderable() = default;

        glm::mat4 getModelTransform() const { return m_mat4Model; }
        void setModelTransform(const glm::mat4& model) { m_mat4Model = model; }

        // TODO: Add translation, rotation, scale methods.

        virtual void draw(Shader& shader, TextureRegistry* textureRegistry = nullptr)
        {
            glm::mat4 transform(1.0f);
            return drawWithTransform(transform, shader, textureRegistry);
        }
        virtual void drawWithTransform(
            const glm::mat4& transform, Shader& shader,
            TextureRegistry* textureRegistry = nullptr) = 0;

    protected:
        // The model transform matrix.
        glm::mat4 m_mat4Model = glm::mat4(1.0f);
    };

    // A node in a tree of Renderables, containing one or more renderables.
    // When rendering, each node's transform is applied to the transform of its
    // Renderables, as well as to its child RenderableNodes.
    class RenderableNode : public Renderable 
    {
    public:
        virtual ~RenderableNode() = default;
        void drawWithTransform(const glm::mat4& transform, Shader& shader,
                                TextureRegistry* textureRegistry = nullptr) override;

        void addRenderable(std::unique_ptr<Renderable> renderable) 
        {
            m_vecRenderables.push_back(std::move(renderable));
        }

        void addChildNode(std::unique_ptr<RenderableNode> childNode) 
        {
            m_vecChildNodes.push_back(std::move(childNode));
        }

        void visitRenderables(std::function<void(Renderable*)> visitor);

    protected:
        // The set of Renderables making up this node.
        std::vector<std::unique_ptr<Renderable>> m_vecRenderables;
        // The set of child RenderableNodes.
        std::vector<std::unique_ptr<RenderableNode>> m_vecChildNodes;
    };

    // An abstract class that represents a triangle mesh and handles loading and
    // rendering. Child classes can specialize when configuring vertex attributes.
    class Mesh : public Renderable
    {
    public:
        virtual ~Mesh() = default;

        void loadInstanceModels(const std::vector<glm::mat4>& models);
        void loadInstanceModels(const glm::mat4* models, unsigned int size);
        void drawWithTransform(const glm::mat4& transform, Shader& shader,
                                TextureRegistry* textureRegistry = nullptr) override;

        std::vector<unsigned int> getIndices() { return m_vecIndices; }
        std::vector<TextureMap> getTextureMaps() { return m_vecTextureMaps; }

    protected:
        // Loads mesh data into the mesh. Calls initializeVertexAttributes and
        // initializeVertexArrayInstanceData under the hood. Must be called
        // immediately after construction.
        virtual void loadMeshData(const void* vertexData, unsigned int numVertices,
                                unsigned int vertexSizeBytes,
                                const std::vector<unsigned int>& indices,
                                const std::vector<TextureMap>& textureMaps,
                                unsigned int instanceCount = 0);
        // Initializes vertex attributes.
        virtual void initializeVertexAttributes() = 0;
        // Allocates and initializes vertex array instance data.
        virtual void initializeVertexArrayInstanceData();
        // Binds texture maps to texture units and sets shader sampler uniforms.
        virtual void bindTextures(Shader& shader, TextureRegistry* textureRegistry);
        // Emits glDraw* calls based on the mesh instancing/indexing. Requires shaders
        // and VAOs to be active prior to calling.
        virtual void glDraw();

        VertexArray m_VertexArrayObj;
        std::vector<unsigned int> m_vecIndices;
        std::vector<TextureMap> m_vecTextureMaps;

        // The number of vertices in the mesh.
        unsigned int m_uiNumVertices;
        // The size, in bytes, of each vertex.
        unsigned int m_uiVertexSizeBytes;
        unsigned int m_uiInstanceCount;
    };

}  // namespace Cme

#endif
