/*
 * liver.cpp
 * =========
 * Draws the 2D liver silhouette and manages 6-stage alcohol damage.
 *
 * Algorithms from syllabus:
 *   Ch.2 - GL_POLYGON scan-line fill      (liver silhouette)
 *   Ch.2 - Mid-point circle fill          (fibrotic damage spots)
 *   Ch.3 - glTranslatef canvas placement
 *   Ch.4 - Quadratic Bezier hepatic veins
 *   Ch.4 - Polar parametric liver contour (Catmull-Rom style)
 *   Ch.6 - Gouraud shading dome highlight (GL_TRIANGLE_FAN gradient)
 *   Ch.6 - Specular reflection highlight  (bright ellipse)
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Global state (extern in globals.h)
 * ============================================================ */
int   g_currentStage = 0;
float g_pourOverlay  = 0.0f;
float g_smokeOverlay = 0.0f;
int   g_activeOrgan  = 0;

const char* g_stageLabels[6] = {
    "1 Month", "1 Year", "5 Years", "10 Years", "20 Years", "30 Years"
};

/* Canvas centre - organ placed here via glTranslatef (Ch.3) */
static const float CX = 390.0f, CY = 375.0f;

/* Stage colours: healthy salmon -> near-black necrotic */
static float s_col[6][3] = {
    {0.88f, 0.52f, 0.50f},   /* Stage 1 - healthy pink-red       */
    {0.82f, 0.40f, 0.38f},   /* Stage 2 - slightly darker        */
    {0.74f, 0.32f, 0.18f},   /* Stage 3 - orange-brown hepatitis */
    {0.58f, 0.20f, 0.10f},   /* Stage 4 - dark fibrosis          */
    {0.40f, 0.13f, 0.07f},   /* Stage 5 - very dark cirrhosis    */
    {0.24f, 0.08f, 0.05f}    /* Stage 6 - near-black end-stage   */
};

/* Damage spot opacity per stage */
static float s_dmgA[6] = { 0.00f, 0.12f, 0.28f, 0.48f, 0.68f, 0.88f };

/* ============================================================
 * SECTION 2 - Ch.4 Polar parametric liver contour
 *   Catmull-Rom style harmonic polar curve.
 *   r(t) = 1 + A*sin(t+phi) + B*cos(2t) - C*sin(3t) + ...
 *   120 sample points give a smooth organic silhouette.
 *   Phase offset phi=-0.52 rad places dominant right lobe
 *   left-of-centre matching anatomical illustrations.
 * ============================================================ */
static void buildLiverContour(float* xs, float* ys, int n) {
    const float PI  = 3.14159265f;
    const float PHI = -0.52f;   /* phase: right lobe dominant   */
    const float Rx  = 185.0f;   /* horizontal semi-axis         */
    const float Ry  = 112.0f;   /* vertical semi-axis           */

    for (int i = 0; i < n; i++) {
        float t = (float)i / (float)n * 2.0f * PI;

        /* Multi-harmonic polar formula - organic liver shape */
        float r = 1.0f
                + 0.32f * sinf(t + PHI)          /* primary lobe asymmetry  */
                + 0.16f * cosf(2.0f * t)          /* inferior curve          */
                - 0.09f * sinf(3.0f * t + PHI)    /* surface ripple          */
                + 0.05f * cosf(4.0f * t)           /* fine detail             */
                + 0.025f* sinf(5.0f * t);          /* micro texture           */

        xs[i] = r * Rx * cosf(t) + 14.0f * cosf(2.0f * t);
        ys[i] = r * Ry * sinf(t) -  9.0f * sinf(2.0f * t);
    }
}

/* ============================================================
 * SECTION 3 - Ch.6 Gouraud shading + Ch.4 Bezier veins
 *   Gouraud: GL_TRIANGLE_FAN with per-vertex colour.
 *   Centre vertex = bright, perimeter = base colour -> GPU
 *   interpolates exactly as Gouraud shading prescribes.
 *   Specular: small bright ellipse simulating Phong highlight.
 * ============================================================ */
static void drawAnatomicalShading(float br, float bg, float bb) {
    const float PI = 3.14159265f;
#define C1(v) ((v) > 1.f ? 1.f : (v))

    /* --- Gouraud dome highlight on right lobe (Ch.6) ---
     * Bright centre fades to transparent at perimeter.      */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(C1(br*1.45f), C1(bg*1.35f), C1(bb*1.25f), 0.35f); /* bright centre */
    glVertex2f(-38.0f, 28.0f);
    glColor4f(br, bg, bb, 0.0f);                                   /* fade to edge  */
    for (int i = 0; i <= 40; i++) {
        float a = (float)i / 40.0f * 2.0f * PI;
        glVertex2f(-38.0f + 115.0f * cosf(a), 28.0f + 68.0f * sinf(a));
    }
    glEnd();

    /* --- Specular reflection highlight (Ch.6 Phong) ---
     * White-hot ellipse simulating overhead light source.   */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(C1(br*1.55f), C1(bg*1.48f), C1(bb*1.38f), 0.22f);
    glVertex2f(-72.0f, 66.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    for (int i = 0; i <= 20; i++) {
        float a = (float)i / 20.0f * 2.0f * PI;
        glVertex2f(-72.0f + 24.0f * cosf(a), 66.0f + 14.0f * sinf(a));
    }
    glEnd();

    /* --- Left lobe tab - slightly lighter (thinner tissue) --- */
    glColor4f(C1(br*1.18f), C1(bg*1.10f), C1(bb*1.06f), 0.24f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(108.0f, 52.0f);
    for (int i = 0; i <= 24; i++) {
        float a = (float)i / 24.0f * 2.0f * PI;
        glVertex2f(108.0f + 52.0f * cosf(a), 52.0f + 30.0f * sinf(a));
    }
    glEnd();

    /* --- Inferior surface shadow --- */
    glColor4f(br*0.40f, bg*0.30f, bb*0.24f, 0.30f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(-12.0f, -62.0f);
    for (int i = 0; i <= 30; i++) {
        float a = (float)i / 30.0f * 2.0f * PI;
        glVertex2f(-12.0f + 130.0f * cosf(a), -62.0f + 44.0f * sinf(a));
    }
    glEnd();

    /* --- Falciform ligament ridge --- */
    glColor4f(C1(br*1.24f), C1(bg*1.16f), C1(bb*1.09f), 0.28f);
    glLineWidth(5.5f);
    glBegin(GL_LINES);
    glVertex2f(35.0f, 78.0f);
    glVertex2f(20.0f, -68.0f);
    glEnd();
    glLineWidth(1.0f);

    /* --- Porta hepatis notch shadow --- */
    glColor4f(br*0.32f, bg*0.22f, bb*0.18f, 0.46f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(24.0f, -72.0f);
    for (int i = 0; i <= 20; i++) {
        float a = (float)i / 20.0f * 3.14159f + 3.14159f;
        glVertex2f(24.0f + 28.0f * cosf(a), -72.0f + 18.0f * sinf(a));
    }
    glEnd();

    /* --- Ch.4 Bezier hepatic veins ---
     * Four veins converge at porta hepatis (22, -14).
     * Each is a quadratic Bezier: P(t)=(1-t)^2*P0+2(1-t)t*P1+t^2*P2  */
    glColor4f(br*0.38f, bg*0.28f, bb*0.22f, 0.50f);
    glLineWidth(2.6f);

#define VEIN(ax,ay,bx,by,cx,cy) do {                              \
    glBegin(GL_LINE_STRIP);                                        \
    for (int ii = 0; ii <= 20; ii++) {                             \
        float tt = ii / 20.0f, mt = 1.0f - tt;                    \
        glVertex2f(mt*mt*(ax) + 2.0f*mt*tt*(bx) + tt*tt*(cx),     \
                   mt*mt*(ay) + 2.0f*mt*tt*(by) + tt*tt*(cy));    \
    }                                                              \
    glEnd();                                                       \
} while(0)

    VEIN(22, -14,  88,  22, 115,  68);  /* right hepatic vein  */
    VEIN(22, -14,  20,  32,  12,  74);  /* middle hepatic vein */
    VEIN(22, -14, -42,  16, -70,  44);  /* left hepatic vein   */
    VEIN(22, -14,  60, -40,  80, -72);  /* inferior branch     */

#undef VEIN
    glLineWidth(1.0f);
#undef C1
}

/* ============================================================
 * SECTION 4 - IVC and hepatic artery vessels
 *   Two coloured tubes passing through porta hepatis.
 *   Blue = IVC (inferior vena cava)
 *   Red  = hepatic artery
 * ============================================================ */
static void drawVessels() {
    const float top = 120.0f, bot = -120.0f;
    /* IVC */
    const float iL = 6.0f,  iR = 20.0f;
    /* Artery */
    const float aL = 23.0f, aR = 36.0f;

    /* --- IVC blue tube --- */
    glColor4f(0.08f, 0.20f, 0.84f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(iL,top); glVertex2f(iR,top);
    glVertex2f(iR,bot); glVertex2f(iL,bot);
    glEnd();
    /* Left highlight (Gouraud edge brightening, Ch.6) */
    glColor4f(0.42f, 0.60f, 0.96f, 0.46f);
    glBegin(GL_QUADS);
    glVertex2f(iL,top); glVertex2f(iL+4,top);
    glVertex2f(iL+4,bot); glVertex2f(iL,bot);
    glEnd();

    /* --- Hepatic artery red tube --- */
    glColor4f(0.86f, 0.08f, 0.06f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(aL,top); glVertex2f(aR,top);
    glVertex2f(aR,bot); glVertex2f(aL,bot);
    glEnd();
    /* Left highlight */
    glColor4f(0.96f, 0.52f, 0.46f, 0.44f);
    glBegin(GL_QUADS);
    glVertex2f(aL,top); glVertex2f(aL+3,top);
    glVertex2f(aL+3,bot); glVertex2f(aL,bot);
    glEnd();

    /* Outlines - Bresenham borders (Ch.2) */
    glColor4f(0, 0, 0, 0.32f);
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

/* ============================================================
 * SECTION 5 - Ch.2 Mid-point circle fill: damage nodules
 *   GL_TRIANGLE_FAN approximates filled circle rasterisation.
 *   More spots appear and become more opaque each stage.
 * ============================================================ */
static void drawDamageSpots(float alpha) {
    if (alpha <= 0.01f) return;
    const float PI = 3.14159265f;

    /* Spot data: {x, y, radius} in local liver coords */
    static const float sp[][3] = {
        { 22,  34, 19}, {-52,   4, 23}, { 88,  50, 16},
        {-70,  40, 21}, {  4, -32, 13}, {110,  14, 11},
        {-22,  66, 15}, { 54,  72, 17}, {-95, -10, 12},
        { -8,  10, 26}, { 66, -24, 14}, {-42, -48, 16},
        {-60, -30, 10}, { 30, -52, 11}
    };
    int cnt  = (int)(sizeof(sp) / sizeof(sp[0]));
    int show = (int)(cnt * alpha);
    if (show < 2)   show = 2;
    if (show > cnt) show = cnt;

    for (int s = 0; s < show; s++) {
        /* Dark necrotic core */
        glColor4f(0.16f, 0.05f, 0.04f, alpha * 0.72f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sp[s][0], sp[s][1]);
        for (int k = 0; k <= 24; k++) {
            float a = (float)k / 24.0f * 2.0f * PI;
            glVertex2f(sp[s][0] + sp[s][2] * cosf(a),
                       sp[s][1] + sp[s][2] * sinf(a));
        }
        glEnd();
        /* Fibrous ring around spot */
        glColor4f(0.62f, 0.42f, 0.22f, alpha * 0.35f);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k <= 24; k++) {
            float a = (float)k / 24.0f * 2.0f * PI;
            glVertex2f(sp[s][0] + sp[s][2] * cosf(a),
                       sp[s][1] + sp[s][2] * sinf(a));
        }
        glEnd();
    }
}

/* ============================================================
 * SECTION 6 - Public entry point: drawLiver()
 *   Ch.3 - glTranslatef positions organ on canvas
 *   Ch.2 - GL_POLYGON scan-line fills silhouette
 * ============================================================ */
void drawLiver() {
    const int N = 120;
    float xs[120], ys[120];
    buildLiverContour(xs, ys, N);

    /* Ch.3 - 2D translation to canvas centre */
    glPushMatrix();
    glTranslatef(CX, CY, 0.0f);

    float* c = s_col[g_currentStage];

    /* Ch.2 - Scan-line polygon fill: liver body */
    glColor4f(c[0], c[1], c[2], 1.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
    glEnd();

    /* Interior anatomy: Gouraud dome + Bezier veins */
    drawAnatomicalShading(c[0], c[1], c[2]);

    /* Damage nodules (mid-point circle fill concept, Ch.2) */
    drawDamageSpots(s_dmgA[g_currentStage]);

    /* Vessels on top */
    drawVessels();

    /* Organ outline - Bresenham polyline (Ch.2) */
    glColor4f(0.18f, 0.05f, 0.05f, 0.92f);
    glLineWidth(2.8f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
    glEnd();
    glLineWidth(1.0f);

    /* Red wine pour overlay (driven by animation state) */
    if (g_pourOverlay > 0.01f) {
        glColor4f(0.55f, 0.02f, 0.05f, g_pourOverlay * 0.52f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
        glEnd();
    }

    glPopMatrix();

    /* Stage label below organ */
    char cap[64];
    snprintf(cap, sizeof(cap), "Stage %d  |  %s",
             g_currentStage + 1, g_stageLabels[g_currentStage]);
    float cw = textWidth18(cap);
    glColor4f(0.95f, 0.85f, 0.60f, 1.0f);
    drawText(CX - cw * 0.5f, CY - 168.0f, GLUT_BITMAP_HELVETICA_18, cap);
}

/* ============================================================
 * SECTION 7 - initStages: reset all state to stage 0
 * ============================================================ */
void initStages() {
    g_currentStage = 0;
    g_pourOverlay  = 0.0f;
    g_smokeOverlay = 0.0f;
    g_activeOrgan  = 0;
}