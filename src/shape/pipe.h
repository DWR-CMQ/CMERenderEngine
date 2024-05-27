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
		Pipe(float fOffset, glm::vec3 vec3RimColor);

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

		void UpdateMaterial(glm::vec3 vec3Ambient, glm::vec3 vec3Diffuse, glm::vec3 vec3Specular, float fShininess)
		{
			m_vec3LoveMaterialAmbient = vec3Ambient;
			m_vec3LoveMaterialDiffuse = vec3Diffuse;
			m_vec3LoveMaterialSpecular = vec3Specular;
			m_fLoveMaterialShiniess = fShininess;
		}

		void UpdateLight(glm::vec3 vec3Position, glm::vec3 vec3Ambient, glm::vec3 vec3Diffuse, glm::vec3 vec3Specular)
		{
			m_vec3LoveLightPosition = vec3Position;
			m_vec3LoveLightaAmbient = vec3Ambient;
			m_vec3LoveLightDiffuse = vec3Diffuse;
			m_vec3LoveLightSpecular = vec3Specular;
		}

		void UpdateRim(glm::vec3 vec3Color, float fWidth, float fStrength)
		{
			m_vec3LoveRimColor = vec3Color;
			m_fLoveRimWidth = fWidth;
			m_fLoveRimStrength = fStrength;
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
		float m_fOffset;
		
		float m_fThickness = 0.1f;
		float m_fTime = 0.0f;

		// Light
		glm::vec3 m_vec3LoveLightPosition = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 m_vec3LoveLightaAmbient = glm::vec3(0.7f, 0.7f, 0.7f);
		glm::vec3 m_vec3LoveLightDiffuse = glm::vec3(0.7f, 0.7f, 0.7f);
		glm::vec3 m_vec3LoveLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
		
		// Material
		glm::vec3 m_vec3LoveMaterialAmbient = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 m_vec3LoveMaterialDiffuse = glm::vec3(0.7f, 0.7f, 0.7f);
		glm::vec3 m_vec3LoveMaterialSpecular = glm::vec3(0.7f, 0.7f, 0.7f);
		float m_fLoveMaterialShiniess = 32.0f;

		float m_fLoveRimWidth = 1.0f;
		float m_fLoveRimStrength = 2.0f;
		glm::vec3 m_vec3LoveRimColor = glm::vec3(1.0f, 1.0f, 1.0f);
	};
}