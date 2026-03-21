/*
 * animation.h  —  Stage timing, breathing, keyframe state
 *
 * Algorithms:
 *   Key-frame systems   (Ch.7.3)
 *   Direct-motion spec  (Ch.7.4.1) — sinusoidal breathing
 *
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once
#include <cmath>

struct StageInfo {
    int         stage;
    const char* label;
    const char* timeLabel;
    const char* condition;
    const char* description;
};

static const StageInfo LUNG_STAGES[] = {
    {0,"Stage 1","1 Month",   "Healthy Lungs",
     "Cilia begin to lose function. Mild irritation."},
    {1,"Stage 2","6 Months",  "Early Irritation",
     "Chronic cough develops. Airway inflammation increases."},
    {2,"Stage 3","1 Year",    "Tissue Darkening",
     "Tar deposits form. Lung capacity begins to drop."},
    {3,"Stage 4","5 Years",   "Moderate Damage",
     "COPD risk rises. Significant tar build-up visible."},
    {4,"Stage 5","10 Years",  "Severe Damage",
     "Emphysema develops. Alveoli permanently destroyed."},
    {5,"Stage 6","20+ Years", "Critical / End-Stage",
     "Lung cancer risk very high. Minimal viable tissue."},
};
static const int LUNG_STAGE_COUNT = 6;

struct AnimState {
    int   stage       = 0;
    float stageTimer  = 0.f;
    float stageDur    = 6.f;
    bool  autoPlay    = true;
    float totalTime   = 0.f;
    float breathPhase = 0.f;

    void reset(){
        stage=0; stageTimer=0.f;
        totalTime=0.f; breathPhase=0.f; autoPlay=true;
    }

    void update(float dt, int maxStage){
        totalTime += dt;
        float spd = 0.95f - stage*0.10f;
        if(spd<0.35f) spd=0.35f;
        breathPhase += dt*spd;
        if(autoPlay && stage<maxStage-1){
            stageTimer += dt;
            if(stageTimer>=stageDur){ stageTimer=0.f; stage++; }
        }
    }

    /* Sinusoidal breathing — direct-motion (Ch.7.4.1) */
    float breathScale() const {
        float amp=0.028f*(1.f-stage*0.16f);
        if(amp<0.005f) amp=0.005f;
        return 1.f + amp*sinf(breathPhase);
    }

    float stageFrac() const { return stageTimer/stageDur; }

    float totalFrac(int maxStage) const {
        return (stage+stageFrac())/(float)maxStage;
    }
};