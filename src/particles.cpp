#include "particles.h"
#include "rbsolver.h"
#include "glm1.h"

#include <algorithm>

/// Particle

bool Particle::step(float deltaT)
{
    for (int i = 0; i != 3; ++i)
    {
        position[i] += velocity[i] * deltaT;
    }
    float deccelerationSize = 1.f;

    glm::vec3 decceleration(velocity[0], velocity[1], velocity[2]);
    glm::normalize(decceleration);
    for (int i = 0; i != 3; ++i)
    {
        decceleration[i] *= deccelerationSize * deltaT;
        velocity[i] -= decceleration[i];
    }
    radius += radiusIncreaseSpeed * deltaT;
    if (position[1] < radius)
        position[1] = radius;
    density -= densityDecreaseSpeed * deltaT;
    return density > 0.f;
}

/// Particles

Particles::Particles()
{
    const int initialAllocationSize = 50;
    m_particleContainer.reserve(initialAllocationSize);
}

void Particles::step(float deltaT)
{
    for (unsigned i = 0; i < m_particleContainer.size(); ++i)
    {
        Particle& particle = m_particleContainer[i];
        bool existsAfterStep = particle.step(deltaT);
        if (!existsAfterStep)
        {
            if (m_particleContainer.size() > 1)
            {
                std::swap(particle, m_particleContainer.back());
            }
            m_particleContainer.pop_back();
        }
    }
    ++m_stepCounter;
}

void Particles::newParticle(const Particle& particle)
{
    m_particleContainer.push_back(particle);
}
