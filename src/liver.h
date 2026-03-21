// /*
//  * liver.h  —  LiverSimulation stub
//  * To be implemented by Tejaswi Acharya (081BCT088).
//  *
//  * Instructions:
//  *   1. Implement all methods in liver.cpp
//  *   2. Recompile — main.cpp already routes to this class
//  *
//  * Tribhuvan University – Organ Damage Visualization
//  */
// #pragma once

/*
 * liver.h  —  LiverSimulation class declaration
 *
 * Author: Sanskriti Adhikari (081BCT075)
 * Tribhuvan University – Organ Damage Visualization
 *
 * Mirrors the structure of lungs.h exactly.
 * Same dependencies: renderer.h, animation.h, particles.h
 * Damage cause: alcohol (bottle replaces cigarette)
 */
/*
 * liver.h  —  LiverSimulation class declaration
 *
 * Author: Sanskriti Adhikari (081BCT075)
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once

#include "renderer.h"    /* Col3, R_* helpers — must come first */
#include "animation.h"   /* LUNG_STAGE_COUNT, StageInfo, LUNG_STAGES */
#include "particles.h"   /* AnimState, ParticleSystem */
#include <vector>

class LiverSimulation {
public:
    AnimState      anim;
    ParticleSystem vapor;

    /* Bottle position — tilted bottle bottom-left of scene */
    float bottleX     = -1.10f;
    float bottleY     = -0.55f;
    float bottleScale =  1.00f;

    void reset();
    void update(float dt);
    void render();
    void onKeyRight();
    void onKeyLeft();
    void toggleAuto();

private:
    /* Colour helpers */
    Col3 liverFill()   const;
    Col3 liverHigh()   const;
    Col3 liverShadow() const;
    Col3 vessel()      const;
    Col3 stageCol()    const;

    /* Scene layers — called in order from render() */
    void drawBackground();
    void drawVessels();      /* IVC + artery — drawn before liver   */
    void drawLiver();        /* organ body + shading + damage spots  */
    void drawBottle();       /* tilted green bottle                  */
    void drawAlcoholDrip();  /* pour stream + falling drops + splash */
    void drawStagePanel();
    void drawBottomBar();
    void drawAlgorithmBar();

    /* Stub — polar contour used instead, kept for linker */
    std::vector<float> buildLiverFan(float cx, float cy,
                                     float sc, float breath);
};