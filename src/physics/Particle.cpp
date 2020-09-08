#include "Particle.h"

Particle::Particle(const glm::vec3 &position) : m_position{position}, m_old_position{position} {

}

void Particle::offset_pos(const glm::vec3 &v) {
    if (m_is_movable) {
        m_position += v;
    }
}

void Particle::set_movable(bool movable) {
    m_is_movable = movable;
}

void Particle::add_force(const glm::vec3& force) {
    m_acceleration += force / m_mass;
}

void Particle::update(float dt) {
    if (m_is_movable) {
        glm::vec3 old_position = m_position;
        m_position = m_position + (m_position - m_old_position) * (1.0f - m_damping) + m_acceleration * dt;
        m_old_position = old_position;
        m_acceleration = glm::vec3(0, 0, 0);
    }
}
