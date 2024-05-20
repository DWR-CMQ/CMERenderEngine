#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "base_particle.h"
#include "../shader/shader.h"
#include "../camera.h"

namespace Cme
{
	class WaterFountainParticleSystem : public BaseParticle
	{
    public:
        static const std::string WATERFOUNTAIN_KEY;

        WaterFountainParticleSystem();
        ~WaterFountainParticleSystem();

        glm::vec3 acceleration;

        void InitPS(float* vertex, unsigned int v_count,
                    unsigned int tex = 0, float* uv = nullptr, bool atlas = false) override;
        void Restart() override;
        void Upload() override;
        void Update(float dt, glm::vec3* cam_pos = nullptr) override;
        void Render(GLenum gl_draw_mode = GL_POINT) override;

        void SetParticleColor(glm::vec3 Color)
        {
            m_vec3ParticleColor = Color;
        };

        unsigned int total() const noexcept override;
        void SetCamera(std::shared_ptr<Camera> spCamera);

    private:
        //std::unique_ptr<WaterFountainPSImpl> m_upImpl;
        unsigned int m_uiVao;
        unsigned int m_uiVbo;

        float debt;
        unsigned int tot;
        bool upload;

        Shader* compute_shader = nullptr;
        Shader* draw_shader = nullptr;

        glm::vec3 m_vec3ParticleColor = glm::vec3(0.0f, 1.0f, 0.0f);
        std::shared_ptr<Camera> m_spCamera;
	};
}


