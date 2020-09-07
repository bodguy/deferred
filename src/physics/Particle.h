#ifndef DEFERRED_PARTICLE_H
#define DEFERRED_PARTICLE_H

#include <glm/gtc/type_ptr.hpp>

class Particle {
public:
  Particle();
  Particle(const glm::vec3& position);

private:
  glm::vec3 m_position, m_old_position;
};

#endif //DEFERRED_PARTICLE_H
