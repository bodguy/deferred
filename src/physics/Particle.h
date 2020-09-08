#ifndef DEFERRED_PARTICLE_H
#define DEFERRED_PARTICLE_H

#include <glm/gtc/type_ptr.hpp>

class Particle {
public:
  Particle() = default;
  explicit Particle(const glm::vec3& position);

  glm::vec3 get_position() const { return m_position; }

  void offset_pos(const glm::vec3& v);
  void set_movable(bool movable);

  void add_force(const glm::vec3& force);
  void update(float dt);

private:
  glm::vec3 m_position{}, m_old_position{}, m_acceleration = glm::vec3(0, 0, 0);
  float m_mass = 1.f, m_damping = 0.01f;
  bool m_is_movable = true;
};

#endif //DEFERRED_PARTICLE_H
