/*
 * liver.cpp  —  LiverSimulation implementation
 */
/*
 * liver.cpp  —  LiverSimulation
 *
 * Algorithms:
 *   Ch.2  GL_POLYGON scan-line fill  (liver body, damage spots)
 *   Ch.3  2D translation via CX/CY offset + manual rotation (bottle)
 *   Ch.4  Cubic Bezier pour stream   (drawAlcoholDrip)
 *   Ch.6  Gouraud radial shading     (GL_TRIANGLE_FAN colour fade)
 *   Ch.7  Key-frame stage animation, sinusoidal breathing
 *
 * Shape: polar parametric curve
 *   r(t) = 1 + 0.30·sin(t+φ) + 0.15·cos(2t) − 0.08·sin(3t+φ) + 0.05·cos(4t)
 *
 * Render order:
 *   drawVessels → drawLiver → drawBottle → drawAlcoholDrip → UI panels
 *
 * Author: Sanskriti Adhikari (081BCT075)
 * Tribhuvan University – Organ Damage Visualization
 */

#include "liver.h"
#include "geometry.h"   /* G_bezier, R_lineStrip */
#include "text.h"       /* T_drawCentred, T_drawText */
#include <cmath>
#include <cstdio>
#include <vector>

/* ── Constants ───────────────────────────────────────────────── */
static const float PI = 3.14159265f;

/* Organ centre in normalised ortho space */
static const float CX = -0.04f;
static const float CY =  0.08f;

/* ── Stage colour tables ─────────────────────────────────────── */
/* UI accent colour per stage */
static const Col3 SCA[6] = {
    {0.15f,0.65f,0.22f}, {0.75f,0.65f,0.05f}, {0.82f,0.45f,0.05f},
    {0.80f,0.22f,0.06f}, {0.72f,0.10f,0.07f}, {0.38f,0.04f,0.04f}
};
/* Liver fill colour — healthy pink → dark necrotic brown */
static const Col3 LVC[6] = {
    {0.90f,0.55f,0.55f}, {0.85f,0.42f,0.42f}, {0.78f,0.35f,0.20f},
    {0.62f,0.22f,0.12f}, {0.45f,0.15f,0.08f}, {0.28f,0.10f,0.06f}
};
/* Damage spot opacity per stage */
static const float DMG[6] = {0.00f,0.10f,0.25f,0.45f,0.65f,0.85f};

/* ── Colour helpers ──────────────────────────────────────────── */
Col3 LiverSimulation::liverFill() const {
    int s = anim.stage;
    return s >= 5 ? LVC[5] : R_lerp(LVC[s], LVC[s+1], anim.stageFrac());
}
Col3 LiverSimulation::liverHigh() const {
    Col3 c = liverFill();
    return { fminf(c.r*1.35f,1.f), fminf(c.g*1.28f,1.f), fminf(c.b*1.20f,1.f) };
}
Col3 LiverSimulation::liverShadow() const {
    Col3 c = liverFill();
    return { c.r*0.45f, c.g*0.35f, c.b*0.28f };
}
Col3 LiverSimulation::vessel()   const { return {1,1,1}; }
Col3 LiverSimulation::stageCol() const { return SCA[anim.stage < 6 ? anim.stage : 5]; }

/* ── Lifecycle ───────────────────────────────────────────────── */
void LiverSimulation::reset()      { anim.reset(); vapor.reset(); }
void LiverSimulation::onKeyRight() { if(anim.stage<LUNG_STAGE_COUNT-1){anim.stage++;anim.stageTimer=0;} }
void LiverSimulation::onKeyLeft()  { if(anim.stage>0){anim.stage--;anim.stageTimer=0;} }
void LiverSimulation::toggleAuto() { anim.autoPlay = !anim.autoPlay; }

void LiverSimulation::update(float dt) {
    anim.update(dt, LUNG_STAGE_COUNT);
    if (anim.stage > 0)
        vapor.spawn(bottleX + 0.28f*bottleScale,
                    bottleY + 0.30f*bottleScale, anim.stage);
    vapor.update(dt);
}

/* ── Render ──────────────────────────────────────────────────── */
void LiverSimulation::render() {
    R_uploadOrtho();
    drawBackground();
    drawVessels();                                    /* behind liver */
    drawLiver();                                      /* organ on top */
    if (anim.stage > 0) { drawBottle(); drawAlcoholDrip(); }
    drawStagePanel();
    drawBottomBar();
    drawAlgorithmBar();
}

/* ── Background ──────────────────────────────────────────────── */
void LiverSimulation::drawBackground() {
    R_col(0.07f, 0.07f, 0.10f);
    R_filledRect(-2.f, -1.f, 2.f, 1.f);
    Col3 sc = stageCol();
    R_col(sc.r*0.22f, sc.g*0.22f, sc.b*0.22f);
    R_filledRect(-2.f, 0.87f, 2.f, 1.f);
    R_col(sc.r*0.55f, sc.g*0.55f, sc.b*0.55f);
    R_filledRect(-2.f, 0.855f, 2.f, 0.875f);
    R_col(1.f, 0.96f, 0.90f);
    T_drawCentred(0.f, 0.892f, 0.055f, "LIVER DAMAGE DUE TO ALCOHOL");
}

/* ── Polar contour ───────────────────────────────────────────── */
/*
 * Direct port of GLUT buildLiverContour().
 * Pixel values ÷ 250 to fit the normalised ortho space.
 * PHI = -0.52 rad shifts the dominant right lobe left-of-centre.
 */
static void buildContour(float* xs, float* ys, int n, float breath = 1.f) {
    const float PHI = -0.52f;
    const float Rx  = 175.f/250.f * breath;
    const float Ry  = 105.f/250.f * breath;
    const float ox  =  12.f/250.f;
    const float oy  =   8.f/250.f;
    for (int i = 0; i < n; i++) {
        float t = (float)i / n * 2.f * PI;
        float r = 1.f
                + 0.30f*sinf(t + PHI)
                + 0.15f*cosf(2.f*t)
                - 0.08f*sinf(3.f*t + PHI)
                + 0.05f*cosf(4.f*t);
        xs[i] = CX + r*Rx*cosf(t) + ox*cosf(2.f*t);
        ys[i] = CY + r*Ry*sinf(t) - oy*sinf(2.f*t);
    }
}

/* ── Draw liver ──────────────────────────────────────────────── */
void LiverSimulation::drawLiver() {
    const int N = 120;
    float xs[N], ys[N];
    buildContour(xs, ys, N, anim.breathScale());
    Col3 c = liverFill();
    std::vector<float> v;
    v.reserve(N * 2);

    /* 1. Cast shadow */
    v.clear();
    for (int i = 0; i < N; i++) {
        v.push_back(xs[i] + 0.020f);
        v.push_back(ys[i] - 0.026f);
    }
    R_col(0.03f, 0.02f, 0.02f, 0.40f);
    R_draw(GL_TRIANGLE_FAN, v.data(), N);

    /* 2. Main body */
    v.clear();
    for (int i = 0; i < N; i++) {
        v.push_back(xs[i]);
        v.push_back(ys[i]);
    }
    R_col(c.r, c.g, c.b, 1.0f);
    R_draw(GL_TRIANGLE_FAN, v.data(), N);

    /* 3. Underside depth, kept inside contour */
    R_col(c.r*0.58f, c.g*0.42f, c.b*0.36f, 0.26f);
    R_filledEllipse(CX + 0.06f, CY - 0.05f, 0.28f, 0.09f, 44);

    /* 4. Broad dome and left-lobe highlights */
    R_col(fminf(c.r*1.18f,1.f), fminf(c.g*1.10f,1.f), fminf(c.b*1.08f,1.f), 0.32f);
    R_filledEllipse(CX - 0.06f, CY + 0.13f, 0.54f, 0.27f, 48);
    R_col(fminf(c.r*1.08f,1.f), fminf(c.g*1.05f,1.f), fminf(c.b*1.03f,1.f), 0.22f);
    R_filledEllipse(CX + 0.30f, CY + 0.16f, 0.20f, 0.11f, 28);

    /* 5. Gloss rims */
    std::vector<float> rimA;
    rimA.reserve(N * 2);
    for (int i = 0; i < N; i++) {
        float t = (float)i / N;
        if (t < 0.20f || t > 0.68f) continue;
        float ix = xs[i] + (CX - xs[i]) * 0.06f;
        float iy = ys[i] + (CY - ys[i]) * 0.06f;
        rimA.push_back(ix);
        rimA.push_back(iy);
    }
    if (!rimA.empty()) {
        R_col(fminf(c.r*1.38f,1.f), fminf(c.g*1.28f,1.f), fminf(c.b*1.20f,1.f), 0.20f);
        glLineWidth(14.0f);
        R_lineStrip(rimA.data(), (int)rimA.size()/2);
        glLineWidth(1.0f);
    }

    std::vector<float> rimB;
    rimB.reserve(N * 2);
    for (int i = 0; i < N; i++) {
        float t = (float)i / N;
        if (t < 0.24f || t > 0.62f) continue;
        float ix = xs[i] + (CX - xs[i]) * 0.018f;
        float iy = ys[i] + (CY - ys[i]) * 0.018f;
        rimB.push_back(ix);
        rimB.push_back(iy);
    }
    if (!rimB.empty()) {
        R_col(1.0f, 1.0f, 1.0f, 0.30f);
        glLineWidth(3.0f);
        R_lineStrip(rimB.data(), (int)rimB.size()/2);
        glLineWidth(1.0f);
    }

    /* tiny hotspot */
    R_col(1.0f, 1.0f, 1.0f, 0.65f);
    R_filledEllipse(CX - 0.232f, CY + 0.282f, 0.026f, 0.014f, 14);

    /* 6. DAMAGE SPOTS — appear from stage 1, increase with severity.
       All positions verified inside contour: |x_off|≤65, |y_off|≤42. */
    float dmgA = DMG[anim.stage < 6 ? anim.stage : 5];
    if (anim.stage < 5) dmgA += (DMG[anim.stage+1] - dmgA) * anim.stageFrac();
    if (dmgA > 0.01f) {
        /* {x_pixel_offset, y_pixel_offset, radius_pixels} — all ÷250 */
        static const float SP[][3] = {
            { 15,  25, 15},   /* centre dome         */
            {-38,   5, 16},   /* left lobe           */
            { 58,  32, 12},   /* right lobe upper    */
            {-48,  25, 13},   /* left lobe upper     */
            {  5, -18, 10},   /* inferior centre     */
            {-12,  38, 11},   /* superior centre     */
            { 38,  16, 11},   /* right mid           */
            {-25, -14, 10},   /* lower left          */
            { 24,  -8, 10},   /* lower right         */
        };
        for (auto& sp : SP) {
            float sx = CX + sp[0]/250.f;
            float sy = CY + sp[1]/250.f;
            float sr = sp[2]/250.f;
            R_col(c.r*0.52f, c.g*0.33f, c.b*0.26f, dmgA*0.30f);
            R_filledEllipse(sx, sy, sr*1.30f, sr*1.10f, 20);
            R_col(c.r*0.42f, c.g*0.26f, c.b*0.20f, dmgA*0.50f);
            R_filledEllipse(sx, sy, sr*0.60f, sr*0.60f, 16);
        }
    }

   
}

/* ── Vessels ─────────────────────────────────────────────────── */
/*
 * IVC (blue) and hepatic artery (red) drawn as GL_QUADS before the
 * liver body. The liver covers them in the middle, with shine caps
 * where the tubes emerge at the top and bottom of the organ.
 */
void LiverSimulation::drawVessels() {
    const float top  = CY + 0.60f;
    const float bot  = CY - 0.460f;
    const float TX   = CX + 20.f/250.f;

    /* Approximate liver entry/exit for shine caps */
    const float lvTop = CY + 0.200f;
    const float lvBot = CY - 0.200f;

    const float iL = TX - 15.f/250.f,  iR = TX -  2.f/250.f;
    const float aL = TX +  1.f/250.f,  aR = TX + 13.f/250.f;

    auto tube = [&](float L, float R,
                    float br, float bg, float bb,
                    float hr, float hg, float hb,
                    float sr, float sg, float sb)
    {
        float hw = R - L;
        R_col(br, bg, bb, 1.0f);
        R_filledRect(L, bot, R, top);

        R_col(hr, hg, hb, 0.50f);
        R_filledRect(L, bot, L + hw*0.30f, top);

        R_col(sr, sg, sb, 0.50f);
        R_filledRect(R - hw*0.25f, bot, R, top);

        R_col(1.f, 1.f, 1.f, 0.45f);
        R_filledEllipse((L+R)*0.5f, lvTop + 0.012f, hw*0.55f, 0.020f, 20);

        R_col(1.f, 1.f, 1.f, 0.32f);
        R_filledEllipse((L+R)*0.5f, lvBot - 0.012f, hw*0.55f, 0.016f, 20);

        float rect[] = {L, top, R, top, R, bot, L, bot};
        R_col(0.f, 0.f, 0.f, 0.22f);
        glLineWidth(1.2f);
        R_draw(GL_LINE_LOOP, rect, 4);
        glLineWidth(1.0f);
    };

    /* IVC — blue */
    tube(iL, iR,
         0.10f,0.22f,0.82f,
         0.45f,0.62f,0.96f,
         0.04f,0.08f,0.48f);

    /* Hepatic artery — red */
    tube(aL, aR,
         0.84f,0.08f,0.06f,
         0.96f,0.50f,0.44f,
         0.46f,0.04f,0.03f);
}

/* ── Tilted bottle ───────────────────────────────────────────── */
/*
 * Drawn as rotated quads (~40° clockwise) so the spout points
 * down-right toward the liver. Uses inline 2D rotation.
 */
void LiverSimulation::drawBottle() {
    if (anim.stage == 0) return;
    float x = bottleX, y = bottleY, s = bottleScale;

    float angle = -0.70f, cosA = cosf(angle), sinA = sinf(angle);
    float pvx = x, pvy = y + 0.13f*s;

    auto rot = [&](float lx, float ly, float& ox, float& oy) {
        float rx = lx - pvx, ry = ly - pvy;
        ox = pvx + rx*cosA - ry*sinA;
        oy = pvy + rx*sinA + ry*cosA;
    };
    auto rQ = [&](float x0,float y0,float x1,float y1,
                  float x2,float y2,float x3,float y3) {
        float ax,ay,bx,by,cx,cy,dx,dy;
        rot(x0,y0,ax,ay); rot(x1,y1,bx,by);
        rot(x2,y2,cx,cy); rot(x3,y3,dx,dy);
        float v[] = {ax,ay,bx,by,cx,cy,dx,dy};
        R_draw(GL_TRIANGLE_FAN, v, 4);
    };

    float bw=.060f*s, bh=.220f*s, sh=.060f*s,
          nw=.022f*s, nh=.120f*s, ch=.020f*s;
    float bY=y, tY=y+bh, nB=tY+sh, nT=nB+nh, cT=nT+ch;

    /* drop shadow */
    R_col(0,0,0,.28f);
    rQ(x-bw+.010f,bY-.008f, x+bw+.010f,bY-.008f,
       x+bw+.010f,nT-.008f, x-bw+.010f,nT-.008f);

    /* glass body + shoulder */
    R_col(.08f,.24f,.10f);
    rQ(x-bw,bY, x+bw,bY, x+bw,tY, x-bw,tY);
    rQ(x-bw,tY, x+bw,tY, x+nw,nB, x-nw,nB);

    /* neck */
    R_col(.07f,.20f,.09f);
    rQ(x-nw,nB, x+nw,nB, x+nw,nT, x-nw,nT);

    /* cork */
    R_col(.68f,.50f,.18f);
    rQ(x-nw*.88f,nT, x+nw*.88f,nT, x+nw*.88f,cT, x-nw*.88f,cT);

    /* glass highlight (Gouraud — left edge catches light) */
    R_col(.32f,.70f,.34f,.24f);
    rQ(x-bw+.005f,bY+.016f, x-bw+.016f,bY+.016f,
       x-bw+.016f,tY-.016f, x-bw+.005f,tY-.016f);

    /* label */
    float lX, lY; rot(x, y+bh*.5f, lX, lY);
    R_col(.94f,.90f,.78f,.80f); R_filledEllipse(lX,lY,.058f*s,.038f*s,20);
    R_col(.20f,.14f,.08f);
    T_drawCentred(lX, lY+.010f*s, .015f, "ALCOHOL");
    T_drawCentred(lX, lY-.014f*s, .012f, "40%");

    /* liquid level (sloshing) */
    float liqY = bY + bh*fmaxf(.04f, .70f - anim.stage*.10f)
               + .004f*s*sinf(anim.totalTime*3.f);
    R_col(.22f,.52f,.24f,.50f);
    rQ(x-bw+.005f,bY+.003f, x+bw-.005f,bY+.003f,
       x+bw-.005f,liqY,      x-bw+.005f,liqY);

    /* body outline */
    R_col(.04f,.14f,.05f); glLineWidth(1.6f);
    {
        float ox[4],oy[4];
        rot(x-bw,bY,ox[0],oy[0]); rot(x+bw,bY,ox[1],oy[1]);
        rot(x+bw,tY,ox[2],oy[2]); rot(x-bw,tY,ox[3],oy[3]);
        float v[] = {ox[0],oy[0],ox[1],oy[1],ox[2],oy[2],ox[3],oy[3]};
        R_draw(GL_LINE_LOOP, v, 4);
    }
    glLineWidth(1.f);
}

/* ── Alcohol drip ────────────────────────────────────────────── */
/*
 * Cubic Bezier arc (Ch.4) from bottle spout to liver surface.
 * Spout: (bottleX + 0.28*s, bottleY + 0.30*s)
 * Land:  (CX - 0.18, CY - 0.25) — inferior-left liver surface
 * Arc rises then falls for a natural parabolic pour.
 */
void LiverSimulation::drawAlcoholDrip() {
    if (anim.stage == 0) return;
    float s   = bottleScale;
    float spX = bottleX + .28f*s,  spY = bottleY + .30f*s;
    float lX  = CX - 0.18f,        lY  = CY - 0.25f;
    float cp1x = spX + 0.35f, cp1y = spY + 0.22f;   /* rise */
    float cp2x = lX  - 0.10f, cp2y = lY  + 0.20f;   /* fall */
    float in  = fminf((float)anim.stage/5.f + .20f, 1.f);

    auto st = G_bezier(spX, spY, cp1x, cp1y, cp2x, cp2y, lX, lY);

    /* outer amber glow */
    glLineWidth(6.0f + in*3.f);
    R_col(.78f,.58f,.12f, .22f*in); R_lineStrip(st);
    /* main stream */
    glLineWidth(3.0f + in*2.f);
    R_col(.78f,.58f,.12f, .82f*in); R_lineStrip(st);
    /* bright liquid core */
    glLineWidth(1.5f);
    R_col(.98f,.90f,.40f, .65f*in); R_lineStrip(st);
    glLineWidth(1.f);

    /* falling drops — 5 staggered along the Bezier path */
    float base = fmodf(anim.totalTime * 1.4f, 1.f);
    for (int d = 0; d < 5; d++) {
        float ph = fmodf(base + (float)d/5.f, 1.f);
        float t  = ph, mt = 1.f - t;
        float dx = mt*mt*mt*spX + 3*mt*mt*t*cp1x + 3*mt*t*t*cp2x + t*t*t*lX;
        float dy = mt*mt*mt*spY + 3*mt*mt*t*cp1y + 3*mt*t*t*cp2y + t*t*t*lY;
        float r  = (.014f + ph*.016f) * s;
        /* glow */
        R_col(.80f,.55f,.10f, (1.f-ph*.6f)*.50f*in);
        R_filledEllipse(dx, dy, r*1.6f, r*1.8f, 12);
        /* core drop */
        R_col(.90f,.65f,.15f, (1.f-ph*.4f)*.90f*in);
        R_filledEllipse(dx, dy, r, r*1.25f, 12);
        /* specular dot */
        R_col(1.f,.92f,.55f, (1.f-ph*.3f)*.65f*in);
        R_filledEllipse(dx - r*.25f, dy + r*.30f, r*.28f, r*.20f, 8);
    }

    /* splash ring at landing point */
    float sp = fmodf(anim.totalTime * 2.2f, 1.f);
    float sr = .012f + sp*.022f;
    R_col(.78f,.58f,.12f, (1.f-sp)*.58f*in);
    glLineWidth(1.8f);
    R_outlineEllipse(lX, lY, sr, sr*.35f, 18);
    glLineWidth(1.f);
    /* small darkened pool at contact point */
    R_col(.55f,.20f,.05f, (1.f-sp)*.28f*in);
    R_filledEllipse(lX, lY, sr*1.4f, sr*.50f, 16);
}

/* ── Stage panel ─────────────────────────────────────────────── */
void LiverSimulation::drawStagePanel() {
    float pW=.50f, pX=1.52f-pW, pT=.855f, pB=-.68f;
    float iH = (pT-pB) / (float)LUNG_STAGE_COUNT;
    Col3 sc = stageCol();

    R_col(.07f,.06f,.10f,.95f); R_filledRect(pX,pB,pX+pW,pT);
    R_col(.25f,.22f,.32f); glLineWidth(1.5f);
    R_outlineRect(pX,pB,pX+pW,pT); glLineWidth(1.f);

    R_col(sc.r*.30f,sc.g*.30f,sc.b*.30f);
    R_filledRect(pX,pT-.068f,pX+pW,pT);
    R_col(sc); glLineWidth(1.5f);
    R_filledRect(pX,pT-.070f,pX+pW,pT-.067f); glLineWidth(1.f);
    R_col(1,1,1);
    T_drawCentred(pX+pW*.5f, pT-.058f, .030f, "STAGE PROGRESS");

    for (int i = 0; i < LUNG_STAGE_COUNT; i++) {
        const StageInfo& si = LUNG_STAGES[i];
        float rT = pT-.068f - i*iH, rB = rT - iH + .005f;
        bool isA = (i==anim.stage), isD = (i<anim.stage);

        if      (isA) R_col(sc.r*.25f,sc.g*.25f,sc.b*.25f,.95f);
        else if (isD) R_col(.12f,.11f,.15f,.85f);
        else          R_col(.09f,.08f,.11f,.65f);
        R_filledRect(pX+.006f,rB,pX+pW-.006f,rT);

        if (isA) {
            R_col(sc); glLineWidth(2.f);
            R_outlineRect(pX+.006f,rB,pX+pW-.006f,rT);
            float am = (rT+rB)*.5f;
            float ar[] = {pX+pW-.035f,am+.018f,
                          pX+pW-.010f,am,
                          pX+pW-.035f,am-.018f};
            R_draw(GL_TRIANGLE_FAN,ar,3); glLineWidth(1.f);
        }

        Col3  rc  = SCA[i];
        float inn = isA ? 1.f : (isD ? .55f : .22f);
        R_col(rc.r*inn,rc.g*inn,rc.b*inn);
        R_filledRect(pX+.006f,rB,pX+.020f,rT);

        float tx = pX+.026f, al = isA ? 1.f : (isD ? .70f : .38f);
        R_col(1.f*al,.96f*al,.88f*al);
        T_drawText(tx, rT-.034f, .024f, si.label);
        R_col(.72f*al,.70f*al,.78f*al);
        T_drawText(tx, rT-.063f, .019f, si.condition);

        if (isA) {
            float pr=anim.stageFrac(), bx1=tx, bx2=pX+pW-.040f,
                  by=rB+.010f, bh=.012f;
            R_col(.12f,.11f,.16f); R_filledRect(bx1,by,bx2,by+bh);
            R_col(sc,.85f); R_filledRect(bx1,by,bx1+(bx2-bx1)*pr,by+bh);
            R_col(sc.r*.6f,sc.g*.6f,sc.b*.6f);
            R_outlineRect(bx1,by,bx2,by+bh);
        }

        R_col(.20f,.18f,.24f);
        float sep[] = {pX+.006f,rB, pX+pW-.006f,rB};
        R_draw(GL_LINES,sep,2);
    }
}

/* ── Bottom bar ──────────────────────────────────────────────── */
void LiverSimulation::drawBottomBar() {
    int si_idx = anim.stage < LUNG_STAGE_COUNT ? anim.stage : LUNG_STAGE_COUNT-1;
    const StageInfo& si = LUNG_STAGES[si_idx];
    Col3 sc = stageCol();
    float bx1=-1.55f, bx2=1.00f, by1=-.99f, by2=-.70f;

    R_col(.08f,.07f,.11f,.97f); R_filledRect(bx1,by1,bx2,by2);
    R_col(sc.r*.28f,sc.g*.28f,sc.b*.28f); glLineWidth(1.5f);
    R_outlineRect(bx1,by1,bx2,by2); glLineWidth(1.f);

    char hd[64]; snprintf(hd,64,"%s  |  %s",si.label,si.timeLabel);
    R_col(1.f,.95f,.80f); T_drawText(bx1+.04f,by2-.042f,.036f,hd);
    R_col(.65f,.62f,.72f); T_drawText(bx1+.04f,by2-.075f,.024f,"Damage Spectrum:");

    float barX1=bx1+.04f, barX2=bx2-.04f, barY=by2-.110f,
          barH=.022f, bw=barX2-barX1;
    float total = anim.totalFrac(LUNG_STAGE_COUNT);

    R_col(.12f,.11f,.15f); R_filledRect(barX1,barY,barX2,barY+barH);
    for (int i = 0; i < 40; i++) {
        float t0=(float)i/40, t1=(float)(i+1)/40;
        if (t0 > total) break;
        float tU = t1 > total ? total : t1;
        R_col(t0, 1.f-t0, .1f, .88f);
        R_filledRect(barX1+bw*t0, barY, barX1+bw*tU, barY+barH);
    }
    float px = barX1 + bw*total;
    R_col(1,1,1);
    R_filledRect(px-.007f,barY-.005f,px+.007f,barY+barH+.005f);
    glLineWidth(1.2f); R_col(.32f,.30f,.42f);
    R_outlineRect(barX1,barY,barX2,barY+barH); glLineWidth(1.f);
}

/* ── Algorithm bar ───────────────────────────────────────────── */
void LiverSimulation::drawAlgorithmBar() {
    R_col(.08f,.07f,.12f,.95f);
    R_filledRect(-1.58f, -.70f, 1.02f, -.68f);
    Col3 sc = stageCol();
    R_col(sc.r*.45f, sc.g*.45f, sc.b*.45f);
}

/* Stub — polar contour used instead */
std::vector<float> LiverSimulation::buildLiverFan(float,float,float,float){
    return {};
}