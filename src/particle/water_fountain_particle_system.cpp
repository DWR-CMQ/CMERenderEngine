#include "water_fountain_particle_system.h"
#include "../cme_defs.h"
#include "../stb_image.h"
#include "../core/texture_manager.h"
#include "../common_helper.h"

namespace Cme
{
	const std::string WaterFountainParticleSystem::WATERFOUNTAIN_KEY = "waterfountain";

	WaterFountainParticleSystem::WaterFountainParticleSystem()
	{
		m_uiVao = 0;
		m_uiVbo = 0;
		m_spCamera = std::make_shared<Camera>();
	}

	WaterFountainParticleSystem::~WaterFountainParticleSystem()
	{
		glDeleteVertexArrays(1, &m_uiVao);
		glDeleteBuffers(1, &m_uiVbo);

		m_uiVao = 0;
		m_uiVbo = 0;
	}

	void WaterFountainParticleSystem::InitPS(float*, unsigned int v_count,
											unsigned int tex, float* uv, bool atlas)
	{
		if (m_pComputeShader == nullptr)
		{
			m_pComputeShader = new Shader(Cme::ShaderPath("assets//shaders//water_fountain_scene.comp"));
		}
		if (m_pDrawShader == nullptr)
		{
			m_pDrawShader = new Shader(Cme::ShaderPath("assets//shaders//water_fountain_scene.vert"),
									 Cme::ShaderPath("assets//shaders//water_fountain_scene.frag"),
									 Cme::ShaderPath("assets//shaders//water_fountain_scene.geom"));
		}

		glGenVertexArrays(1, &m_uiVao);
		glGenBuffers(1, &m_uiVbo);

		m_pDrawShader->activate();
		// draw_shader->bind("GlobalAttributes", 0);
		m_pDrawShader->setInt("sprite", 0);
		glBindVertexArray(m_uiVao);
		glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
		glEnableVertexAttribArray(0);   // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(0));
		glEnableVertexAttribArray(1);   // color alpha
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);   // delta position
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float)));
		m_pDrawShader->deactivate();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		auto& tm = TextureManager::GetInstance();
		{
			auto spTexture = std::make_shared<Texture>();
			spTexture->LoadTexture("assets//texture//particle3.png", true);
			tm.AddTexture(WATERFOUNTAIN_KEY, std::vector<std::shared_ptr<Texture>>{spTexture});
		}

		tot = max_particles;
		debt = 0.f;
		upload = true;
	}

	void WaterFountainParticleSystem::Restart()
	{
		//tot = max_particles;
		//debt = 0.f;
		//upload = true;
	}

	void WaterFountainParticleSystem::Upload()
	{
		if (!upload)
		{
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * max_particles, nullptr, GL_STATIC_DRAW);

		m_uiParticleCount = max_particles;
		Update(-1.f, 0.0f);
		m_uiParticleCount = 0;

		upload = false;
	}

	void WaterFountainParticleSystem::Update(float fDelta, float fTime)
	{
		m_fTime = fTime;
		if (m_uiParticleCount != total())
		{
			debt += std::min(0.05f, fDelta) * birth_rate;
			auto new_particles = static_cast<int>(debt);
			debt -= new_particles;
			m_uiParticleCount += new_particles;
			if (m_uiParticleCount > total())
			{
				m_uiParticleCount = total();
			}
		}

		// std::cout << "dt count(): " << dt  << " "  << count() << std::endl;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_uiVbo);
		m_pComputeShader->activate();
		m_pComputeShader->setFloat("dt", fDelta);
		m_pComputeShader->setVec3("acceleration", acceleration);
		m_pComputeShader->setInt("n_particles", static_cast<int>(count()));
		// 此处报错
		glDispatchCompute(256, 1, 1);
		m_pComputeShader->deactivate();
	}

	void WaterFountainParticleSystem::Render(GLenum gl_draw_mode)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		m_pDrawShader->activate();
		m_pDrawShader->setMat4("view", m_spCamera->getViewTransform());
		m_pDrawShader->setMat4("proj", m_spCamera->getProjectionTransform());
		m_pDrawShader->setVec3("cam_pos", m_spCamera->getPosition());
		m_pDrawShader->setVec3("particleColor", m_vec3ParticleColor);
		m_pDrawShader->setFloat("time", m_fTime);

		auto modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -2.5f, 0.0f));
		m_pDrawShader->setMat4("modelMatrix", modelMatrix);

		auto& tm = TextureManager::GetInstance();

		glBindVertexArray(m_uiVao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tm.GetTexture(WATERFOUNTAIN_KEY)[0]->getId());
		glDrawArrays(GL_POINTS, 0, count());

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		m_pDrawShader->deactivate();

		glDisable(GL_BLEND);
		// 上面禁用深度测试后 此时一定要开启深度测试 否则头盔(模型)渲染不出来
		glEnable(GL_DEPTH_TEST);     
	}

	unsigned int WaterFountainParticleSystem::total() const noexcept
	{
		return tot;
	}

	void WaterFountainParticleSystem::SetCamera(std::shared_ptr<Camera> spCamera)
	{
		m_spCamera = spCamera;
	}
}

