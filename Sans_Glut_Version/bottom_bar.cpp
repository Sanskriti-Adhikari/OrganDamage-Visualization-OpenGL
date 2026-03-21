/*
 * bottom_bar.cpp
 * ==============
 * Bottom panel (x=0, y=0, w=736, h=148).
 *
 * Three clearly separated rows — no overlap:
 *   Row A  y=108..138 : "Damage Spectrum:" + gradient swatch
 *   Row B  y=68..88   : Swatch end labels (Healthy / Damaged)
 *   Row C  y=44..62   : Current stage one-line description
 *   Row D  y=12..28   : Algorithms listing (small font)
 *
 * Text never exceeds panel width of 736px.
 */

#include <GL/glut.h>
#include <stdio.h>

extern int   g_currentStage;
extern void  drawRect(float,float,float,float,float,float,float,float);
extern void  drawRectOutline(float,float,float,float,float,float,float,float,float);

static const char* s_line[6] = {
    "1 Month  -  Occasional use, mild fat deposits, no permanent damage yet",
    "1 Year   -  Regular use, fatty liver (steatosis), cells under stress",
    "5 Years  -  Persistent use, alcoholic hepatitis, inflammation spreading",
    "10 Years -  Fibrosis, scar tissue accumulating, healthy cells replaced",
    "20 Years -  Cirrhosis, liver hardening, severely reduced function",
    "30 Years -  End-stage cirrhosis, organ failure risk, transplant needed"
};

static const char* s_algo =
    "Algorithms: Bezier (Ch.4)  |  Scan-line Fill (Ch.2)  |"
    "  2D/3D Transforms (Ch.3)  |  Gouraud Shading (Ch.6)  |  Key-frame Anim (Ch.7)";

static void t24(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* c=s; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
}
static void h18(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* c=s; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}
static void h12(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* c=s; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
}

void drawBottomBar() {
    const float PW=736.f, PH=148.f;

    /* Panel background */
    drawRect(0,0, PW,PH, 0.09f,0.04f,0.04f, 0.92f);
    drawRectOutline(0,0, PW,PH, 0.46f,0.24f,0.15f, 0.80f, 1.4f);

    /* ── Row A: Spectrum label + swatch ── */
    const float SX=14.f, SY=100.f, SW=340.f, SH=24.f;

    glColor4f(0.90f, 0.80f, 0.58f, 1.0f);
    t24(SX, SY + SH + 8.0f, "Damage Spectrum:");

    /* Gradient swatch */
    glBegin(GL_QUADS);
    glColor4f(0.78f, 0.52f, 0.50f, 1.0f);   /* healthy */
    glVertex2f(SX,    SY);
    glVertex2f(SX,    SY+SH);
    glColor4f(0.20f, 0.07f, 0.04f, 1.0f);   /* end-stage */
    glVertex2f(SX+SW, SY+SH);
    glVertex2f(SX+SW, SY);
    glEnd();
    drawRectOutline(SX,SY, SW,SH, 0.50f,0.28f,0.16f, 0.85f, 1.2f);

    /* Stage tick */
    float tx = SX + (float)g_currentStage/5.0f * SW;
    glColor4f(1.0f, 0.96f, 0.48f, 1.0f);
    glLineWidth(2.6f);
    glBegin(GL_LINES);
    glVertex2f(tx, SY-5.f); glVertex2f(tx, SY+SH+5.f);
    glEnd();
    glLineWidth(1.0f);

    /* ── Row B: swatch labels (below swatch, no overlap) ── */
    glColor4f(0.85f, 0.75f, 0.55f, 0.95f);
    h18(SX,          SY - 24.0f, "Healthy");
    h18(SX+SW-74.0f, SY - 24.0f, "Damaged");

    /* ── Row C: stage description (one line, truncated safe) ── */
    glColor4f(1.0f, 0.90f, 0.68f, 1.0f);
    h18(14.0f, 46.0f, s_line[g_currentStage]);

    /* ── Row D: algorithms (small, fits on one line) ── */
    glColor4f(0.58f, 0.48f, 0.36f, 0.88f);
    h12(14.0f, 14.0f, s_algo);
}