#include "Cloth.h"
#include <glm/gtc/type_ptr.hpp>

Cloth::Cloth(int w, int h)
    : m_width(w), m_height(h), m_enabled(true), m_use_gravity(true), m_cloth_solver_freq(15.f), m_damping(0.f) {
    m_particles.resize(m_width * m_height);

    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            glm::vec3 position = glm::vec3(1.f * (x / (float)m_width), -1.f * (y / (float)m_height), 0.f);
            m_particles[y * m_width + x] = Particle(position);
        }
    }
}