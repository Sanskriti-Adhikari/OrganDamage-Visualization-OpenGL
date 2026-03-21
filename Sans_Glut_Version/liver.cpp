/*
 * liver.cpp
 * =========
 * Draws a 2-D liver silhouette and manages per-stage colour updates
 * that visualise progressive alcohol damage.
 *
 * Shape:
 *   The liver is modelled using a polar parametric curve tuned to
 *   match the natural resting orientation of the organ as seen in an
 *   anterior anatomical illustration:
 *     - Large right lobe dominates the left-centre of the frame
 *     - Smaller left lobe tapers toward the upper-right
 *     - Superior surface follows the diaphragm curve
 *     - Inferior notch is visible at the lower-centre
 *
 *   The radial formula is:
 *     r(t) = base + A·sin(t+φ) + B·cos(2t) − C·sin(3t) + D·cos(4t)
 *   with per-axis scale (Rx, Ry) and a centre offset (ox, oy) chosen
 *   so the dominant mass sits left-of-centre and the left lobe tab
 *   extends naturally to the upper-right.
 *
 * Interior anatomy:
 *   - Gouraud-style dome highlight on the right lobe (Ch.6)
 *   - Depth tint on the smaller left lobe
 *   - Falciform ligament ridge
 *   - Porta hepatis shadow
 *   - Hepatic vein branches as Bezier curves (Ch.4)
 *   - IVC (blue) and hepatic artery (red) tubes at the porta hepatis
 *
 * Algorithms demonstrated:
 *   Ch.2 — GL_POLYGON scan-line fill, circle rasterisation (damage spots)
 *   Ch.3 — glTranslatef for canvas placement
 *   Ch.4 — Quadratic Bezier hepatic vein lines
 *   Ch.6 — Gouraud shading (radial colour gradient on dome highlight)
 */

#include <GL/glut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

extern void  drawRect(float,float,float,float,float,float,float,float);
extern void  drawRectOutline(float,float,float,float,float,float,float,float,float);
extern void  drawText(float,float,void*,const char*);
extern float textWidth18(const char*);

/* Canvas centre — liver is placed here via glTranslatef */
static const float CX = 370.0f;
static const float CY = 355.0f;

#define NUM_STAGES 6
int   g_currentStage = 0;
float g_pourOverlay  = 0.0f;

const char* g_stageLabels[NUM_STAGES] = {
    "1 Month","1 Year","5 Years",
    "10 Years","20 Years","30 Years"
};

static float s_col[NUM_STAGES][3] = {
    { 0.90f, 0.55f, 0.55f },
    { 0.85f, 0.42f, 0.42f },
    { 0.78f, 0.35f, 0.20f },
    { 0.62f, 0.22f, 0.12f },
    { 0.45f, 0.15f, 0.08f },
    { 0.28f, 0.10f, 0.06f }
};
static float s_dmgA[NUM_STAGES] = {
    0.00f, 0.10f, 0.25f, 0.45f, 0.65f, 0.85f
};

static inline float lerpf(float a, float b, float t) { return a+(b-a)*t; }

/*
 * buildLiverContour()
 * -------------------
 * Polar parametric curve producing the natural liver silhouette.
 *
 * The phase offset φ = -0.52 rad (≈ -30°) shifts the dominant lobe
 * mass to the left-of-centre and lifts the left lobe tab to the
 * upper-right, matching the natural anatomical resting position.
 *
 * Rx = 175, Ry = 105 — wider than tall (correct liver proportions).
 * Centre offset (ox=12, oy=8) nudges the shape so the right lobe
 * bulk and left lobe tab sit symmetrically around the portal area.
 */
static void buildLiverContour(float* xs, float* ys, int n) {
    const float PI   = 3.14159265f;
    const float PHI  = -0.52f;   /* phase shift — places mass correctly     */
    const float Rx   = 175.0f;   /* horizontal semi-axis                    */
    const float Ry   = 105.0f;   /* vertical semi-axis                      */
    const float ox   =  12.0f;   /* centre offset x                         */
    const float oy   =   8.0f;   /* centre offset y                         */

    for (int i = 0; i < n; i++) {
        float t = (float)i / (float)n * 2.0f * PI;

        float r = 1.0f
                + 0.30f * sinf(t + PHI)
                + 0.15f * cosf(2.0f * t)
                - 0.08f * sinf(3.0f * t + PHI)
                + 0.05f * cosf(4.0f * t);

        xs[i] = r * Rx * cosf(t) + ox * cosf(2.0f * t);
        ys[i] = r * Ry * sinf(t) - oy * sinf(2.0f * t);
    }
}

/*
 * drawAnatomicalShading()
 * -----------------------
 * Layered semi-transparent fills and lines giving the liver depth:
 *
 *   Dome highlight   — soft lighter ellipse on the superior right lobe
 *                      (Gouraud radial fade, Ch.6)
 *   Left-lobe tint   — darker fill on the smaller upper-right tab
 *   Falciform ridge  — pale diagonal stripe separating the lobes
 *   Porta shadow     — dark arc at the inferior notch
 *   Hepatic veins    — three Bezier curves converging at the porta (Ch.4)
 */
static void drawAnatomicalShading(float br, float bg, float bb) {
    const float PI = 3.14159265f;
    #define C1(v) ((v)>1.f?1.f:(v))

    /* Right lobe dome — Gouraud radial fade (Ch.6) */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(C1(br*1.40f), C1(bg*1.30f), C1(bb*1.22f), 0.30f);
    glVertex2f(-28.0f, 25.0f);
    glColor4f(br, bg, bb, 0.0f);
    for (int i = 0; i <= 36; i++) {
        float a = (float)i/36.0f*2.0f*PI;
        glVertex2f(-28.0f+105.0f*cosf(a), 25.0f+60.0f*sinf(a));
    }
    glEnd();

    /* Specular spot — upper dome */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(C1(br*1.22f), C1(bg*1.15f), C1(bb*1.08f), 0.20f);
    glVertex2f(-55.0f, 52.0f);
    glColor4f(br, bg, bb, 0.0f);
    for (int i = 0; i <= 24; i++) {
        float a = (float)i/24.0f*2.0f*PI;
        glVertex2f(-55.0f+55.0f*cosf(a), 52.0f+28.0f*sinf(a));
    }
    glEnd();

    /* Left lobe tab tint — upper right, slightly lighter (thinner tissue) */
    glColor4f(C1(br*1.15f), C1(bg*1.08f), C1(bb*1.05f), 0.22f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(98.0f, 48.0f);
    for (int i = 0; i <= 24; i++) {
        float a = (float)i/24.0f*2.0f*PI;
        glVertex2f(98.0f+50.0f*cosf(a), 48.0f+28.0f*sinf(a));
    }
    glEnd();

    /* Inferior shadow — underside darker */
    glColor4f(br*0.45f, bg*0.35f, bb*0.28f, 0.28f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(-10.0f, -55.0f);
    for (int i = 0; i <= 28; i++) {
        float a = (float)i/28.0f*2.0f*PI;
        glVertex2f(-10.0f+120.0f*cosf(a), -55.0f+40.0f*sinf(a));
    }
    glEnd();

    /* Falciform ligament — diagonal pale ridge */
    glColor4f(C1(br*1.22f), C1(bg*1.15f), C1(bb*1.08f), 0.26f);
    glLineWidth(5.0f);
    glBegin(GL_LINES);
    glVertex2f( 32.0f,  72.0f);
    glVertex2f( 18.0f, -62.0f);
    glEnd();
    glLineWidth(1.0f);

    /* Porta hepatis notch shadow */
    glColor4f(br*0.35f, bg*0.25f, bb*0.20f, 0.42f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(22.0f, -68.0f);
    for (int i = 0; i <= 20; i++) {
        float a = (float)i/20.0f*PI + PI;
        glVertex2f(22.0f+26.0f*cosf(a), -68.0f+16.0f*sinf(a));
    }
    glEnd();

    /* Hepatic veins — Bezier curves converging at porta (Ch.4) */
    glColor4f(br*0.40f, bg*0.30f, bb*0.25f, 0.46f);
    glLineWidth(2.4f);

    auto bez = [](float ax,float ay,float bx,float by,float cx,float cy){
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 16; i++) {
            float t=i/16.0f, mt=1.0f-t;
            glVertex2f(mt*mt*ax+2.0f*mt*t*bx+t*t*cx,
                       mt*mt*ay+2.0f*mt*t*by+t*t*cy);
        }
        glEnd();
    };

    bez( 20.0f, -10.0f,  80.0f,  20.0f, 105.0f,  62.0f);  /* right vein */
    bez( 20.0f, -10.0f,  18.0f,  28.0f,  10.0f,  68.0f);  /* middle vein */
    bez( 20.0f, -10.0f, -38.0f,  12.0f, -62.0f,  38.0f);  /* left vein */

    glLineWidth(1.0f);
    #undef C1
}

/*
 * drawVessels()
 * -------------
 * IVC (hepatic vein, blue) and hepatic artery (red) pass vertically
 * through the porta hepatis at the inferior centre of the liver.
 * Drawn as rounded quads with edge highlights for a cylindrical appearance.
 */
static void drawVessels() {
    /* Tube geometry — positioned at the porta hepatis (x ≈ +20) */
    const float TX   =  20.0f;  /* centre of tube bundle             */
    const float TH   = 230.0f;  /* height — extends beyond the organ */
    const float top  =  TH * 0.5f;
    const float bot  = -TH * 0.5f;

    /* IVC positions */
    const float iL = TX - 15.0f;
    const float iR = TX - 2.0f;
    /* Artery positions */
    const float aL = TX + 1.0f;
    const float aR = TX + 13.0f;

    /* ── IVC — deep blue ── */
    glColor4f(0.10f, 0.22f, 0.82f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(iL,top); glVertex2f(iR,top);
    glVertex2f(iR,bot); glVertex2f(iL,bot);
    glEnd();
    /* left highlight */
    glColor4f(0.45f, 0.62f, 0.96f, 0.44f);
    glBegin(GL_QUADS);
    glVertex2f(iL,     top); glVertex2f(iL+3.5f,top);
    glVertex2f(iL+3.5f,bot); glVertex2f(iL,     bot);
    glEnd();
    /* right shadow */
    glColor4f(0.04f, 0.08f, 0.48f, 0.44f);
    glBegin(GL_QUADS);
    glVertex2f(iR-3.5f,top); glVertex2f(iR,top);
    glVertex2f(iR,     bot); glVertex2f(iR-3.5f,bot);
    glEnd();

    /* ── Hepatic artery — deep red ── */
    glColor4f(0.84f, 0.08f, 0.06f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(aL,top); glVertex2f(aR,top);
    glVertex2f(aR,bot); glVertex2f(aL,bot);
    glEnd();
    /* left highlight */
    glColor4f(0.96f, 0.50f, 0.44f, 0.42f);
    glBegin(GL_QUADS);
    glVertex2f(aL,     top); glVertex2f(aL+3.0f,top);
    glVertex2f(aL+3.0f,bot); glVertex2f(aL,     bot);
    glEnd();
    /* right shadow */
    glColor4f(0.46f, 0.04f, 0.03f, 0.44f);
    glBegin(GL_QUADS);
    glVertex2f(aR-3.0f,top); glVertex2f(aR,top);
    glVertex2f(aR,     bot); glVertex2f(aR-3.0f,bot);
    glEnd();

    /* Outlines */
    glColor4f(0.0f, 0.0f, 0.0f, 0.30f);
    glLineWidth(1.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(iL,top); glVertex2f(iR,top);
    glVertex2f(iR,bot); glVertex2f(iL,bot);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2f(aL,top); glVertex2f(aR,top);
    glVertex2f(aR,bot); glVertex2f(aL,bot);
    glEnd();
    glLineWidth(1.0f);
}

/*
 * drawDamageSpots()
 * -----------------
 * Fibrotic nodules overlaid on the liver surface.
 * Opacity increases each stage. Circle fill concept (Ch.2).
 */
static void drawDamageSpots(float alpha) {
    if (alpha <= 0.01f) return;
    const float PI = 3.14159265f;

    static const float sp[][3] = {
        {  20.0f,  32.0f, 18.0f }, { -48.0f,   2.0f, 22.0f },
        {  82.0f,  48.0f, 15.0f }, { -65.0f,  38.0f, 20.0f },
        {   2.0f, -30.0f, 12.0f }, { 105.0f,  12.0f, 10.0f },
        { -20.0f,  62.0f, 14.0f }, {  50.0f,  70.0f, 16.0f },
        { -90.0f,  -8.0f, 11.0f }, {  -5.0f,   8.0f, 25.0f },
        {  62.0f, -22.0f, 13.0f }, { -38.0f, -45.0f, 15.0f },
    };
    int cnt = (int)(sizeof(sp)/sizeof(sp[0]));

    for (int s = 0; s < cnt; s++) {
        glColor4f(0.18f, 0.06f, 0.04f, alpha*0.70f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sp[s][0], sp[s][1]);
        for (int k = 0; k <= 22; k++) {
            float a = (float)k/22.0f*2.0f*PI;
            glVertex2f(sp[s][0]+sp[s][2]*cosf(a),
                       sp[s][1]+sp[s][2]*sinf(a));
        }
        glEnd();
    }
}

/* ─── Public entry ─── */
void drawLiver() {
    const int N = 80;
    float xs[N], ys[N];
    buildLiverContour(xs, ys, N);

    glPushMatrix();
    glTranslatef(CX, CY, 0.0f);   /* Ch.3 — 2D translation */

    float* c = s_col[g_currentStage];

    /* Filled silhouette — GL_POLYGON scan-line fill (Ch.2) */
    glColor4f(c[0], c[1], c[2], 1.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
    glEnd();

    /* Interior anatomy */
    drawAnatomicalShading(c[0], c[1], c[2]);

    /* Damage spots */
    drawDamageSpots(s_dmgA[g_currentStage]);

    /* Vessels on top */
    drawVessels();

    /* Outline */
    glColor4f(0.20f, 0.05f, 0.05f, 0.90f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
    glEnd();
    glLineWidth(1.0f);

    /* Pour overlay */
    if (g_pourOverlay > 0.01f) {
        glColor4f(0.55f, 0.02f, 0.05f, g_pourOverlay*0.55f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
        glEnd();
    }

    glPopMatrix();

    /* Stage label */
    char cap[64];
    snprintf(cap, sizeof(cap), "Stage %d  |  %s",
             g_currentStage+1, g_stageLabels[g_currentStage]);
    float cw = textWidth18(cap);
    glColor4f(0.95f, 0.85f, 0.60f, 1.0f);
    drawText(CX - cw*0.5f, CY - 155.0f,
             GLUT_BITMAP_HELVETICA_18, cap);
}

void initStages() {
    g_currentStage = 0;
    g_pourOverlay  = 0.0f;
}