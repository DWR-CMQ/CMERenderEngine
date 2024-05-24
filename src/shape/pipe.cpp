#include "pipe.h"
#include "line.h"
#include "plane.h"
#include "../common_helper.h"

#include <iostream>

const int CIRCLE_SECTORS = 100;     // # of vertices per contour
const int PATH_POINTS = 300;        // # of points on the path
const float PATH_TURNS = 4.5f;      // # of turns of the spiral

// Variables
const int POINT_COUNT = 64;
const float speed = -0.5;
const float len = 0.015;
const float radius = 0.5;

namespace Cme
{
	Pipe::Pipe(float fOffset, glm::vec3 vec3RimLight, glm::vec3 vec3Ambient)
	{
        // m_vecSpiralPath.clear();
        //m_vecSpiralPath = BuildSpiralPath(4, 1, -3, 3, PATH_TURNS, PATH_POINTS);
        //m_vecPath = m_vecSpiralPath;
        //m_vecContour = BuildCircle(0.1f, CIRCLE_SECTORS);
        //GenerateContours();

        m_vecLovePath.clear();
        //BuildSegment(2.0f, 0.0);
        //m_vecPath = m_vecLovePath;
        m_vecContour = BuildCircle(0.1f, CIRCLE_SECTORS);
        GenerateContours();

        m_fOffset = fOffset;
        vec3LoveRimLight = vec3RimLight;
        vec3LoveAmbient = vec3Ambient;

        m_VAO = 0;
        m_VBO = 0;
        m_EBO = 0;

        if (m_pShader == nullptr)
        {
            m_pShader = new Shader(Cme::ShaderPath("assets//shaders//spiral.vert"),
                Cme::ShaderPath("assets//shaders//spiral.frag"));
        }
        InitializeData();
	}

    void Pipe::InitializeData()
    {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        BuildSegment(1.0f, m_fOffset);
        m_vecContour = BuildCircle(m_fThickness, CIRCLE_SECTORS);
        GenerateContours();

        int contourCount = getContourCount();            // total # of contours
        int vertexCount = (int)getContour(0).size();     // # of vertices per contour
        int vertexSize = sizeof(float) * 3 * vertexCount;     // # of bytes per contour
        int offset = vertexSize * contourCount;               // offset where normals begin


        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)offset);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    void Pipe::Update(float dt)
    {
        // 重新计算会非常耗时
        m_vecContour.clear();
        BuildSegment(dt, m_fOffset);
        m_vecContour = BuildCircle(m_fThickness, CIRCLE_SECTORS);
        GenerateContours();

        int contourCount = getContourCount();            // total # of contours
        int vertexCount = (int)getContour(0).size();     // # of vertices per contour
        int vertexSize = sizeof(float) * 3 * vertexCount;     // # of bytes per contour
        int offset = vertexSize * contourCount;               // offset where normals begin
        int bufferCount = 2 * 3 * vertexCount * contourCount; // total # of floats (vertex + normal)
        int bufferSize = sizeof(float) * bufferCount;         // total # of bytes
        std::vector<float> buffer(bufferCount);

        for (int i = 0; i < contourCount; ++i)
        {
            std::vector<glm::vec3> contour = m_vecContours[i]; // vertices per contour
            std::vector<glm::vec3> normal = m_vecNormals[i];   // normals per contours
            int vIndex = i * 3 * vertexCount;
            int nIndex = (3 * vertexCount * contourCount) + vIndex;
            // copy # of bytes per contour
            std::memcpy(&buffer[vIndex], &contour[0].x, vertexSize);
            std::memcpy(&buffer[nIndex], &normal[0].x, vertexSize);
        }

        // copy to VBO using glBufferData()
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer.data(), GL_DYNAMIC_DRAW);

        for (int i = 0; i < contourCount; ++i)
        {
            std::vector<glm::vec3> contour = m_vecContours[i]; // vertices per contour
            std::vector<glm::vec3> normal = m_vecNormals[i];   // normals per contours
            glBufferSubData(GL_ARRAY_BUFFER, (i * vertexSize), vertexSize, &contour[0].x);
            glBufferSubData(GL_ARRAY_BUFFER, offset + (i * vertexSize), vertexSize, &normal[0].x);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // build indices for triangle strip
        int k1 = 0, k2 = vertexCount;
        std::vector<unsigned int> indices;
        for (int i = 0; i < (contourCount - 1); ++i)
        {
            for (int j = 0; j < vertexCount; ++j)
            {
                indices.push_back(k2++);
                indices.push_back(k1++);
            }
        }

        // copy indices to IBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void Pipe::Render(std::shared_ptr<Cme::Camera> spCamera, glm::vec3 vec3LightDir)
    {
        m_pShader->activate();
        auto modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -8.0f));
        m_pShader->setMat4("projection", spCamera->getProjectionTransform());
        m_pShader->setMat4("view", spCamera->getViewTransform());
        m_pShader->setMat4("model", modelMatrix);
        m_pShader->setVec3("lightDirection", vec3LightDir);
        m_pShader->setVec3("viewPos", spCamera->getPosition());

        m_pShader->setVec3("rimlight", vec3LoveRimLight);
        m_pShader->setVec3("ambient", vec3LoveAmbient);

        //m_pShader->setVec4("lightPosition", glm::vec4(vec3LoveLightPosition, 0.0f));
        //m_pShader->setVec4("lightAmbient", glm::vec4(vec3LoveLightAmbient, 1.0f));
        //m_pShader->setVec4("lightDiffuse", glm::vec4(vec3LoveLightDiffuse, 1.0f));
        //m_pShader->setVec4("lightSpecular", glm::vec4(vec3LoveLightSpecular, 1.0f));
        //m_pShader->setVec4("materialAmbient", glm::vec4(vec3LoveMaterialAmbient, 1.0f));
        //m_pShader->setVec4("materialDiffuse", glm::vec4(vec3LoveMaterialDiffuse, 1.0f));
        //m_pShader->setVec4("materialSpecular", glm::vec4(vec3LoveMaterialSpecular, 1.0f));
        //m_pShader->setFloat("materialShininess", 16.0f);

        glBindVertexArray(m_VAO);
        int indexSize = 2 * (getContourCount() - 1) * getContour(0).size();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glDrawElements(GL_TRIANGLE_STRIP, indexSize, GL_UNSIGNED_INT, 0);

    }

	std::vector<glm::vec3> Pipe::BuildSpiralPath(float r1, float r2, float h1, float h2, float turns, int points)
	{
        const float PI = acos(-1.0f);
        std::vector<glm::vec3> vecResult;
        glm::vec3 vertex;
        float r = r1;
        float rStep = (r2 - r1) / (points - 1);
        float y = h1;
        float yStep = (h2 - h1) / (points - 1);
        float a = 0;
        float aStep = (turns * 2 * PI) / (points - 1);
        for (int i = 0; i < points; ++i)
        {
            vertex.x = r * cos(a);
            vertex.z = r * sin(a);
            vertex.y = y;
            vecResult.push_back(vertex);
            // next
            r += rStep;
            y += yStep;
            a += aStep;
        }
        return vecResult;
	}

    std::vector<glm::vec3> Pipe::BuildCircle(float radius, int steps)
    {
        std::vector<glm::vec3> vecResult;
        if (steps < 2) return vecResult;

        const float PI2 = acos(-1.0f) * 2.0f;
        float x, y, a;
        for (int i = 0; i <= steps; ++i)
        {
            a = PI2 / steps * i;
            x = radius * cosf(a);
            y = radius * sinf(a);
            vecResult.push_back(glm::vec3(x, y, 0));
        }
        return vecResult;
    }

    void Pipe::GenerateContours()
    {
        // reset
        m_vecContours.clear();
        m_vecNormals.clear();

        // path must have at least a point
        if (m_vecPath.size() < 1)
        {
            return;
        }
            
        // rotate and translate the contour to the first path point
        TransformFirstContour();
        m_vecContours.push_back(m_vecContour);
        m_vecNormals.push_back(ComputeContourNormal(0));

        // project contour to the plane at the next path point
        int count = (int)m_vecPath.size();
        for (int i = 1; i < count; ++i)
        {
            m_vecContours.push_back(ProjectContour(i - 1, i));
            m_vecNormals.push_back(ComputeContourNormal(i));
        }
    }

    std::vector<glm::vec3> Pipe::ComputeContourNormal(int pathIndex)
    {
        // get current contour and center point
        std::vector<glm::vec3>& contour = m_vecContours[pathIndex];
        glm::vec3 center = m_vecPath[pathIndex];
        
        std::vector<glm::vec3> contourNormal;
        glm::vec3 normal;
        for (int i = 0; i < (int)contour.size(); ++i)
        {
            normal = glm::normalize(contour[i] - center);
            contourNormal.push_back(normal);
        }

        return contourNormal;
    }

    std::vector<glm::vec3> Pipe::ProjectContour(int fromIndex, int toIndex)
    {
        glm::vec3 dir1, dir2, normal;
        Line line;

        dir1 = m_vecPath[toIndex] - m_vecPath[fromIndex];
        if (toIndex == (int)m_vecPath.size() - 1)
        {
            dir2 = dir1;
        }
        else
        {
            dir2 = m_vecPath[toIndex + 1] - m_vecPath[toIndex];
        }
            

        normal = dir1 + dir2;               // normal vector of plane at toIndex
        Plane plane(normal, m_vecPath[toIndex]);

        // project each vertex of contour to the plane
        std::vector<glm::vec3>& fromContour = m_vecContours[fromIndex];
        std::vector<glm::vec3> toContour;
        int count = (int)fromContour.size();
        for (int i = 0; i < count; ++i)
        {
            line.set(dir1, fromContour[i]);
            toContour.push_back(plane.intersect(line));
        }

        return toContour;
    }

    void Pipe::TransformFirstContour()
    {
        int pathCount = (int)m_vecPath.size();
        int vertexCount = (int)m_vecContour.size();
        glm::mat4 matrix;

        if (pathCount > 0)
        {
            // transform matrix
            if (pathCount > 1)
            {
                matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), m_vecPath[0] - m_vecPath[1], glm::vec3(0.0f, 1.0f, 0.0f));
                matrix = glm::transpose(matrix);
                matrix[3][0] = m_vecPath[0].x;
                matrix[3][1] = m_vecPath[0].y;
                matrix[3][2] = m_vecPath[0].z;
            }

            // multiply matrix to the contour
            // NOTE: the contour vertices are transformed here
            // MUST resubmit contour data if the path is resset to 0
            for (int i = 0; i < vertexCount; ++i)
            {
                // 根据线的方向确定转换矩阵
                m_vecContour[i] = matrix * glm::vec4(m_vecContour[i], 1.0f);
            }
        }
    }

    glm::vec2 Pipe::GetHeartPosition(float t)
    {
        return glm::vec2(16.0 * sin(t) * sin(t) * sin(t) * radius,
                -(13.0 * cos(t) - 5.0 * cos(2.0 * t) - 2.0 * cos(3.0 * t) - cos(4.0 * t))) * radius;
    }

    void Pipe::BuildSegment(float t, float fOffset)
    {
        // 爱心的中心位置
        m_vecPath.clear();
        for (int i = 0; i < POINT_COUNT; i++)
        {
            vec3 vec3XYZ = vec3(GetHeartPosition(fOffset + float(i) * len + fract(speed * t) * 6.28), 2.0);
            m_vecPath.push_back(vec3XYZ * glm::vec3(1.0f, -1.0f, 1.0f));      // 反转Y轴方向
        }
    }
}