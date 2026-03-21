/*
 * side_bar.cpp
 * ============
 * Six stage-progress boxes, right side of 1000x680 canvas.
 *
 * Box layout:
 *   X=742, Width=252, Height=74 each, Gap=5
 *   Two text lines per box — both safely inset 14px from edges.
 *   Line 1 uses TIMES_ROMAN_24 (fits "Stage 6 | 30 Years" in 224px)
 *   Line 2 uses HELVETICA_18  (keyword only — short strings)
 */

#include <GL/glut.h>
#include <stdio.h>

extern int         g_currentStage;
extern const char* g_stageLabels[];
extern void drawRect(float,float,float,float,float,float,float,float);
extern void drawRectOutline(float,float,float,float,float,float,float,float,float);

static const char* s_kw[6] = {
    "Healthy", "Fatty Liver", "Hepatitis",
    "Fibrosis", "Cirrhosis", "End-Stage"
};

static const float s_bc[6][3] = {
    { 0.70f, 0.30f, 0.30f }, { 0.62f, 0.24f, 0.22f },
    { 0.55f, 0.22f, 0.12f }, { 0.44f, 0.14f, 0.07f },
    { 0.32f, 0.09f, 0.05f }, { 0.20f, 0.06f, 0.03f }
};

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

void drawSideBar() {
    const float BX=742.f, BW=252.f, BH=74.f, GAP=5.f;

    /* Header */
    float hy = 148.f + 6.f*(BH+GAP) + 10.f;
    glColor4f(0.92f, 0.82f, 0.60f, 1.0f);
    t24(BX+14.f, hy, "STAGE PROGRESS");

    for (int i = 0; i < 6; i++) {
        float by = 148.f + (float)(5-i)*(BH+GAP);
        bool active = (i == g_currentStage);

        /* Fill */
        drawRect(BX, by, BW, BH,
                 s_bc[i][0], s_bc[i][1], s_bc[i][2],
                 active ? 0.96f : 0.42f);

        /* Border */
        if (active)
            drawRectOutline(BX-3.f,by-3.f,BW+6.f,BH+6.f,
                            1.0f,0.94f,0.46f,1.0f,2.8f);
        else
            drawRectOutline(BX,by,BW,BH,
                            0.46f,0.22f,0.14f,0.65f,1.2f);

        /* Line 1: "Stage N | Time" — TIMES_ROMAN_24, inset 14px */
        char top[40];
        snprintf(top, sizeof(top), "Stage %d | %s", i+1, g_stageLabels[i]);
        float tc = active ? 1.0f : 0.68f;
        glColor4f(tc, tc*0.88f, tc*0.70f, 1.0f);
        t24(BX+14.f, by+BH-28.f, top);

        /* Line 2: keyword — HELVETICA_18 */
        float kc = active ? 1.0f : 0.52f;
        glColor4f(kc, kc*0.60f, kc*0.40f, 1.0f);
        h18(BX+16.f, by+12.f, s_kw[i]);

        /* Active arrow — positioned at right edge safely */
        if (active) {
            glColor4f(1.0f, 0.95f, 0.44f, 1.0f);
            t24(BX+BW-32.f, by+BH-28.f, "<");
        }
    }
}