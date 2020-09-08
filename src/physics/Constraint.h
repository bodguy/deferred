#ifndef DEFERRED_CONSTRAINT_H
#define DEFERRED_CONSTRAINT_H

class Particle;
class Constraint {
public:
  Constraint(Particle* p1, Particle* p2);
  void satisfy();

private:
  float m_rest_distance;
  Particle* m_p1, *m_p2;
};

#endif //DEFERRED_CONSTRAINT_H
