/*
 * ui.cpp
 * ======
 * Draws the top title banner across the full 1000px width.
 * Uses GLUT_BITMAP_TIMES_ROMAN_24 for maximum readability.
 * Title is pixel-centred at X=500.
 */

#include <GL/glut.h>

extern void  drawRect(float,float,float,float,float,float,float,float);
extern void  drawRectOutline(float,float,float,float,float,float,float,float,float);
extern float textWidth18(const char*);

/* Centre a string using TIMES_ROMAN_24 width */
static float titleWidth(const char* text) {
    float w = 0.0f;
    for (const char* c = text; *c != '\0'; c++)
        w += (float)glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    return w;
}

static void drawTitleText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
}

void drawUI() {
    /* Background bar — full width, 42px tall */
    drawRect(0.0f, 638.0f, 1000.0f, 42.0f,
             0.13f, 0.05f, 0.05f, 0.96f);
    drawRectOutline(0.0f, 638.0f, 1000.0f, 42.0f,
                    0.45f, 0.22f, 0.12f, 0.70f, 1.2f);

    /* Centred title */
    const char* title = "LIVER DAMAGE DUE TO ALCOHOL CONSUMPTION";
    float tw = titleWidth(title);
    glColor4f(1.0f, 0.90f, 0.65f, 1.0f);
    drawTitleText(500.0f - tw * 0.5f, 651.0f, title);
}