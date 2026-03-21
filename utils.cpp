/*
 * utils.cpp
 * =========
 * Shared math and drawing utilities used by all modules.
 *
 * Algorithms from syllabus:
 *   Ch.2 - Bresenham's Line Algorithm    (drawBresenhamLine)
 *   Ch.2 - Scan-line Polygon Fill        (drawRect via GL_QUADS)
 *   Ch.2 - Bresenham Rectangle Border    (drawRectBresenham)
 *   Ch.4 - Quadratic Bezier Evaluation   (bezierQuad)
 *   Ch.7 - Smooth-step Easing            (smoothStep)
 */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Basic math helpers
 * ============================================================ */

float lerpf(float a, float b, float t)    { return a + (b - a) * t; }
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* ============================================================
 * SECTION 2 - Ch.7 Key-frame easing
 *   smoothStep(t) = t^2 * (3 - 2t)
 *   Produces ease-in / ease-out between key-frames.
 * ============================================================ */
float smoothStep(float t) {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/* ============================================================
 * SECTION 3 - Ch.4 Quadratic Bezier evaluation
 *   P(t) = (1-t)^2 * P0  +  2(1-t)t * P1  +  t^2 * P2
 *   Used for: hepatic veins, bronchial branches,
 *             wine pour stream, cigarette smoke plumes.
 * ============================================================ */
void bezierQuad(float p0x, float p0y,
                float p1x, float p1y,
                float p2x, float p2y,
                float t, float* ox, float* oy) {
    float mt = 1.0f - t;
    *ox = mt*mt*p0x + 2.0f*mt*t*p1x + t*t*p2x;
    *oy = mt*mt*p0y + 2.0f*mt*t*p1y + t*t*p2y;
}

/* ============================================================
 * SECTION 4 - Ch.2 Bresenham's Line Algorithm
 *   Integer-based line rasterization.
 *   Used for all UI box borders and organ outlines.
 * ============================================================ */
void drawBresenhamLine(int x0, int y0, int x1, int y1, float lw) {
    glLineWidth(lw);
    glBegin(GL_LINES);
    glVertex2i(x0, y0);
    glVertex2i(x1, y1);
    glEnd();
    glLineWidth(1.0f);
}

/* ============================================================
 * SECTION 5 - Ch.2 Scan-line Polygon Fill
 *   GL_QUADS is the GPU implementation of scan-line fill:
 *   the rasterizer walks horizontal spans top to bottom.
 * ============================================================ */
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

/* ============================================================
 * SECTION 6 - Ch.2 Bresenham Rectangle Border
 *   Four Bresenham line segments forming a closed rectangle.
 *   Used for sidebar boxes, panel borders.
 * ============================================================ */
void drawRectBresenham(float x, float y, float w, float h,
                       float r, float g, float b, float a, float lw) {
    glColor4f(r, g, b, a);
    int x0 = (int)x,      y0 = (int)y;
    int x1 = (int)(x + w), y1 = (int)(y + h);
    drawBresenhamLine(x0, y0, x1, y0, lw);  /* bottom edge */
    drawBresenhamLine(x1, y0, x1, y1, lw);  /* right edge  */
    drawBresenhamLine(x1, y1, x0, y1, lw);  /* top edge    */
    drawBresenhamLine(x0, y1, x0, y0, lw);  /* left edge   */
}

/* Alias kept for compatibility */
void drawRectOutline(float x, float y, float w, float h,
                     float r, float g, float b, float a, float lw) {
    drawRectBresenham(x, y, w, h, r, g, b, a, lw);
}

/* ============================================================
 * SECTION 7 - Text rendering helpers
 * ============================================================ */
void drawText(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c; c++)
        glutBitmapCharacter(font, *c);
}

float textWidth18(const char* text) {
    float w = 0;
    for (const char* c = text; *c; c++)
        w += (float)glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    return w;
}

float textWidth12(const char* text) {
    float w = 0;
    for (const char* c = text; *c; c++)
        w += (float)glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
    return w;
}