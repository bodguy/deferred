#include "Constraint.h"
#include "Particle.h"
#include <glm/gtc/type_ptr.hpp>

Constraint::Constraint(Particle *p1, Particle *p2) : m_p1{p1}, m_p2{p2} {
    glm::vec3 v = m_p1->get_position() - m_p2->get_position();
    m_rest_distance = glm::length(v);
}

void Constraint::satisfy() {
    glm::vec3 p1_to_p2 = m_p1->get_position() - m_p2->get_position();
    float current_distance = glm::length(p1_to_p2);
    glm::vec3 correction_vec_half = p1_to_p2 * (1.f - m_rest_distance / current_distance) * 0.5f;
    m_p1->offset_pos(correction_vec_half);
    m_p2->offset_pos(-correction_vec_half);
}