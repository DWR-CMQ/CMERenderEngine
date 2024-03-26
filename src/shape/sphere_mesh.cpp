#include "sphere_mesh.h"

namespace Cme
{
    SphereMesh::SphereMesh(std::string texturePath, int numMeridians, int numParallels)
        : m_iNumMeridians(numMeridians), m_iNumParallels(numParallels)
    {
        std::vector<TextureMap> textureMaps;
        if (!texturePath.empty())
        {
            TextureMap textureMap(Texture::LoadTexture(texturePath.c_str()), TextureMapType::DIFFUSE);
            textureMaps.push_back(textureMap);
        }
        loadMeshAndTextures(textureMaps);
    }

    SphereMesh::SphereMesh(const std::vector<TextureMap>& textureMaps,
        int numMeridians, int numParallels)
        : m_iNumMeridians(numMeridians), m_iNumParallels(numParallels)
    {
        loadMeshAndTextures(textureMaps);
    }

    void SphereMesh::loadMeshAndTextures(const std::vector<TextureMap>& textureMaps)
    {
        // Generate the sphere vertex components. This uses the common "UV" approach.

        const float PI = glm::pi<float>();

        // Always use at least 3 meridians.
        const unsigned int widthSegments = glm::max(m_iNumMeridians, 3);
        // Add two segments to account for the poles.
        const unsigned int heightSegments = glm::max(m_iNumParallels, 2);

        std::vector<float> vertexData;
        // We use <= instead of < because we want to create one more "layer" of
        // vertices in order for UVs to work properly.
        for (unsigned int iy = 0; iy <= heightSegments; ++iy)
        {
            // We can subdividue the surface into meridians / parallels, and treat
            // these as UVs since they lie in the range [0, 1].
            float v = iy / static_cast<float>(heightSegments);
            // However, we adjust it by a tiny amount to ensure that we don't generate
            // tangents of zero length (due to the sin() in the calculations).
            // TODO: Is this reasonable?
            v += 1e-5;
            // Use a special offset for the poles.
            float uOffset = 0.0f;
            if (iy == 0)
            {
                uOffset = 0.5f / widthSegments;
            }
            else if (iy == heightSegments)
            {
                uOffset = -0.5f / widthSegments;
            }
            for (unsigned int ix = 0; ix <= widthSegments; ++ix)
            {
                float u = ix / static_cast<float>(widthSegments);

                float x = cos(u * 2.0f * PI) * sin(v * PI);
                float y = cos(v * PI);
                float z = sin(u * 2.0f * PI) * sin(v * PI);

                // Positions.
                vertexData.push_back(x);
                vertexData.push_back(y);
                vertexData.push_back(z);
                // Normals. Unit sphere makes this easy. 8D
                vertexData.push_back(x);
                vertexData.push_back(y);
                vertexData.push_back(z);
                // Tangents. Rotated 90 degrees about the origin.
                vertexData.push_back(-z);
                vertexData.push_back(0.0f);  // Spherical tangents ignore parallels.
                vertexData.push_back(x);
                // Texture coordinates.
                vertexData.push_back(u + uOffset);
                vertexData.push_back(1.0f - v);  // We're generating the sphere top-down, so reverse V.
            }
        }


        std::vector<unsigned int> indices;
        // Since we created an extra duplicate "wraparound" vertex for each parallel,
        // we have to adjust the stride.
        const unsigned int widthStride = widthSegments + 1;
        for (unsigned int iy = 0; iy < heightSegments; ++iy)
        {
            for (unsigned int ix = 0; ix < widthSegments; ++ix)
            {
                // Form triangles from quad vertices.
                unsigned int a = iy * widthStride + (ix + 1);
                unsigned int b = iy * widthStride + ix;
                unsigned int c = (iy + 1) * widthStride + ix;
                unsigned int d = (iy + 1) * widthStride + (ix + 1);

                if (iy != 0)
                {
                    indices.push_back(a);
                    indices.push_back(b);
                    indices.push_back(d);
                }
                if (iy != heightSegments - 1)
                {
                    indices.push_back(b);
                    indices.push_back(c);
                    indices.push_back(d);
                }
            }
        }

        constexpr unsigned int sphereVertexSizeBytes = 11 * sizeof(float);
        LoadMeshData(vertexData.data(),
            (sizeof(float) * vertexData.size()) / sphereVertexSizeBytes,
            sphereVertexSizeBytes, indices, textureMaps);
    }

    void SphereMesh::initializeVertexAttributes()
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
}
