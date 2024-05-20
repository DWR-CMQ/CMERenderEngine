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
		if (compute_shader == nullptr)
		{
			compute_shader = new Shader(Cme::ShaderPath("assets//shaders//water_fountain_scene.comp"));
		}
		if (draw_shader == nullptr)
		{
			draw_shader = new Shader(Cme::ShaderPath("assets//shaders//water_fountain_scene.vert"),
									 Cme::ShaderPath("assets//shaders//water_fountain_scene.frag"),
									 Cme::ShaderPath("assets//shaders//water_fountain_scene.geom"));
		}

		glGenVertexArrays(1, &m_uiVao);
		glGenBuffers(1, &m_uiVbo);

		draw_shader->activate();
		// draw_shader->bind("GlobalAttributes", 0);
		draw_shader->setInt("sprite", 0);
		glBindVertexArray(m_uiVao);
		glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
		glEnableVertexAttribArray(0);   // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(0));
		glEnableVertexAttribArray(1);   // color alpha
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);   // delta position
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float)));
		draw_shader->deactivate();

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
		Update(-1.f);
		m_uiParticleCount = 0;

		upload = false;
	}

	void WaterFountainParticleSystem::Update(float dt, glm::vec3* cam_pos)
	{
		if (m_uiParticleCount != total())
		{
			debt += std::min(0.05f, dt) * birth_rate;
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
		compute_shader->activate();
		compute_shader->setFloat("dt", dt);
		compute_shader->setVec3("acceleration", acceleration);
		compute_shader->setInt("n_particles", static_cast<int>(count()));
		// 此处报错
		glDispatchCompute(256, 1, 1);
		compute_shader->deactivate();
	}

	void WaterFountainParticleSystem::Render(GLenum gl_draw_mode)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		draw_shader->activate();
		draw_shader->setVec3("baseColor", glm::vec3(0.0f, 1.0f, 0.0f));
		draw_shader->setMat4("view", m_spCamera->getViewTransform());
		draw_shader->setMat4("proj", m_spCamera->getProjectionTransform());
		draw_shader->setVec3("cam_pos", m_spCamera->getPosition());
		draw_shader->setVec3("particleColor", m_vec3ParticleColor);

		auto modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -2.5f, 0.0f));
		draw_shader->setMat4("modelMatrix", modelMatrix);

		auto& tm = TextureManager::GetInstance();

		glBindVertexArray(m_uiVao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tm.GetTexture(WATERFOUNTAIN_KEY)[0]->getId());
		glDrawArrays(GL_POINTS, 0, count());

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		draw_shader->deactivate();

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

