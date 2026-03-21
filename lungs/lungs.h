/*
 * lungs.h  —  LungSimulation class declaration
 *
 * Author: Sanskriti Adhikari (081BCT075)
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once

/* renderer.h MUST come first — it defines Col3 */
#include "renderer.h"
#include "animation.h"
#include "particles.h"
#include <vector>

class LungSimulation {
public:
    AnimState      anim;
    ParticleSystem smoke;

    float cigX     = -1.42f;
    float cigY     = -0.60f;
    float cigScale =  1.10f;

    void reset();
    void update(float dt);
    void render();

    void onKeyRight();
    void onKeyLeft();
    void toggleAuto();

private:
    /* -- Colour helpers (interpolate between stage tables) --- */
    Col3 lungFill()   const;
    Col3 lungHigh()   const;
    Col3 lungShadow() const;
    Col3 bronchi()    const;
    Col3 stageCol()   const;

    /* -- Scene layers ---------------------------------------- */
    void drawBackground();
    void drawTrachea(float cx, float cy, float sc);
    void drawLung(float cx, float cy, float sc, int side);
    void drawCigarette();
    void drawStagePanel();
    void drawBottomBar();
    void drawAlgorithmBar();

    /* -- Geometry -------------------------------------------- */
    std::vector<float> buildLungFan(float cx, float cy, float sc,
                                    int side, float breath);
};