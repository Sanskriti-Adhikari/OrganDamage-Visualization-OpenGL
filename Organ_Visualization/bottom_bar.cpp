/*
 * bottom_bar.cpp
 * ==============
 * Bottom panel: damage spectrum swatch, stage description,
 * algorithm credits.
 *
 * Algorithms from syllabus:
 *   Ch.2 - Gradient scan-line fill (GL_QUADS with per-vertex colour)
 *   Ch.2 - Bresenham tick mark and panel borders
 *   Ch.3 - 2D coordinate placement
 */

#include <GL/glut.h>
#include <stdio.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Stage description strings per organ
 * ============================================================ */
static const char* s_lv[6] = {
    "1 Month  - Occasional use, mild fat deposits, no permanent damage yet",
    "1 Year   - Regular use, fatty liver (steatosis), cells under stress",
    "5 Years  - Persistent use, alcoholic hepatitis, inflammation spreading",
    "10 Years - Fibrosis, scar tissue accumulating, healthy cells replaced",
    "20 Years - Cirrhosis, liver hardening, severely reduced function",
    "30 Years - End-stage cirrhosis, organ failure risk, transplant needed"
};
static const char* s_lu[6] = {
    "1 Month  - Mild airway irritation, cilia damage begins, occasional cough",
    "1 Year   - Chronic bronchitis, excess mucus, reduced airway clearance",
    "5 Years  - Early COPD, air sac damage begins, shortness of breath",
    "10 Years - Moderate COPD, emphysema onset, alveolar walls breaking down",
    "20 Years - Severe emphysema, gas exchange critically impaired",
    "30 Years - End-stage lung failure, oxygen dependency, cancer risk high"
};

/* Algorithm credits shown at bottom */
static const char* s_algo =
    "Algorithms: Bresenham Line(Ch.2) | Bezier Curve(Ch.4) | Scan-line Fill(Ch.2)"
    " | 2D/3D Transforms(Ch.3) | Gouraud+Specular Shading(Ch.6) | Key-frame Anim(Ch.7)";

/* ============================================================
 * SECTION 2 - Local text helpers
 * ============================================================ */
static void t24(float x,float y,const char* s){
    glRasterPos2f(x,y);
    for(const char* c=s;*c;c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,*c);
}
static void h18(float x,float y,const char* s){
    glRasterPos2f(x,y);
    for(const char* c=s;*c;c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*c);
}
static void h12(float x,float y,const char* s){
    glRasterPos2f(x,y);
    for(const char* c=s;*c;c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,*c);
}

/* ============================================================
 * SECTION 3 - drawBottomBar
 *   Row A (y=96..118): "Damage Spectrum:" + gradient swatch
 *   Row B (y=74..94):  swatch end labels
 *   Row C (y=44..62):  stage one-line description
 *   Row D (y=12..28):  algorithm credits (small font)
 * ============================================================ */
void drawBottomBar() {
    /* Panel background - scan-line fill (Ch.2) */
    drawRect(0.0f, 0.0f, (float)WIN_W, 148.0f,
             0.08f, 0.04f, 0.04f, 0.94f);
    drawRectBresenham(0.0f, 0.0f, (float)WIN_W, 148.0f,
                      0.46f, 0.24f, 0.15f, 0.80f, 1.4f);

    bool isL = (g_activeOrgan == 0);

    /* --- Row A: Spectrum label + gradient swatch --- */
    glColor4f(0.90f, 0.80f, 0.58f, 1.0f);
    t24(14.0f, 126.0f, "Damage Spectrum:");

    /* Gradient swatch - GL_QUADS with per-vertex colour (Ch.2 scan-line fill)
     * Left vertex = healthy colour, right = end-stage near-black             */
    const float SX = 14.0f, SY = 96.0f, SW = 320.0f, SH = 22.0f;
    glBegin(GL_QUADS);
    glColor4f(isL ? 0.78f : 0.72f,
              isL ? 0.52f : 0.60f,
              isL ? 0.50f : 0.58f, 1.0f);   /* healthy colour */
    glVertex2f(SX,      SY);
    glVertex2f(SX,      SY + SH);
    glColor4f(0.12f, 0.06f, 0.04f, 1.0f);   /* end-stage black */
    glVertex2f(SX + SW, SY + SH);
    glVertex2f(SX + SW, SY);
    glEnd();
    drawRectBresenham(SX, SY, SW, SH, 0.50f, 0.28f, 0.16f, 0.85f, 1.2f);

    /* Stage tick mark - Bresenham vertical line (Ch.2) */
    float tx = SX + (float)g_currentStage / 5.0f * SW;
    glColor4f(1.0f, 0.96f, 0.46f, 1.0f);
    drawBresenhamLine((int)tx, (int)(SY - 5.0f),
                      (int)tx, (int)(SY + SH + 5.0f), 2.4f);

    /* --- Row B: Swatch end labels --- */
    glColor4f(0.85f, 0.75f, 0.55f, 0.95f);
    h18(SX,              SY - 22.0f, "Healthy");
    h18(SX + SW - 74.0f, SY - 22.0f, "Damaged");

    /* --- Row C: Stage description --- */
    glColor4f(1.0f, 0.90f, 0.68f, 1.0f);
    h18(14.0f, 46.0f, (isL ? s_lv : s_lu)[g_currentStage]);

    /* --- Row D: Algorithm credits --- */
    glColor4f(0.55f, 0.46f, 0.34f, 0.88f);
    h12(14.0f, 14.0f, s_algo);
}