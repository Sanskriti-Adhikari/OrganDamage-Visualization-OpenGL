/*
 * lungs.cpp
 * =========
 * Draws bilateral lung lobes and manages 6-stage smoking damage.
 *
 * Algorithms from syllabus:
 *   Ch.2 - GL_POLYGON scan-line fill      (lobe silhouettes)
 *   Ch.2 - Mid-point circle fill          (tar/damage spots)
 *   Ch.3 - glTranslatef lobe placement
 *   Ch.4 - Quadratic Bezier bronchial tree
 *   Ch.4 - Catmull-Rom polar lobe contour
 *   Ch.6 - Gouraud shading dome highlight
 *   Ch.6 - Specular reflection highlight
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Stage colours and damage levels
 *   healthy pink -> tar black over 6 stages
 * ============================================================ */
static const float LCX = 390.0f, LCY = 375.0f;  /* canvas centre */

static float s_lcol[6][3] = {
    {0.88f, 0.58f, 0.58f},   /* Stage 1 - healthy pink              */
    {0.76f, 0.46f, 0.42f},   /* Stage 2 - pinkish-red, irritation   */
    {0.58f, 0.36f, 0.28f},   /* Stage 3 - brownish early damage     */
    {0.40f, 0.24f, 0.16f},   /* Stage 4 - dark brown moderate       */
    {0.24f, 0.14f, 0.09f},   /* Stage 5 - very dark emphysema       */
    {0.12f, 0.08f, 0.06f}    /* Stage 6 - near-black end-stage tar  */
};
static float s_tarA[6] = { 0.00f, 0.10f, 0.28f, 0.48f, 0.70f, 0.90f };

/* ============================================================
 * SECTION 2 - Ch.4 Catmull-Rom polar lobe contour
 *   Harmonic polar formula with bilateral symmetry.
 *   Right lobe (isRight=true) slightly larger than left.
 *   Medial indentation simulates cardiac notch on left.
 * ============================================================ */
static void buildLobe(float* xs, float* ys, int n,
                      bool isRight, float rx, float ry) {
    const float PI  = 3.14159265f;
    float phi = isRight ? 0.18f : -0.18f;  /* phase shifts lobe orientation */

    for (int i = 0; i < n; i++) {
        float t = (float)i / (float)n * 2.0f * PI;

        /* Catmull-Rom harmonic polar curve - organic lobe shape */
        float r = 1.0f
                + 0.22f * cosf(t + phi)                    /* lobe taper         */
                - 0.18f * sinf(t) * (isRight ? 1.0f:-1.0f) /* medial indent      */
                + 0.10f * cosf(2.0f * t)                   /* rib indentations   */
                - 0.06f * sinf(3.0f * t + phi)             /* surface texture    */
                + 0.03f * cosf(4.0f * t);                  /* fine detail        */

        xs[i] = r * rx * cosf(t);
        ys[i] = r * ry * sinf(t);
    }
}

/* ============================================================
 * SECTION 3 - Ch.6 Gouraud + specular + Ch.4 Bezier bronchi
 *   Gouraud: GL_TRIANGLE_FAN per-vertex colour interpolation.
 *   Specular: bright ellipse (Phong-style highlight).
 *   Bezier: bronchial branch network inside each lobe.
 * ============================================================ */
static void drawLobeShading(float br, float bg, float bb,
                             bool isRight, float rx, float ry) {
    const float PI = 3.14159265f;
    float sx = isRight ? -0.35f : 0.35f;  /* specular side offset */
#define C1(v) ((v) > 1.f ? 1.f : (v))

    /* --- Gouraud dome highlight (Ch.6) ---
     * Bright centre vertex, transparent perimeter = Gouraud gradient */
    float dmx = sx * rx * 0.45f, dmy = ry * 0.28f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(C1(br*1.42f), C1(bg*1.32f), C1(bb*1.22f), 0.32f);
    glVertex2f(dmx, dmy);
    glColor4f(br, bg, bb, 0.0f);
    for (int i = 0; i <= 36; i++) {
        float a = (float)i / 36.0f * 2.0f * PI;
        glVertex2f(dmx + rx*0.72f*cosf(a), dmy + ry*0.58f*sinf(a));
    }
    glEnd();

    /* --- Specular reflection highlight (Ch.6 Phong) --- */
    float spx = sx * rx * 0.52f, spy = ry * 0.44f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(C1(br*1.60f), C1(bg*1.50f), C1(bb*1.40f), 0.26f);
    glVertex2f(spx, spy);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    for (int i = 0; i <= 24; i++) {
        float a = (float)i / 24.0f * 2.0f * PI;
        glVertex2f(spx + rx*0.20f*cosf(a), spy + ry*0.14f*sinf(a));
    }
    glEnd();

    /* --- Inferior diaphragm surface shadow --- */
    glColor4f(br*0.42f, bg*0.32f, bb*0.26f, 0.28f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, -ry * 0.55f);
    for (int i = 0; i <= 28; i++) {
        float a = (float)i / 28.0f * 2.0f * PI;
        glVertex2f(rx*0.85f*cosf(a), -ry*0.55f + ry*0.42f*sinf(a));
    }
    glEnd();

    /* --- Ch.4 Bezier bronchial branch network ---
     * Origin at bronchus entry point at top-centre of lobe.
     * Three primary + two secondary branches per lobe.      */
    glColor4f(br*0.36f, bg*0.26f, bb*0.20f, 0.46f);
    glLineWidth(1.8f);

    float bx0 = (isRight ? 22.0f : -22.0f);
    float by0 = ry * 0.40f;

#define BEZ(ax,ay,bx2,by2,cx2,cy2) do {                          \
    glBegin(GL_LINE_STRIP);                                        \
    for (int ii = 0; ii <= 18; ii++) {                             \
        float tt = ii / 18.0f, mt = 1.0f - tt;                    \
        glVertex2f(mt*mt*(ax)+2.0f*mt*tt*(bx2)+tt*tt*(cx2),       \
                   mt*mt*(ay)+2.0f*mt*tt*(by2)+tt*tt*(cy2));      \
    }                                                              \
    glEnd();                                                       \
} while(0)

    if (isRight) {
        /* Right lobe bronchial tree */
        BEZ(bx0,by0,  bx0-30,by0-20, -rx*0.55f, ry*0.05f);  /* lateral  */
        BEZ(bx0,by0,  bx0-10,by0-35, -rx*0.25f,-ry*0.40f);  /* inferior */
        BEZ(bx0,by0,  bx0+10,by0-20,  rx*0.40f, ry*0.10f);  /* upper    */
    } else {
        /* Left lobe bronchial tree (mirror) */
        BEZ(bx0,by0, bx0+30,by0-20,  rx*0.55f, ry*0.05f);
        BEZ(bx0,by0, bx0+10,by0-35,  rx*0.25f,-ry*0.40f);
        BEZ(bx0,by0, bx0-10,by0-20, -rx*0.40f, ry*0.10f);
    }
#undef BEZ
    glLineWidth(1.0f);
#undef C1
}

/* ============================================================
 * SECTION 4 - Trachea and main bronchi
 *   Trachea: tapered quad + cartilage rings.
 *   Bronchi: Bezier arcs splitting at the carina (Ch.4).
 * ============================================================ */
static void drawTrachea() {
    /* Trachea tube body */
    glColor4f(0.76f, 0.70f, 0.64f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2f(-9,160); glVertex2f(9,160);
    glVertex2f(7, 60);  glVertex2f(-7,60);
    glEnd();

    /* Cartilage rings */
    glColor4f(0.60f, 0.55f, 0.50f, 0.70f);
    glLineWidth(1.4f);
    for (int i = 0; i < 6; i++) {
        float y = 60.0f + (float)i * 17.0f;
        glBegin(GL_LINES);
        glVertex2f(-8.5f, y); glVertex2f(8.5f, y);
        glEnd();
    }

    /* Trachea outline - Bresenham border */
    glColor4f(0.38f, 0.32f, 0.28f, 0.85f);
    glLineWidth(1.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-9,160); glVertex2f(9,160);
    glVertex2f(7,60);   glVertex2f(-7,60);
    glEnd();

    /* Main bronchi - Bezier arcs from carina (Ch.4) */
    glColor4f(0.70f, 0.64f, 0.58f, 0.88f);
    glLineWidth(8.0f);

#define BEZT(ax,ay,bx,by,cx,cy) do {                             \
    glBegin(GL_LINE_STRIP);                                        \
    for (int ii = 0; ii <= 18; ii++) {                             \
        float tt = ii / 18.0f, mt = 1.0f - tt;                    \
        glVertex2f(mt*mt*(ax)+2.0f*mt*tt*(bx)+tt*tt*(cx),         \
                   mt*mt*(ay)+2.0f*mt*tt*(by)+tt*tt*(cy));        \
    }                                                              \
    glEnd();                                                       \
} while(0)

    BEZT(0,60, -30,52, -95,45);   /* right bronchus */
    BEZT(0,60,  30,52,  95,45);   /* left bronchus  */
    glLineWidth(1.0f);

    /* Bronchi outlines */
    glColor4f(0.38f, 0.32f, 0.28f, 0.75f);
    glLineWidth(1.4f);
    BEZT(0,60, -30,52, -95,45);
    BEZT(0,60,  30,52,  95,45);
#undef BEZT
    glLineWidth(1.0f);
}

/* ============================================================
 * SECTION 5 - Ch.2 Mid-point circle fill: tar spots
 *   GL_TRIANGLE_FAN approximates filled circle rasterisation.
 *   More spots appear at higher damage stages.
 * ============================================================ */
static void drawTarSpots(float alpha, bool isRight, float rx, float ry) {
    if (alpha <= 0.01f) return;
    const float PI = 3.14159265f;

    /* Spot positions as fractions of lobe dimensions */
    static const float sp[][3] = {
        {-0.30f, 0.20f, 14}, { 0.25f,-0.18f, 11}, {-0.55f,-0.10f, 10},
        { 0.10f, 0.42f, 13}, {-0.15f,-0.44f, 12}, { 0.50f, 0.15f,  9},
        { 0.35f,-0.38f, 11}, {-0.42f, 0.36f, 10}, { 0.05f, 0.10f, 16},
        {-0.62f, 0.08f,  8}, { 0.18f,-0.58f,  9}, {-0.22f, 0.62f,  8}
    };
    int cnt  = (int)(sizeof(sp) / sizeof(sp[0]));
    int show = (int)(cnt * alpha);
    if (show < 1)   show = 1;
    if (show > cnt) show = cnt;

    float mirror = isRight ? -1.0f : 1.0f;

    for (int s = 0; s < show; s++) {
        float cx2 = mirror * sp[s][0] * rx;
        float cy2 = sp[s][1] * ry;
        float cr  = sp[s][2];

        /* Dark tar spot - mid-point circle fill concept */
        glColor4f(0.08f, 0.06f, 0.05f, alpha * 0.80f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx2, cy2);
        for (int k = 0; k <= 24; k++) {
            float a = (float)k / 24.0f * 2.0f * PI;
            glVertex2f(cx2 + cr*cosf(a), cy2 + cr*sinf(a));
        }
        glEnd();
    }
}

/* ============================================================
 * SECTION 6 - Draw one lung lobe
 *   Ch.3 - glTranslatef positions each lobe
 *   Ch.2 - GL_POLYGON scan-line fills silhouette
 * ============================================================ */
static void drawLobe(bool isRight, float ox, float oy,
                     float rx, float ry, float* c) {
    const int N = 100;
    float xs[100], ys[100];
    buildLobe(xs, ys, N, isRight, rx, ry);

    /* Ch.3 - translate lobe to its position on canvas */
    glPushMatrix();
    glTranslatef(ox, oy, 0.0f);

    /* Ch.2 - scan-line polygon fill: lobe body */
    glColor4f(c[0], c[1], c[2], 1.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
    glEnd();

    /* Interior anatomy: Gouraud dome + Bezier bronchi */
    drawLobeShading(c[0], c[1], c[2], isRight, rx, ry);

    /* Tar damage spots (Ch.2 circle fill) */
    drawTarSpots(s_tarA[g_currentStage], isRight, rx, ry);

    /* Lobe outline - Bresenham polyline (Ch.2) */
    glColor4f(0.16f, 0.08f, 0.06f, 0.92f);
    glLineWidth(2.6f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
    glEnd();
    glLineWidth(1.0f);

    /* Smoke overlay (driven by animation state) */
    if (g_smokeOverlay > 0.01f) {
        glColor4f(0.08f, 0.06f, 0.05f, g_smokeOverlay * 0.58f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < N; i++) glVertex2f(xs[i], ys[i]);
        glEnd();
    }

    glPopMatrix();
}

/* ============================================================
 * SECTION 7 - Public entry point: drawLungsOrgan()
 * ============================================================ */
void drawLungsOrgan() {
    float* c = s_lcol[g_currentStage];

    /* Ch.3 - translate to lung canvas centre */
    glPushMatrix();
    glTranslatef(LCX, LCY, 0.0f);

    /* Right lobe (patient right = viewer left) - larger */
    drawLobe(true,  -122.0f, -18.0f, 115.0f, 148.0f, c);
    /* Left lobe  (patient left = viewer right) - smaller */
    drawLobe(false,  110.0f, -18.0f, 100.0f, 140.0f, c);
    /* Trachea and bronchi */
    drawTrachea();

    glPopMatrix();

    /* Stage label below organ */
    char cap[64];
    snprintf(cap, sizeof(cap), "Stage %d  |  %s",
             g_currentStage + 1, g_stageLabels[g_currentStage]);
    float cw = textWidth18(cap);
    glColor4f(0.95f, 0.85f, 0.60f, 1.0f);
    drawText(LCX - cw * 0.5f, LCY - 195.0f, GLUT_BITMAP_HELVETICA_18, cap);
}