/*
 * particles.cpp  —  Smoke particle implementation
 * Tribhuvan University – Organ Damage Visualization
 */
#include "particles.h"
#include "renderer.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

static float rnd(){ return (float)(rand()%1000)/1000.f; }

void ParticleSystem::spawn(float ex, float ey, int stage){
    if(stage==0 || (int)particles.size()>100) return;
    int n = stage+1;
    for(int i=0;i<n;i++){
        SmokeParticle p;
        p.x    = ex+(rnd()-0.5f)*0.018f;
        p.y    = ey;
        p.vx   = (rnd()-0.5f)*0.0048f;
        p.vy   = 0.004f+rnd()*0.003f;
        p.life = 1.f;
        p.sz   = 0.018f+rnd()*0.024f;
        particles.push_back(p);
    }
}

void ParticleSystem::update(float dt){
    for(auto& p : particles){
        p.x   += p.vx;
        p.y   += p.vy;
        p.vx  += (rnd()-0.5f)*0.0008f;
        p.life -= dt*0.22f;
        p.sz   += dt*0.030f;
    }
    particles.erase(
        std::remove_if(particles.begin(),particles.end(),
            [](const SmokeParticle& p){ return p.life<=0.f; }),
        particles.end());
}

void ParticleSystem::draw() const {
    for(const auto& p : particles){
        float gr = 0.70f+(1.f-p.life)*0.20f;
        R_col(gr,gr,gr,p.life*0.38f);
        R_filledEllipse(p.x,p.y,p.sz,p.sz*1.35f,14);
    }
}