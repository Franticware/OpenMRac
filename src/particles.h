#ifndef PARTICLES_H
#define PARTICLES_H

#include <vector>

struct Particle
{
    Particle(float posX, float posY, float posZ, float vX, float vY, float vZ, int texCfg, float r, float rIncSpeed, float dens, float densDecSpeed)
    {
        position[0] = posX + vX * r;
        position[1] = posY + vY * r;
        position[2] = posZ + vZ * r;
        velocity[0] = vX;
        velocity[1] = vY;
        velocity[2] = vZ;
        texConfig = texCfg;
        radius = r;
        radiusIncreaseSpeed = rIncSpeed;
        density = dens;
        densityDecreaseSpeed = densDecSpeed;
    }

    Particle()
    {
        position[0] = 0;
        position[1] = 0;
        position[2] = 0;
        velocity[0] = 0;
        velocity[1] = 0;
        velocity[2] = 0;
        texConfig = 0;
        radius = 0;
        radiusIncreaseSpeed = 1;
        density = 0;
        densityDecreaseSpeed = 1;
    }

    bool step(float deltaT); // true - po kroku existuje, false - po kroku zmizí

    float position[3]; // aktuální pozice
    float velocity[3]; // aktuální rychlost
    int texConfig; // 0, 1, 2 nebo 3 - konfigurace (natočení) textury
    float radius; // aktuální velikost (poloměr!)
    float radiusIncreaseSpeed; // lineární rychlost zvětšování
    float density; // průhlednost
    float densityDecreaseSpeed; // lineární rychlost zprůhledňování
};

class Particles
{
public:
    Particles();
    void clear()
    {
        m_particleContainer.clear();
        m_stepCounter = 0;
    }
    void step(float deltaT);
    void newParticle(const Particle& particle);

    std::vector<Particle> m_particleContainer;
    unsigned m_stepCounter;
};

#endif // PARTICLES_H
