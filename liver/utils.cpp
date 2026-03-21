/*
 * utils.cpp
 * =========
 * Shared math utilities used across all modules.
 *
 * Algorithms from syllabus utilized here:
 *   - Linear interpolation (foundation for all smooth transitions)
 *   - Quadratic Bezier curve evaluation (Ch.4 Parametric Curves)
 *   - Smooth-step easing (key-frame animation, Ch.7)
 *
 * No external includes beyond math.h needed.
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

/* ─── Linear interpolation between two floats ─── */
float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

/* ─── Clamp a float to [lo, hi] ─── */
float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* ─── Smooth-step easing (Ken Perlin's version, Ch.7 key-frame easing) ─── */
float smoothStep(float t) {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/* ─── Quadratic Bezier point evaluation (Ch.4 Bezier Curves) ─── */
/*     P(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2               */
void bezierQuad(float p0x, float p0y,
                float p1x, float p1y,
                float p2x, float p2y,
                float t,
                float* outX, float* outY) {
    float mt  = 1.0f - t;
    *outX = mt*mt*p0x + 2.0f*mt*t*p1x + t*t*p2x;
    *outY = mt*mt*p0y + 2.0f*mt*t*p1y + t*t*p2y;
}

/* ─── Draw a filled rectangle (used by ui, sidebar, bottom_bar) ─── */
void drawRect(float x, float y, float w, float h,
              float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

/* ─── Draw an outlined rectangle ─── */
void drawRectOutline(float x, float y, float w, float h,
                     float r, float g, float b, float a, float lw) {
    glColor4f(r, g, b, a);
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
    glLineWidth(1.0f);
}

/* ─── Render a string at 2D position (GLUT bitmap text) ─── */
void drawText(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++)
        glutBitmapCharacter(font, *c);
}

/* ─── Approximate pixel width of a string for centering ─── */
/*     HELVETICA_18 averages ~10px per character               */
float textWidth18(const char* text) {
    float w = 0.0f;
    for (const char* c = text; *c != '\0'; c++)
        w += (float)glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    return w;
}