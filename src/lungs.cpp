/*
 * lungs.cpp  —  Full LungSimulation implementation
 *
 * Algorithms:
 *   Bezier curves      (Ch.4) — trachea / bronchi arcs
 *   Catmull-Rom spline (Ch.4) — organic lung outline
 *   Scan-line fill     (Ch.2) — GL_TRIANGLE_FAN polygon fill
 *   2D Transforms      (Ch.3) — mirror + scale, left vs right lung
 *   Gouraud Shading    (Ch.6) — highlight/shadow layering
 *   Key-frame Anim     (Ch.7) — damage stage progression
 *   Direct-motion      (Ch.7.4.1) — sinusoidal breathing
 *
 * Author: Sanskriti Adhikari (081BCT075)
 * Tribhuvan University – Organ Damage Visualization
 */

#include "lungs.h"
#include "geometry.h"
#include "lighting.h"
#include "text.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

/* ─────────────────────────────────────────────────────────────
   COLOUR TABLES  (index 0=healthy … 5=critical)
───────────────────────────────────────────────────────────── */
static const Col3 LC[6]={
    {0.82f,0.18f,0.20f},{0.72f,0.17f,0.18f},{0.52f,0.20f,0.12f},
    {0.36f,0.15f,0.09f},{0.20f,0.09f,0.06f},{0.08f,0.04f,0.03f}};
static const Col3 LH[6]={
    {0.98f,0.55f,0.55f},{0.88f,0.42f,0.38f},{0.65f,0.30f,0.20f},
    {0.40f,0.16f,0.11f},{0.22f,0.10f,0.07f},{0.10f,0.05f,0.04f}};
static const Col3 LS[6]={
    {0.52f,0.08f,0.09f},{0.44f,0.07f,0.07f},{0.30f,0.08f,0.05f},
    {0.18f,0.05f,0.04f},{0.10f,0.03f,0.02f},{0.04f,0.02f,0.01f}};
static const Col3 BC[6]={
    {0.92f,0.90f,0.87f},{0.80f,0.77f,0.73f},{0.60f,0.56f,0.50f},
    {0.40f,0.34f,0.26f},{0.24f,0.18f,0.13f},{0.14f,0.10f,0.07f}};
static const Col3 SC[6]={
    {0.15f,0.65f,0.22f},{0.75f,0.65f,0.05f},{0.82f,0.45f,0.05f},
    {0.80f,0.22f,0.06f},{0.72f,0.10f,0.07f},{0.38f,0.04f,0.04f}};

/* ── Colour helpers ─────────────────────────────────────────── */
Col3 LungSimulation::lungFill()   const { int s=anim.stage; return s>=5?LC[5]:R_lerp(LC[s],LC[s+1],anim.stageFrac()); }
Col3 LungSimulation::lungHigh()   const { int s=anim.stage; return s>=5?LH[5]:R_lerp(LH[s],LH[s+1],anim.stageFrac()); }
Col3 LungSimulation::lungShadow() const { int s=anim.stage; return s>=5?LS[5]:R_lerp(LS[s],LS[s+1],anim.stageFrac()); }
Col3 LungSimulation::bronchi()    const { int s=anim.stage; return s>=5?BC[5]:R_lerp(BC[s],BC[s+1],anim.stageFrac()); }
Col3 LungSimulation::stageCol()   const { return SC[anim.stage<6?anim.stage:5]; }

/* ── Lifecycle ──────────────────────────────────────────────── */
void LungSimulation::reset(){ anim.reset(); smoke.reset(); }

void LungSimulation::update(float dt){
    anim.update(dt, LUNG_STAGE_COUNT);
    if(anim.stage>0){
        float ex=cigX+0.42f*cigScale+0.020f;
        float ey=cigY-0.060f*cigScale*0.5f;
        smoke.spawn(ex,ey,anim.stage);
    }
    smoke.update(dt);
}
void LungSimulation::onKeyRight(){ if(anim.stage<LUNG_STAGE_COUNT-1){anim.stage++;anim.stageTimer=0;} }
void LungSimulation::onKeyLeft() { if(anim.stage>0){anim.stage--;anim.stageTimer=0;} }
void LungSimulation::toggleAuto(){ anim.autoPlay=!anim.autoPlay; }

/* ── Render ─────────────────────────────────────────────────── */
void LungSimulation::render(){
    R_uploadOrtho();
    drawBackground();
    drawTrachea(0.f,0.05f,0.82f);
    drawLung(-0.37f,0.05f,0.82f,-1);
    drawLung( 0.37f,0.05f,0.82f, 1);
    if(anim.stage>0){ smoke.draw(); drawCigarette(); }
    drawStagePanel();
    drawBottomBar();
    drawAlgorithmBar();
}

/* ── Background ─────────────────────────────────────────────── */
void LungSimulation::drawBackground(){
    /* Dark bg */
    R_col(0.07f,0.07f,0.10f);
    R_filledRect(-2.f,-1.f,2.f,1.f);

    /* Title bar */
    Col3 sc=stageCol();
    R_col(sc.r*0.22f,sc.g*0.22f,sc.b*0.22f);
    R_filledRect(-2.f,0.87f,2.f,1.f);
    R_col(sc.r*0.55f,sc.g*0.55f,sc.b*0.55f);
    R_filledRect(-2.f,0.855f,2.f,0.875f);

    /* Title text */
    R_col(1.f,0.96f,0.90f);
    T_drawCentred(0.f, 0.892f, 0.055f, "LUNG DAMAGE DUE TO SMOKING");

    /* Body silhouette */
    R_col(0.11f,0.10f,0.13f);
    R_filledEllipse(0.f,-0.05f,0.72f,0.80f,80);

    /* Rib cage — FEWER ribs (5 instead of 8) and more subtle */
    R_col(0.16f,0.15f,0.18f);
    glLineWidth(0.8f);
    for(int i=0;i<5;i++){
        float ry=0.30f-i*0.13f;
        float rw=0.54f-fabsf(ry)*0.22f;
        R_outlineEllipse(0.f,ry,rw,0.042f,32);
    }
    glLineWidth(1.f);
}

/* ── Lung shape ─────────────────────────────────────────────── */
std::vector<float> LungSimulation::buildLungFan(
    float cx,float cy,float sc,int side,float breath)
{
    float m=(float)side;
    std::vector<G_Point> kp={
        { 0.03f, 0.30f},{ 0.04f, 0.42f},{ 0.13f, 0.47f},
        { 0.24f, 0.43f},{ 0.35f, 0.34f},{ 0.41f, 0.16f},
        { 0.43f,-0.02f},{ 0.40f,-0.19f},{ 0.34f,-0.35f},
        { 0.20f,-0.45f},{ 0.04f,-0.47f},{-0.06f,-0.41f},
        {-0.10f,-0.25f},{-0.12f,-0.07f},{-0.08f, 0.10f},
        { 0.00f, 0.24f},{ 0.03f, 0.30f},
    };
    if(side==-1){kp[12]={-0.14f,-0.06f};kp[13]={-0.16f,0.08f};kp[14]={-0.10f,0.18f};}

    auto base=G_catmullRomFan(kp,0.16f*m,-0.04f,14);
    std::vector<float> out;
    out.push_back(cx+(0.16f*m)*sc*breath);
    out.push_back(cy+(-0.04f)*sc*breath);
    for(int i=2;i<(int)base.size();i+=2){
        out.push_back(cx+base[i]  *m*sc*breath);
        out.push_back(cy+base[i+1] *sc*breath);
    }
    return out;
}

/* ── Draw one lung ──────────────────────────────────────────── */
void LungSimulation::drawLung(float cx,float cy,float sc,int side){
    Col3 lc=lungFill(),hc=lungHigh(),shc=lungShadow(),vc=bronchi();
    float m=(float)side, b=anim.breathScale();

    auto sf=buildLungFan(cx,cy,sc*1.045f,side,b);
    R_col(L_shadow(shc,0.55f)); R_draw(GL_TRIANGLE_FAN,sf);

    auto mf=buildLungFan(cx,cy,sc,side,b);
    R_col(lc); R_draw(GL_TRIANGLE_FAN,mf);

    auto inf=buildLungFan(cx,cy,sc*0.76f,side,b);
    R_col(lc.r*1.20f,lc.g*1.20f,lc.b*1.20f,0.50f);
    R_draw(GL_TRIANGLE_FAN,inf);

    R_col(hc,0.48f);
    R_filledEllipse(cx+0.07f*m*sc,cy+0.27f*sc,0.09f*sc,0.13f*sc,32);

    R_col(hc.r*0.72f,hc.g*0.72f,hc.b*0.72f,0.22f);
    R_filledEllipse(cx+0.31f*m*sc,cy+0.07f*sc,0.07f*sc,0.11f*sc,32);

    if(anim.stage>=2){
        float sev=(anim.stage-2+anim.stageFrac())/4.f;
        int spots=(int)(sev*28); float seed=0.137f;
        for(int i=0;i<spots;i++){
            seed=fmodf(seed*1613.97f+0.07f,1.f);
            float spx=(seed-0.42f)*0.68f;
            seed=fmodf(seed*1217.13f+0.31f,1.f);
            float spy=(seed-0.48f)*0.72f;
            seed=fmodf(seed*1009.07f+0.19f,1.f);
            float spr=0.020f+seed*0.040f, dk=0.030f+sev*0.065f;
            R_col(dk,dk*0.48f,dk*0.38f,0.88f);
            R_filledEllipse(cx+spx*sc,cy+spy*sc,spr*sc,spr*0.82f*sc,14);
        }
    }

    std::vector<float> outline(mf.begin()+2,mf.end()-2);
    R_col(shc.r*0.60f,shc.g*0.60f,shc.b*0.60f);
    glLineWidth(2.6f); R_draw(GL_LINE_LOOP,outline); glLineWidth(1.f);

    R_col(vc);
    glLineWidth(3.5f);
    float pb[]={cx+0.03f*m*sc,cy+0.23f*sc,cx+0.07f*m*sc,cy+0.11f*sc};
    R_lineStrip(pb,2);
    glLineWidth(2.8f);
    float dk2[]={cx+0.07f*m*sc,cy+0.11f*sc,cx+0.14f*m*sc,cy-0.06f*sc,
                 cx+0.12f*m*sc,cy-0.26f*sc,cx+0.10f*m*sc,cy-0.39f*sc};
    R_lineStrip(dk2,4);
    glLineWidth(2.2f);
    float ub[]={cx+0.07f*m*sc,cy+0.11f*sc,cx+0.16f*m*sc,cy+0.23f*sc,
                cx+0.20f*m*sc,cy+0.35f*sc,cx+0.18f*m*sc,cy+0.43f*sc};
    R_lineStrip(ub,4);
    glLineWidth(2.0f);
    float mb[]={cx+0.14f*m*sc,cy-0.06f*sc,cx+0.26f*m*sc,cy-0.02f*sc,cx+0.34f*m*sc,cy-0.11f*sc};
    R_lineStrip(mb,3);
    glLineWidth(1.4f);
    float sb1[]={cx+0.16f*m*sc,cy+0.23f*sc,cx+0.28f*m*sc,cy+0.21f*sc,cx+0.36f*m*sc,cy+0.10f*sc};
    R_lineStrip(sb1,3);
    float sb2[]={cx+0.20f*m*sc,cy+0.35f*sc,cx+0.29f*m*sc,cy+0.37f*sc}; R_lineStrip(sb2,2);
    float sb3[]={cx+0.26f*m*sc,cy-0.02f*sc,cx+0.33f*m*sc,cy+0.07f*sc}; R_lineStrip(sb3,2);
    float sb4[]={cx+0.12f*m*sc,cy-0.26f*sc,cx+0.22f*m*sc,cy-0.31f*sc}; R_lineStrip(sb4,2);
    float sb5[]={cx+0.10f*m*sc,cy-0.39f*sc,cx+0.19f*m*sc,cy-0.41f*sc}; R_lineStrip(sb5,2);
    float sb6[]={cx+0.10f*m*sc,cy-0.39f*sc,cx+0.12f*m*sc,cy-0.45f*sc}; R_lineStrip(sb6,2);
    glLineWidth(1.f);
}

/* ── Trachea ─────────────────────────────────────────────────── */
void LungSimulation::drawTrachea(float cx,float cy,float sc){
    Col3 vc=bronchi();
    float tw=0.038f*sc, ttop=cy+0.80f*sc, tbot=cy+0.30f*sc;
    R_col(vc.r*0.52f,vc.g*0.52f,vc.b*0.52f);
    R_filledRect(cx-tw,tbot,cx+tw,ttop);
    R_col(vc); glLineWidth(2.8f);
    for(int i=0;i<8;i++){
        float ry=ttop-(ttop-tbot)*(float)i/7.f;
        float r[]={cx-tw,ry,cx+tw,ry}; R_draw(GL_LINES,r,2);
    }
    R_col(vc.r*0.38f,vc.g*0.38f,vc.b*0.38f); glLineWidth(2.f);
    float lw[]={cx-tw,ttop,cx-tw,tbot},rw[]={cx+tw,ttop,cx+tw,tbot};
    R_lineStrip(lw,2); R_lineStrip(rw,2);
    float lhx=cx-0.37f*sc,rhx=cx+0.37f*sc,hilY=cy+0.23f*sc;
    auto lbez=G_bezier(cx,tbot,cx-0.05f*sc,tbot-0.09f*sc,lhx+0.08f*sc,hilY+0.07f*sc,lhx,hilY);
    auto rbez=G_bezier(cx,tbot,cx+0.05f*sc,tbot-0.09f*sc,rhx-0.08f*sc,hilY+0.07f*sc,rhx,hilY);
    glLineWidth(9.f); R_col(vc.r*0.38f,vc.g*0.38f,vc.b*0.38f);
    R_lineStrip(lbez); R_lineStrip(rbez);
    glLineWidth(6.f); R_col(vc.r*0.62f,vc.g*0.62f,vc.b*0.62f);
    R_lineStrip(lbez); R_lineStrip(rbez);
    glLineWidth(2.5f); R_col(vc);
    R_lineStrip(lbez); R_lineStrip(rbez);
    glLineWidth(1.f);
}

/* ── Cigarette ──────────────────────────────────────────────── */
void LungSimulation::drawCigarette(){
    if(anim.stage==0) return;
    float x=cigX,y=cigY,s=cigScale;
    float len=0.42f*s,h=0.060f*s,flt=0.10f*s;
    R_col(0,0,0,0.40f);
    R_filledRect(x-flt+0.007f,y-h-0.007f,x+len+0.007f,y+0.007f);
    R_col(0.97f,0.96f,0.91f); R_filledRect(x,y-h,x+len,y);
    R_col(0.90f,0.50f,0.12f); R_filledRect(x-flt,y-h,x,y);
    R_col(0.70f,0.34f,0.06f);
    R_filledRect(x-flt,y-h,x-flt+0.011f,y);
    R_filledRect(x-0.011f,y-h,x,y);
    R_col(0.72f,0.70f,0.67f);
    float ash[]={x+len,y-h*0.08f,x+len+0.050f,y-h*0.50f,x+len,y+h*0.08f};
    R_draw(GL_TRIANGLE_FAN,ash,3);
    float pulse=0.55f+0.45f*sinf(anim.totalTime*5.5f);
    R_col(1.f,0.28f,0.f,pulse);
    R_filledEllipse(x+len+0.021f,y-h*0.50f,0.022f,0.022f,20);
    R_col(1.f,0.72f,0.f,pulse*0.42f);
    R_filledEllipse(x+len+0.021f,y-h*0.50f,0.040f,0.040f,20);
    R_col(0.38f,0.34f,0.28f); glLineWidth(1.8f);
    R_outlineRect(x-flt,y-h,x+len,y); glLineWidth(1.f);
}

/* ── Stage panel (right side) with real text ────────────────── */
void LungSimulation::drawStagePanel(){
    float panelW=0.50f, panelX=1.52f-panelW;
    float panelTop=0.855f, panelBot=-0.68f;
    float itemH=(panelTop-panelBot)/(float)LUNG_STAGE_COUNT;
    Col3 sc=stageCol();

    /* Panel background */
    R_col(0.07f,0.06f,0.10f,0.95f);
    R_filledRect(panelX,panelBot,panelX+panelW,panelTop);
    R_col(0.25f,0.22f,0.32f); glLineWidth(1.5f);
    R_outlineRect(panelX,panelBot,panelX+panelW,panelTop);
    glLineWidth(1.f);

    /* "STAGE PROGRESS" header */
    R_col(sc.r*0.30f,sc.g*0.30f,sc.b*0.30f);
    R_filledRect(panelX,panelTop-0.068f,panelX+panelW,panelTop);
    R_col(sc); glLineWidth(1.5f);
    R_filledRect(panelX,panelTop-0.070f,panelX+panelW,panelTop-0.067f);
    glLineWidth(1.f);
    R_col(1.f,1.f,1.f);
    T_drawCentred(panelX+panelW*0.5f, panelTop-0.058f, 0.030f, "STAGE PROGRESS");

    /* Stage rows */
    for(int i=0;i<LUNG_STAGE_COUNT;i++){
        const StageInfo& si=LUNG_STAGES[i];
        float rowTop=panelTop-0.068f-i*itemH;
        float rowBot=rowTop-itemH+0.005f;
        bool isActive=(i==anim.stage), isDone=(i<anim.stage);

        /* Row background */
        if(isActive)     R_col(sc.r*0.25f,sc.g*0.25f,sc.b*0.25f,0.95f);
        else if(isDone)  R_col(0.12f,0.11f,0.15f,0.85f);
        else             R_col(0.09f,0.08f,0.11f,0.65f);
        R_filledRect(panelX+0.006f,rowBot,panelX+panelW-0.006f,rowTop);

        /* Active border */
        if(isActive){
            R_col(sc); glLineWidth(2.f);
            R_outlineRect(panelX+0.006f,rowBot,panelX+panelW-0.006f,rowTop);
            /* Arrow */
            float amid=(rowTop+rowBot)*0.5f;
            float arr[]={panelX+panelW-0.035f,amid+0.018f,
                         panelX+panelW-0.010f,amid,
                         panelX+panelW-0.035f,amid-0.018f};
            R_draw(GL_TRIANGLE_FAN,arr,3);
            glLineWidth(1.f);
        }

        /* Colour accent strip on left */
        Col3 rc=SC[i];
        float inten=isActive?1.f:(isDone?0.55f:0.22f);
        R_col(rc.r*inten,rc.g*inten,rc.b*inten);
        R_filledRect(panelX+0.006f,rowBot,panelX+0.020f,rowTop);

        float tx=panelX+0.026f;
        float alpha=isActive?1.f:(isDone?0.70f:0.38f);

        /* Text layout tuned to reduce crowding inside each stage row */
        float labelSz=0.024f;
        float timeSz=0.020f;
        float condSz=0.019f;
        float labelY=rowTop-labelSz-0.010f;
        float timeY=labelY-timeSz-0.006f;
        float condY=timeY-condSz-0.007f;

        /* Stage label: "Stage 1" */
        R_col(1.f*alpha,0.96f*alpha,0.88f*alpha);
        T_drawText(tx, labelY, labelSz, si.label);


        /* Condition text below */
        R_col(0.72f*alpha,0.70f*alpha,0.78f*alpha);
        T_drawText(tx, condY, condSz, si.condition);

        /* Progress bar for active row */
        if(isActive){
            float prog=anim.stageFrac();
            float bx1=tx, bx2=panelX+panelW-0.040f;
            float by=rowBot+0.010f, bh=0.012f;
            R_col(0.12f,0.11f,0.16f);
            R_filledRect(bx1,by,bx2,by+bh);
            R_col(sc,0.85f);
            R_filledRect(bx1,by,bx1+(bx2-bx1)*prog,by+bh);
            glLineWidth(1.f);
            R_col(sc.r*0.6f,sc.g*0.6f,sc.b*0.6f);
            R_outlineRect(bx1,by,bx2,by+bh);
        }

        /* Row separator */
        R_col(0.20f,0.18f,0.24f);
        float sep[]={panelX+0.006f,rowBot,panelX+panelW-0.006f,rowBot};
        R_draw(GL_LINES,sep,2);
    }
}

/* ── Bottom bar with real text ──────────────────────────────── */
void LungSimulation::drawBottomBar(){
    const StageInfo& si=LUNG_STAGES[anim.stage<LUNG_STAGE_COUNT?anim.stage:LUNG_STAGE_COUNT-1];
    Col3 sc=stageCol();

    float bx1=-1.55f,bx2=1.00f,by1=-0.99f,by2=-0.70f;
    R_col(0.08f,0.07f,0.11f,0.97f);
    R_filledRect(bx1,by1,bx2,by2);
    R_col(sc.r*0.28f,sc.g*0.28f,sc.b*0.28f); glLineWidth(1.5f);
    R_outlineRect(bx1,by1,bx2,by2); glLineWidth(1.f);

    /* Stage + time heading */
    char heading[64];
    snprintf(heading,sizeof(heading),"%s  |  %s",si.label,si.timeLabel);
    R_col(1.f,0.95f,0.80f);
    T_drawText(bx1+0.04f, by2-0.042f, 0.036f, heading);

    /* "Damage Spectrum:" label */
    R_col(0.65f,0.62f,0.72f);
    T_drawText(bx1+0.04f, by2-0.075f, 0.024f, "Damage Spectrum:");

    /* Gradient bar */
    float barX1=bx1+0.04f, barX2=bx2-0.04f;
    float barY=by2-0.110f, barH=0.022f;
    R_col(0.12f,0.11f,0.15f);
    R_filledRect(barX1,barY,barX2,barY+barH);
    float total=anim.totalFrac(LUNG_STAGE_COUNT);
    float bw=barX2-barX1;
    for(int i=0;i<40;i++){
        float t0=(float)i/40,t1=(float)(i+1)/40;
        if(t0>total) break;
        float tU=t1>total?total:t1;
        R_col(t0,1.f-t0,0.1f,0.88f);
        R_filledRect(barX1+bw*t0,barY,barX1+bw*tU,barY+barH);
    }
    float px=barX1+bw*total;
    R_col(1,1,1); R_filledRect(px-0.007f,barY-0.005f,px+0.007f,barY+barH+0.005f);

    glLineWidth(1.2f);
    R_col(0.32f,0.30f,0.42f);
    R_outlineRect(barX1,barY,barX2,barY+barH);
    glLineWidth(1.f);
}

/* ── Algorithm bar with real text ───────────────────────────── */
void LungSimulation::drawAlgorithmBar(){
    float y=-0.70f, h=-0.68f;
    R_col(0.08f,0.07f,0.12f,0.95f);
    R_filledRect(-1.58f,y,1.02f,h);

    Col3 sc=stageCol();
    R_col(sc.r*0.45f,sc.g*0.45f,sc.b*0.45f);

}