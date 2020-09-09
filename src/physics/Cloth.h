#ifndef DEFERRED_CLOTH_H
#define DEFERRED_CLOTH_H

#include <vector>
#include "Particle.h"
#include "Constraint.h"

class Cloth {
public:
  Cloth(int w, int h);
  ~Cloth();

  void initVertex();
  void add_wind_force(const glm::vec3& direction);

  void render();
  void update(float dt);

  Particle* get_particle(int x, int y);

private:
  glm::vec3 calc_triangle_normal(Particle* p1, Particle* p2, Particle* p3) const;
  void add_wind_force_for_triangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3& direction);

private:
  int m_width, m_height;
  bool m_enabled = true, m_use_gravity = true;
  int m_cloth_solver_freq = 15;
  std::vector<Particle> m_particles;
  std::vector<Constraint> m_constraint;
  unsigned int vao = 0, vbo = 0;
};

#endif //DEFERRED_CLOTH_H
