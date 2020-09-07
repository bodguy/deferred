#include "Particle.h"

Particle::Particle() : m_position(), m_old_position() {

}

Particle::Particle(const glm::vec3 &position) : m_position(position), m_old_position(position) {

}
