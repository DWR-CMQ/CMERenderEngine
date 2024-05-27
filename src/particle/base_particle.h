#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <functional>
#include <glad/glad.h>

namespace Cme
{
    struct ParticleProperty
    {
        glm::vec3 position;
        glm::vec4 color;
        float size, rotation;
        float u_offset, v_offset, texture_scale;

        ParticleProperty();
    };

    using ParticleSpawnFn_t = std::function<bool(ParticleProperty&, float&, int id)>;
    using ParticleUpdateFn_t = std::function<bool(ParticleProperty&, float&, float, int id)>;

    class BaseParticle
    {
    public:
        unsigned int max_particles;
        float birth_rate;

        ParticleSpawnFn_t spawn_fn;
        ParticleUpdateFn_t update_fn;

    public:
        virtual void InitPS(float* vertex, unsigned int v_count,
                            unsigned int tex = 0, float* uv = nullptr, bool atlas = false) = 0;
        virtual void ActivateTexture(unsigned int tex = 0, float* uv = nullptr, bool atlas = false) {};
        virtual void ActivateAtlas(bool atlas = false) {};
        virtual void Restart() = 0;
        virtual void Upload() {};
        virtual void Update(float fDelta, float fTime) = 0;
        virtual void Render(GLenum gl_draw_mode = GL_POINT) = 0;

        virtual unsigned int const& count() const noexcept { return m_uiParticleCount; }
        virtual unsigned int total() const noexcept { return static_cast<unsigned int>(m_vecParticles.size()); }

        void err(std::string const& msg);
        virtual ~BaseParticle() = default;
    protected:
        BaseParticle() {};

    protected:
        unsigned int m_uiParticleCount;
        std::vector<ParticleProperty> m_vecParticles;
    };
}