#pragma once

#include <glad/glad.h>
#include <vector>
#include "../camera.h"
#include "../shader/shader.h"

namespace Cme
{
	class Pipe
	{
	public:
		Pipe();
		Pipe(const std::vector<glm::vec3>& pathPoints, const std::vector<glm::vec3>& contourPoints);

		std::vector<glm::vec3> BuildSpiralPath(float r1, float r2, float h1, float h2, float turns, int points);
		std::vector<glm::vec3> BuildCircle(float radius, int steps);

		// °®ÐÄ
		glm::vec2 GetHeartPosition(float t);
		void BuildSegment(float t, float fOffset);

		int getPathCount() const { return (int)m_vecPath.size(); }
		const std::vector<glm::vec3>& getPathPoints() const { return m_vecPath; }
		const glm::vec3& getPathPoint(int index) const { return m_vecPath.at(index); }

		int getContourCount() const { return (int)m_vecContours.size(); }
		const std::vector< std::vector<glm::vec3> >& getContours() const { return m_vecContours; }
		const std::vector<glm::vec3>& getContour(int index) const { return m_vecContours.at(index); }

		const std::vector< std::vector<glm::vec3> >& getNormals() const { return m_vecNormals; }
		const std::vector<glm::vec3>& getNormal(int index) const { return m_vecNormals.at(index); }

		void SetColor(glm::vec3 Color)
		{
			m_vec3Color = Color;
		};

		void SetThickness(float fValue)
		{
			m_fThickness = fValue;
		}

	public:
		void InitializeData();
		void Render(std::shared_ptr<Cme::Camera> spCamera);
		void Update(float dt);

	private:
		void GenerateContours();
		void TransformFirstContour();
		std::vector<glm::vec3> ProjectContour(int fromIndex, int toIndex);
		std::vector<glm::vec3> ComputeContourNormal(int pathIndex);

		std::vector<glm::vec3> m_vecSpiralPath;
		std::vector<glm::vec3> m_vecPath;
		std::vector<glm::vec3> m_vecContour;
		std::vector<std::vector<glm::vec3>> m_vecContours;
		std::vector<std::vector<glm::vec3>> m_vecNormals;

		std::vector<glm::vec3> m_vecLovePath;

		GLuint m_VAO;
		GLuint m_VBO;
		GLuint m_EBO;
		Shader* m_pShader = nullptr;

		glm::vec3 m_vec3Color = glm::vec3(0.0f, 1.0f, 0.0f);;
		float m_fThickness = 0.1f;
	};
}