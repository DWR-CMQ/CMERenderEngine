#include "cylinder.h"
#include "../cme_defs.h"
#include <iostream>

const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT = 1;

namespace Cme
{
    Cylinder::Cylinder(float FuckbaseRadius, float FucktopRadius, float Fuckheight, int Fucksectors,
        int Fuckstacks, bool Fucksmooth, int Fuckup) : interleavedStride(32)
    {
        baseRadius = FuckbaseRadius;
        topRadius = FucktopRadius;
        height = Fuckheight;
        sectorCount = Fucksectors;

        if (Fucksectors < MIN_SECTOR_COUNT)
        {
            sectorCount = MIN_SECTOR_COUNT;
        }
        
        if (Fuckstacks < MIN_STACK_COUNT)
        {
            stackCount = MIN_STACK_COUNT;
        }
            
        stackCount = Fuckstacks;
        smooth = Fucksmooth;
        upAxis = Fuckup;
        if (Fuckup < 1 || Fuckup > 3)
        {
            upAxis = 3;
        }
            
        // generate unit circle vertices first
        buildUnitCircleVertices();

        if (smooth)
            buildVerticesSmooth();
        else
            buildVerticesFlat();

        m_VAO = 0;
        m_VBO = 0;
        m_EBO = 0;

        if (m_pShader == nullptr)
        {
            m_pShader = new Shader(Cme::ShaderPath("assets//shaders//cylinder.vert"),
                                   Cme::ShaderPath("assets//shaders//cylinder.frag"));
        }

        InitializeData();
    }

    Cylinder::~Cylinder()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);

        m_VAO = 0;
        m_VBO = 0;
        m_EBO = 0;
    }


    void Cylinder::clearArrays()
    {
        std::vector<float>().swap(vertices);
        std::vector<float>().swap(normals);
        std::vector<float>().swap(texCoords);
        std::vector<unsigned int>().swap(indices);
        std::vector<unsigned int>().swap(lineIndices);
    }

    void Cylinder::buildVerticesSmooth()
    {
        // clear memory of prev arrays
        clearArrays();

        float x, y, z;                                  // vertex position
        //float s, t;                                     // texCoord
        float radius;                                   // radius for each stack

        // get normals for cylinder sides
        std::vector<float> sideNormals = getSideNormals();

        // put vertices of side cylinder to array by scaling unit circle
        for (int i = 0; i <= stackCount; ++i)
        {
            z = -(height * 0.5f) + (float)i / stackCount * height;      // vertex position z
            radius = baseRadius + (float)i / stackCount * (topRadius - baseRadius);     // lerp
            float t = 1.0f - (float)i / stackCount;   // top-to-bottom

            for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
            {
                x = unitCircleVertices[k];
                y = unitCircleVertices[k + 1];
                addVertex(x * radius, y * radius, z);   // position
                addNormal(sideNormals[k], sideNormals[k + 1], sideNormals[k + 2]); // normal
                addTexCoord((float)j / sectorCount, t); // tex coord
            }
        }

        // remember where the base.top vertices start
        unsigned int baseVertexIndex = (unsigned int)vertices.size() / 3;

        // put vertices of base of cylinder
        z = -height * 0.5f;
        addVertex(0, 0, z);
        addNormal(0, 0, -1);
        addTexCoord(0.5f, 0.5f);
        for (int i = 0, j = 0; i < sectorCount; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVertex(x * baseRadius, y * baseRadius, z);
            addNormal(0, 0, -1);
            addTexCoord(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f);    // flip horizontal
        }

        // remember where the top vertices start
        unsigned int topVertexIndex = (unsigned int)vertices.size() / 3;

        // put vertices of top of cylinder
        z = height * 0.5f;
        addVertex(0, 0, z);
        addNormal(0, 0, 1);
        addTexCoord(0.5f, 0.5f);
        for (int i = 0, j = 0; i < sectorCount; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVertex(x * topRadius, y * topRadius, z);
            addNormal(0, 0, 1);
            addTexCoord(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
        }

        // put indices for sides
        unsigned int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // bebinning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 trianles per sector
                //std::cout << "k1 k1 + 1 k2     " << k1 << " " << k1 + 1 << " " << k2 << std::endl;
                //std::cout << "k2 k1 + 1 k2 + 1 " << k2 << " " << k1 + 1 << " " << k2 + 1 << std::endl;
                addIndices(k1, k1 + 1, k2);
                addIndices(k2, k1 + 1, k2 + 1);

                // vertical lines for all stacks
                lineIndices.push_back(k1);
                lineIndices.push_back(k2);
                // horizontal lines
                lineIndices.push_back(k2);
                lineIndices.push_back(k2 + 1);
                if (i == 0)
                {
                    lineIndices.push_back(k1);
                    lineIndices.push_back(k1 + 1);
                }
            }
        }

        //// remember where the base indices start
        //baseIndex = (unsigned int)indices.size();

        //// put indices for base
        //for (int i = 0, k = baseVertexIndex + 1; i < sectorCount; ++i, ++k)
        //{
        //    if (i < (sectorCount - 1))
        //        addIndices(baseVertexIndex, k + 1, k);
        //    else    // last triangle
        //        addIndices(baseVertexIndex, baseVertexIndex + 1, k);
        //}

        //// remember where the top indices start
        //topIndex = (unsigned int)indices.size();

        //for (int i = 0, k = topVertexIndex + 1; i < sectorCount; ++i, ++k)
        //{
        //    if (i < (sectorCount - 1))
        //        addIndices(topVertexIndex, k, k + 1);
        //    else
        //        addIndices(topVertexIndex, k, topVertexIndex + 1);
        //}

        // generate interleaved vertex array as well
        buildInterleavedVertices();

        // change up axis from Z-axis to the given
        if (this->upAxis != 3)
            changeUpAxis(3, this->upAxis);
    }

    void Cylinder::InitializeData()
    {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, getInterleavedVertexSize(), getInterleavedVertices(), GL_STATIC_DRAW);

        // EBO
        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexSize(), getIndices(), GL_STATIC_DRAW);

        // enable vertex array attributes for bound VAO
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        // store vertex array pointers to bound VAO
        int stride = getInterleavedStride();
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (void*)(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Cylinder::Render(std::shared_ptr<Cme::Camera> spCamera)
    {
        glBindVertexArray(m_VAO);

        auto modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -10.0f));
        m_pShader->setMat4("matrixModelViewProjection", spCamera->getProjectionTransform() * spCamera->getViewTransform() * modelMatrix);
        m_pShader->setMat4("matrixModelView", spCamera->getViewTransform() * modelMatrix);
        m_pShader->setMat4("matrixNormal", spCamera->getViewTransform() * modelMatrix);
        m_pShader->setMat4("u_vm", spCamera->getViewTransform());

        m_pShader->setVec4("lightPosition", glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        m_pShader->setVec4("lightAmbient", glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        m_pShader->setVec4("lightDiffuse", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
        m_pShader->setVec4("lightSpecular", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_pShader->setVec4("materialAmbient", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        m_pShader->setVec4("materialDiffuse", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
        m_pShader->setVec4("materialSpecular", glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
        m_pShader->setFloat("materialShininess", 16.0f);

        glDrawElements(GL_TRIANGLES,                // primitive type
            getIndexCount(),                        // # of indices
            GL_UNSIGNED_INT,             // data type
            (void*)0);                   // ptr to indices
    }

    ///////////////////////////////////////////////////////////////////////////////
    // generate vertices with flat shading
    // each triangle is independent (no shared vertices)
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::buildVerticesFlat()
    {
        // tmp vertex definition (x,y,z,s,t)
        struct Vertex
        {
            float x, y, z, s, t;
        };
        std::vector<Vertex> tmpVertices;

        int i, j, k;    // indices
        float x, y, z, s, t, radius;

        // put tmp vertices of cylinder side to array by scaling unit circle
        //NOTE: start and end vertex positions are same, but texcoords are different
        //      so, add additional vertex at the end point
        for (i = 0; i <= stackCount; ++i)
        {
            z = -(height * 0.5f) + (float)i / stackCount * height;      // vertex position z
            radius = baseRadius + (float)i / stackCount * (topRadius - baseRadius);     // lerp
            t = 1.0f - (float)i / stackCount;   // top-to-bottom

            for (j = 0, k = 0; j <= sectorCount; ++j, k += 3)
            {
                x = unitCircleVertices[k];
                y = unitCircleVertices[k + 1];
                s = (float)j / sectorCount;

                Vertex vertex;
                vertex.x = x * radius;
                vertex.y = y * radius;
                vertex.z = z;
                vertex.s = s;
                vertex.t = t;
                tmpVertices.push_back(vertex);
            }
        }

        // clear memory of prev arrays
        clearArrays();

        Vertex v1, v2, v3, v4;      // 4 vertex positions v1, v2, v3, v4
        std::vector<float> n;       // 1 face normal
        int vi1, vi2;               // indices
        int index = 0;

        // v2-v4 <== stack at i+1
        // | \ |
        // v1-v3 <== stack at i
        for (i = 0; i < stackCount; ++i)
        {
            vi1 = i * (sectorCount + 1);            // index of tmpVertices
            vi2 = (i + 1) * (sectorCount + 1);

            for (j = 0; j < sectorCount; ++j, ++vi1, ++vi2)
            {
                v1 = tmpVertices[vi1];
                v2 = tmpVertices[vi2];
                v3 = tmpVertices[vi1 + 1];
                v4 = tmpVertices[vi2 + 1];

                // compute a face normal of v1-v3-v2
                n = computeFaceNormal(v1.x, v1.y, v1.z, v3.x, v3.y, v3.z, v2.x, v2.y, v2.z);

                // put quad vertices: v1-v2-v3-v4
                addVertex(v1.x, v1.y, v1.z);
                addVertex(v2.x, v2.y, v2.z);
                addVertex(v3.x, v3.y, v3.z);
                addVertex(v4.x, v4.y, v4.z);

                // put tex coords of quad
                addTexCoord(v1.s, v1.t);
                addTexCoord(v2.s, v2.t);
                addTexCoord(v3.s, v3.t);
                addTexCoord(v4.s, v4.t);

                // put normal
                for (k = 0; k < 4; ++k)  // same normals for all 4 vertices
                {
                    addNormal(n[0], n[1], n[2]);
                }

                // put indices of a quad
                addIndices(index, index + 2, index + 1);    // v1-v3-v2
                addIndices(index + 1, index + 2, index + 3);    // v2-v3-v4

                // vertical line per quad: v1-v2
                lineIndices.push_back(index);
                lineIndices.push_back(index + 1);
                // horizontal line per quad: v2-v4
                lineIndices.push_back(index + 1);
                lineIndices.push_back(index + 3);
                if (i == 0)
                {
                    lineIndices.push_back(index);
                    lineIndices.push_back(index + 2);
                }

                index += 4;     // for next
            }
        }

        // remember where the base index starts
        baseIndex = (unsigned int)indices.size();
        unsigned int baseVertexIndex = (unsigned int)vertices.size() / 3;

        // put vertices of base of cylinder
        z = -height * 0.5f;
        addVertex(0, 0, z);
        addNormal(0, 0, -1);
        addTexCoord(0.5f, 0.5f);
        for (i = 0, j = 0; i < sectorCount; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVertex(x * baseRadius, y * baseRadius, z);
            addNormal(0, 0, -1);
            addTexCoord(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f); // flip horizontal
        }

        // put indices for base
        for (i = 0, k = baseVertexIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
                addIndices(baseVertexIndex, k + 1, k);
            else
                addIndices(baseVertexIndex, baseVertexIndex + 1, k);
        }

        // remember where the top index starts
        topIndex = (unsigned int)indices.size();
        unsigned int topVertexIndex = (unsigned int)vertices.size() / 3;

        // put vertices of top of cylinder
        z = height * 0.5f;
        addVertex(0, 0, z);
        addNormal(0, 0, 1);
        addTexCoord(0.5f, 0.5f);
        for (i = 0, j = 0; i < sectorCount; ++i, j += 3)
        {
            x = unitCircleVertices[j];
            y = unitCircleVertices[j + 1];
            addVertex(x * topRadius, y * topRadius, z);
            addNormal(0, 0, 1);
            addTexCoord(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
        }

        for (i = 0, k = topVertexIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
                addIndices(topVertexIndex, k, k + 1);
            else
                addIndices(topVertexIndex, k, topVertexIndex + 1);
        }

        // generate interleaved vertex array as well
        buildInterleavedVertices();

        // change up axis from Z-axis to the given
        if (this->upAxis != 3)
            changeUpAxis(3, this->upAxis);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // generate interleaved vertices: V/N/T
    // stride must be 32 bytes
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::buildInterleavedVertices()
    {
        std::vector<float>().swap(interleavedVertices);

        std::size_t i, j;
        std::size_t count = vertices.size();
        for (i = 0, j = 0; i < count; i += 3, j += 2)
        {
            //interleavedVertices.push_back(vertices[i]);
            //interleavedVertices.push_back(vertices[i+1]);
            //interleavedVertices.push_back(vertices[i+2]);
            interleavedVertices.insert(interleavedVertices.end(), &vertices[i], &vertices[i] + 3);

            //interleavedVertices.push_back(normals[i]);
            //interleavedVertices.push_back(normals[i+1]);
            //interleavedVertices.push_back(normals[i+2]);
            interleavedVertices.insert(interleavedVertices.end(), &normals[i], &normals[i] + 3);

            //interleavedVertices.push_back(texCoords[j]);
            //interleavedVertices.push_back(texCoords[j+1]);
            interleavedVertices.insert(interleavedVertices.end(), &texCoords[j], &texCoords[j] + 2);
        }
    }



    ///////////////////////////////////////////////////////////////////////////////
    // generate 3D vertices of a unit circle on XY plance
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::buildUnitCircleVertices()
    {
        const float PI = acos(-1.0f);
        float sectorStep = 2 * PI / sectorCount;
        float sectorAngle;  // radian

        std::vector<float>().swap(unitCircleVertices);
        for (int i = 0; i <= sectorCount; ++i)
        {
            sectorAngle = i * sectorStep;
            unitCircleVertices.push_back(cos(sectorAngle)); // x
            unitCircleVertices.push_back(sin(sectorAngle)); // y
            unitCircleVertices.push_back(0);                // z
        }
    }



    ///////////////////////////////////////////////////////////////////////////////
    // transform vertex/normal (x,y,z) coords
    // assume from/to values are validated: 1~3 and from != to
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::changeUpAxis(int from, int to)
    {
        // initial transform matrix cols
        float tx[] = { 1.0f, 0.0f, 0.0f };    // x-axis (left)
        float ty[] = { 0.0f, 1.0f, 0.0f };    // y-axis (up)
        float tz[] = { 0.0f, 0.0f, 1.0f };    // z-axis (forward)

        // X -> Y
        if (from == 1 && to == 2)
        {
            tx[0] = 0.0f; tx[1] = 1.0f;
            ty[0] = -1.0f; ty[1] = 0.0f;
        }
        // X -> Z
        else if (from == 1 && to == 3)
        {
            tx[0] = 0.0f; tx[2] = 1.0f;
            tz[0] = -1.0f; tz[2] = 0.0f;
        }
        // Y -> X
        else if (from == 2 && to == 1)
        {
            tx[0] = 0.0f; tx[1] = -1.0f;
            ty[0] = 1.0f; ty[1] = 0.0f;
        }
        // Y -> Z
        else if (from == 2 && to == 3)
        {
            ty[1] = 0.0f; ty[2] = 1.0f;
            tz[1] = -1.0f; tz[2] = 0.0f;
        }
        //  Z -> X
        else if (from == 3 && to == 1)
        {
            tx[0] = 0.0f; tx[2] = -1.0f;
            tz[0] = 1.0f; tz[2] = 0.0f;
        }
        // Z -> Y
        else
        {
            ty[1] = 0.0f; ty[2] = -1.0f;
            tz[1] = 1.0f; tz[2] = 0.0f;
        }

        std::size_t i, j;
        std::size_t count = vertices.size();
        float vx, vy, vz;
        float nx, ny, nz;
        for (i = 0, j = 0; i < count; i += 3, j += 8)
        {
            // transform vertices
            vx = vertices[i];
            vy = vertices[i + 1];
            vz = vertices[i + 2];
            vertices[i] = tx[0] * vx + ty[0] * vy + tz[0] * vz;   // x
            vertices[i + 1] = tx[1] * vx + ty[1] * vy + tz[1] * vz;   // y
            vertices[i + 2] = tx[2] * vx + ty[2] * vy + tz[2] * vz;   // z

            // transform normals
            nx = normals[i];
            ny = normals[i + 1];
            nz = normals[i + 2];
            normals[i] = tx[0] * nx + ty[0] * ny + tz[0] * nz;   // nx
            normals[i + 1] = tx[1] * nx + ty[1] * ny + tz[1] * nz;   // ny
            normals[i + 2] = tx[2] * nx + ty[2] * ny + tz[2] * nz;   // nz

            // trnasform interleaved array
            interleavedVertices[j] = vertices[i];
            interleavedVertices[j + 1] = vertices[i + 1];
            interleavedVertices[j + 2] = vertices[i + 2];
            interleavedVertices[j + 3] = normals[i];
            interleavedVertices[j + 4] = normals[i + 1];
            interleavedVertices[j + 5] = normals[i + 2];
        }
    }



    ///////////////////////////////////////////////////////////////////////////////
    // add single vertex to array
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::addVertex(float x, float y, float z)
    {
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // add single normal to array
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::addNormal(float nx, float ny, float nz)
    {
        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(nz);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // add single texture coord to array
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::addTexCoord(float s, float t)
    {
        texCoords.push_back(s);
        texCoords.push_back(t);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // add 3 indices to array
    ///////////////////////////////////////////////////////////////////////////////
    void Cylinder::addIndices(unsigned int i1, unsigned int i2, unsigned int i3)
    {
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i3);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // generate shared normal vectors of the side of cylinder
    ///////////////////////////////////////////////////////////////////////////////
    std::vector<float> Cylinder::getSideNormals()
    {
        const float PI = acos(-1.0f);
        float sectorStep = 2 * PI / sectorCount;
        float sectorAngle;  // radian

        // compute the normal vector at 0 degree first
        // tanA = (baseRadius-topRadius) / height
        float zAngle = atan2(baseRadius - topRadius, height);
        float x0 = cos(zAngle);     // nx
        float y0 = 0;               // ny
        float z0 = sin(zAngle);     // nz

        // rotate (x0,y0,z0) per sector angle
        std::vector<float> normals;
        for (int i = 0; i <= sectorCount; ++i)
        {
            sectorAngle = i * sectorStep;
            normals.push_back(cos(sectorAngle) * x0 - sin(sectorAngle) * y0);   // nx
            normals.push_back(sin(sectorAngle) * x0 + cos(sectorAngle) * y0);   // ny
            normals.push_back(z0);  // nz
            /*
            //debug
            float nx = cos(sectorAngle)*x0 - sin(sectorAngle)*y0;
            float ny = sin(sectorAngle)*x0 + cos(sectorAngle)*y0;
            std::cout << "normal=(" << nx << ", " << ny << ", " << z0
                      << "), length=" << sqrtf(nx*nx + ny*ny + z0*z0) << std::endl;
            */
        }

        return normals;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // return face normal of a triangle v1-v2-v3
    // if a triangle has no surface (normal length = 0), then return a zero vector
    ///////////////////////////////////////////////////////////////////////////////
    std::vector<float> Cylinder::computeFaceNormal(float x1, float y1, float z1,  // v1
        float x2, float y2, float z2,  // v2
        float x3, float y3, float z3)  // v3
    {
        const float EPSILON = 0.000001f;

        std::vector<float> normal(3, 0.0f);     // default return value (0,0,0)
        float nx, ny, nz;

        // find 2 edge vectors: v1-v2, v1-v3
        float ex1 = x2 - x1;
        float ey1 = y2 - y1;
        float ez1 = z2 - z1;
        float ex2 = x3 - x1;
        float ey2 = y3 - y1;
        float ez2 = z3 - z1;

        // cross product: e1 x e2
        nx = ey1 * ez2 - ez1 * ey2;
        ny = ez1 * ex2 - ex1 * ez2;
        nz = ex1 * ey2 - ey1 * ex2;

        // normalize only if the length is > 0
        float length = sqrtf(nx * nx + ny * ny + nz * nz);
        if (length > EPSILON)
        {
            // normalize
            float lengthInv = 1.0f / length;
            normal[0] = nx * lengthInv;
            normal[1] = ny * lengthInv;
            normal[2] = nz * lengthInv;
        }

        return normal;
    }

}