/*
 * ui.cpp
 * ======
 * Top title banner and organ-selector TAB buttons.
 *
 * Algorithms from syllabus:
 *   Ch.2 - Bresenham rectangle borders (drawRectBresenham)
 *   Ch.2 - Scan-line fill (drawRect)
 *   Ch.3 - 2D coordinate placement for centred text
 */

#include <GL/glut.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Local text helpers
 * ============================================================ */
static void drawH18(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}
static void drawT24(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
}
static float tw24(const char* s) {
    float w = 0;
    for (const char* c = s; *c; c++) w += (float)glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    return w;
}
static float tw18(const char* s) {
    float w = 0;
    for (const char* c = s; *c; c++) w += (float)glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    return w;
}

/* ============================================================
 * SECTION 2 - drawUI: top banner + organ selector tabs
 *   Ch.2 - scan-line fill for background panels
 *   Ch.2 - Bresenham borders on panels and tabs
 *   Ch.3 - text centring via pixel-width calculation
 * ============================================================ */
void drawUI() {
    /* --- Top title banner - scan-line fill (Ch.2) --- */
    drawRect(0.0f, 658.0f, (float)WIN_W, 42.0f,
             0.10f, 0.04f, 0.06f, 0.97f);
    drawRectBresenham(0.0f, 658.0f, (float)WIN_W, 42.0f,
                      0.55f, 0.26f, 0.14f, 0.80f, 1.4f);

    /* Centred title text - Ch.3 positioning */
    const char* title = (g_activeOrgan == 0)
        ? "LIVER DAMAGE - ALCOHOL CONSUMPTION"
        : "LUNG DAMAGE - SMOKING";
    float tw = tw24(title);
    glColor4f(1.0f, 0.90f, 0.65f, 1.0f);
    drawT24(500.0f - tw * 0.5f, 670.0f, title);

    /* --- Organ selector tabs --- */
    const char* tabs[2] = { "LIVER (Alcohol)", "LUNGS (Smoking)" };
    float tx = 170.0f, ty = 628.0f, tw2 = 175.0f, th = 22.0f, gap = 8.0f;

    for (int i = 0; i < 2; i++) {
        float bx  = tx + (float)i * (tw2 + gap);
        bool  act = (i == g_activeOrgan);

        /* Tab background - scan-line fill */
        drawRect(bx, ty, tw2, th,
                 act ? 0.80f : 0.18f,
                 act ? 0.50f : 0.10f,
                 act ? 0.18f : 0.08f, 0.95f);

        /* Tab border - Bresenham */
        drawRectBresenham(bx, ty, tw2, th,
                          act ? 1.00f : 0.46f,
                          act ? 0.88f : 0.26f,
                          act ? 0.30f : 0.14f,
                          0.90f, act ? 2.0f : 1.2f);

        /* Centred tab label */
        float lw = tw18(tabs[i]);
        glColor4f(act ? 1.0f : 0.72f,
                  act ? 0.96f : 0.65f,
                  act ? 0.78f : 0.52f, 1.0f);
        drawH18(bx + (tw2 - lw) * 0.5f, ty + 4.0f, tabs[i]);
    }

    /* Controls hint text */
    glColor4f(0.55f, 0.45f, 0.35f, 0.82f);
    drawH18(570.0f, ty + 4.0f, "TAB = switch organ   R = restart   ESC = quit");
}