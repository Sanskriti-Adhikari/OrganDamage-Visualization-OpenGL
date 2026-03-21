/*
 * particles.h  —  Smoke particle system
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once
#include <vector>

struct SmokeParticle {
    float x, y;    /* position  */
    float vx, vy;  /* velocity  */
    float life;    /* 1=fresh, 0=dead */
    float sz;      /* radius    */
};

class ParticleSystem {
public:
    std::vector<SmokeParticle> particles;

    void reset(){ particles.clear(); }
    void spawn(float ex, float ey, int stage);
    void update(float dt);
    void draw() const;
};