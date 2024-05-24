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
		Pipe(float fOffset, glm::vec3 vec3RimLight, glm::vec3 vec3Ambient);

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

		void SetThickness(float fValue)
		{
			m_fThickness = fValue;
		}

		void SetLightPosition(glm::vec3 value)
		{
			vec3LoveLightPosition = value;
		}

		void SetLightAmbient(glm::vec3 value)
		{
			vec3LoveLightAmbient = value;
		};

		void SetLightDiffuse(glm::vec3 value)
		{
			vec3LoveLightDiffuse = value;
		}

		void SetLightSpecular(glm::vec3 value)
		{
			vec3LoveLightSpecular = value;
		}

		void SetMaterialAmbient(glm::vec3 value)
		{
			vec3LoveMaterialAmbient = value;
		}

		void SetMaterialDiffuse(glm::vec3 value)
		{
			vec3LoveMaterialDiffuse = value;
		}

		void SetMaterialSpecular(glm::vec3 value)
		{
			vec3LoveMaterialSpecular = value;
		}

	public:
		void InitializeData();
		void Render(std::shared_ptr<Cme::Camera> spCamera, glm::vec3 vec3LightDir);
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
		float m_fOffset;

		glm::vec3 m_vec3Color = glm::vec3(0.0f, 1.0f, 0.0f);;
		float m_fThickness = 0.1f;
		glm::vec3 vec3LoveLightPosition = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 vec3LoveLightAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
		glm::vec3 vec3LoveLightDiffuse = glm::vec3(0.7f, 0.7f, 0.7f);
		glm::vec3 vec3LoveLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 vec3LoveMaterialAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
		glm::vec3 vec3LoveMaterialDiffuse = glm::vec3(0.7f, 0.7f, 0.7f);
		glm::vec3 vec3LoveMaterialSpecular = glm::vec3(0.4f, 0.4f, 0.4f);

		glm::vec3 vec3LoveRimLight = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 vec3LoveAmbient = glm::vec3(0.0f, 1.0f, 0.4f);
	};
}