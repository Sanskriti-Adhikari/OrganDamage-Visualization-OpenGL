/*
 * globals.h
 * =========
 * Shared declarations used by every module.
 * Include with:  #include "globals.h"
 *
 * Syllabus coverage:
 *   Ch.2 - Bresenham line, scan-line fill, circle fill
 *   Ch.3 - 2D transform helpers
 *   Ch.4 - Bezier curve evaluation
 *   Ch.6 - Shading utilities
 *   Ch.7 - Key-frame animation state
 */

#pragma once
#include <GL/glut.h>
#include <math.h>

/* ============================================================
 * SECTION 1 - Canvas dimensions
 * ============================================================ */
#define WIN_W 1100
#define WIN_H  700

/* ============================================================
 * SECTION 2 - Shared state  (defined in liver.cpp)
 *   extern so every .cpp can read/write the same variables
 * ============================================================ */
extern int   g_currentStage;   /* 0..5  current damage stage        */
extern float g_pourOverlay;    /* 0..1  red wine wash on liver       */
extern float g_smokeOverlay;   /* 0..1  dark smoke wash on lungs     */
extern int   g_activeOrgan;    /* 0=liver  1=lungs                   */
extern const char* g_stageLabels[6];

/* ============================================================
 * SECTION 3 - Math utilities  (utils.cpp)
 *   Ch.7 - smoothStep easing for key-frame transitions
 *   Ch.4 - Quadratic Bezier point evaluation
 * ============================================================ */
float lerpf(float a, float b, float t);
float clampf(float v, float lo, float hi);
float smoothStep(float t);
void  bezierQuad(float p0x, float p0y,
                 float p1x, float p1y,
                 float p2x, float p2y,
                 float t, float* outX, float* outY);

/* ============================================================
 * SECTION 4 - Drawing utilities  (utils.cpp)
 *   Ch.2 - Bresenham line algorithm
 *   Ch.2 - Scan-line polygon fill (drawRect uses GL_QUADS)
 *   Ch.2 - Bresenham rectangle border (4 line segments)
 * ============================================================ */
void  drawBresenhamLine(int x0, int y0, int x1, int y1, float lw);
void  drawRect(float x, float y, float w, float h,
               float r, float g, float b, float a);
void  drawRectBresenham(float x, float y, float w, float h,
                        float r, float g, float b, float a, float lw);
void  drawRectOutline(float x, float y, float w, float h,
                      float r, float g, float b, float a, float lw);

/* ============================================================
 * SECTION 5 - Text helpers  (utils.cpp)
 * ============================================================ */
void  drawText(float x, float y, void* font, const char* text);
float textWidth18(const char* text);
float textWidth12(const char* text);