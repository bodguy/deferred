#ifndef DEFERRED_CLOTH_H
#define DEFERRED_CLOTH_H

#include <vector>
#include "Particle.h"

class Cloth {
public:
  Cloth(int w, int h);

private:
  int m_width, m_height;
  bool m_enabled, m_use_gravity;
  float m_cloth_solver_freq, m_damping;
  std::vector<Particle> m_particles;
};

#endif //DEFERRED_CLOTH_H
