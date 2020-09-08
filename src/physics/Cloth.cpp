#include "Cloth.h"
#include <glm/gtc/type_ptr.hpp>

Cloth::Cloth(int w, int h) : m_width{w}, m_height{h} {
    m_particles.resize(m_width * m_height);

    // creating particles in a grid of particles from (0,0,0) to (width,-height,0)
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            glm::vec3 position = glm::vec3(1.f * (x / (float)m_width), -1.f * (y / (float)m_height), 0.f);
            m_particles[y * m_width + x] = Particle(position);
        }
    }

    // Connecting immediate neighbor
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            if (x < m_width - 1) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 1, y)));
            if (y < m_height - 1) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x, y + 1)));
            if (x < m_width - 1 && y < m_height - 1) {
                m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 1, y + 1)));
                m_constraint.emplace_back(Constraint(get_particle(x + 1, y), get_particle(x, y + 1)));
            }
        }
    }

    // Connecting secondary neighbors
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            if (x < m_width - 2) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 2, y)));
            if (y < m_height - 2) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x, y + 2)));
            if (x < m_width - 2 && y < m_height - 2) {
                m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 2, y + 2)));
                m_constraint.emplace_back(Constraint(get_particle(x + 2, y), get_particle(x, y + 2)));
            }
        }
    }

    // disable top 3 particle to hold cloth
    for (int i = 0; i< 3; i++) {
        get_particle(0 + i, 0)->set_movable(false);
        get_particle(m_width - 1 - i, 0)->set_movable(false);
    }


}

void Cloth::add_wind_force(const glm::vec3& direction) {
    for (int x = 0; x < m_width - 1; ++x) {
        for (int y = 0; y < m_height - 1; ++y) {
            add_wind_force_for_triangle(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1), direction);
            add_wind_force_for_triangle(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1), direction);
        }
    }
}

glm::vec3 Cloth::calc_triangle_normal(Particle *p1, Particle *p2, Particle *p3) const {
    glm::vec3 position1 = p1->get_position();
    glm::vec3 position2 = p2->get_position();
    glm::vec3 position3 = p3->get_position();

    glm::vec3 v1 = position2 - position1;
    glm::vec3 v2 = position3 - position1;

    return glm::cross(v1, v2);
}

void Cloth::add_wind_force_for_triangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3& direction) {
    glm::vec3 normal = calc_triangle_normal(p1, p2, p3);
    glm::vec3 d = glm::normalize(normal);
    glm::vec3 force = normal * glm::dot(d, direction);
    p1->add_force(force);
    p2->add_force(force);
    p3->add_force(force);
}

void Cloth::render() {

}

void Cloth::update(float dt) {
    if (!m_enabled) return;

    for (int i = 0; i < m_cloth_solver_freq; i++) {
        for (auto& constraint : m_constraint) {
            constraint.satisfy();
        }
    }

    for (auto& particle : m_particles) {
        if (m_use_gravity) {
            particle.add_force(glm::vec3(0.f, -0.1f, 0.f) * dt);
        }
        particle.update(dt);
    }
}

Particle* Cloth::get_particle(int x, int y) {
    return &m_particles[y * m_width + x];
}
