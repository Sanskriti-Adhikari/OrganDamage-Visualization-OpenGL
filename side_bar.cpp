/*
 * side_bar.cpp
 * ============
 * Six stage-progress boxes on the right side of the canvas.
 * Active stage highlighted; boxes darken with damage level.
 *
 * Algorithms from syllabus:
 *   Ch.2 - Scan-line fill (drawRect)
 *   Ch.2 - Bresenham rectangle borders (drawRectBresenham)
 *   Ch.3 - 2D coordinate placement
 */

#include <GL/glut.h>
#include <stdio.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Stage keyword labels per organ
 * ============================================================ */
static const char* s_kwL[6]  = {
    "Fatty Deposits", "Steatosis",    "Hepatitis",
    "Fibrosis",       "Cirrhosis",    "End-Stage"
};
static const char* s_kwLu[6] = {
    "Mild Irritation","Bronchitis",   "COPD Early",
    "COPD Moderate",  "Emphysema",    "Lung Failure"
};

/* Box colours: lighter at stage 1, near-black at stage 6 */
static const float s_bcL[6][3] = {
    {0.68f,0.28f,0.28f},{0.60f,0.22f,0.20f},{0.52f,0.20f,0.11f},
    {0.42f,0.13f,0.07f},{0.30f,0.08f,0.05f},{0.18f,0.05f,0.03f}
};
static const float s_bcLu[6][3] = {
    {0.40f,0.32f,0.30f},{0.35f,0.26f,0.22f},{0.28f,0.20f,0.16f},
    {0.22f,0.15f,0.11f},{0.16f,0.10f,0.07f},{0.10f,0.06f,0.04f}
};

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
 * SECTION 3 - drawSideBar: six stage boxes
 *   Boxes drawn bottom (stage 1) to top (stage 6).
 *   Active stage gets bright Bresenham border + full opacity.
 * ============================================================ */
void drawSideBar() {
    const float BX  = 852.0f;   /* left edge of sidebar     */
    const float BW  = 242.0f;   /* box width                */
    const float BH  = 72.0f;    /* box height               */
    const float GAP = 4.0f;     /* gap between boxes        */
    const float BOT = 152.0f;   /* y of lowest box bottom   */

    bool isL = (g_activeOrgan == 0);
    const char** kw        = isL ? s_kwL  : s_kwLu;
    const float (*bc)[3]   = isL ? s_bcL  : s_bcLu;

    /* Sidebar header */
    float hy = BOT + 6.0f * (BH + GAP) + 8.0f;
    glColor4f(0.92f, 0.82f, 0.60f, 1.0f);
    t24(BX + 12.0f, hy, "STAGE PROGRESS");

    for (int i = 0; i < 6; i++) {
        float by  = BOT + (float)(5 - i) * (BH + GAP);
        bool  act = (i == g_currentStage);

        /* --- Box fill - scan-line (Ch.2) --- */
        drawRect(BX, by, BW, BH,
                 bc[i][0], bc[i][1], bc[i][2],
                 act ? 0.95f : 0.40f);

        /* --- Box border - Bresenham (Ch.2) ---
         * Active: bright gold border outset by 2px
         * Inactive: dim brown border               */
        if (act)
            drawRectBresenham(BX-2, by-2, BW+4, BH+4,
                              1.0f, 0.92f, 0.44f, 1.0f, 2.6f);
        else
            drawRectBresenham(BX, by, BW, BH,
                              0.46f, 0.22f, 0.14f, 0.65f, 1.2f);

        /* --- Line 1: "Stage N | Time" --- */
        char top[48];
        snprintf(top, sizeof(top), "Stage %d | %s", i + 1, g_stageLabels[i]);
        float tc = act ? 1.0f : 0.70f;
        glColor4f(tc, tc * 0.86f, tc * 0.68f, 1.0f);
        t24(BX + 12.0f, by + BH - 26.0f, top);

        /* --- Line 2: condition keyword --- */
        float kc = act ? 1.0f : 0.55f;
        glColor4f(kc, kc * 0.58f, kc * 0.38f, 1.0f);
        h18(BX + 14.0f, by + 10.0f, kw[i]);

        /* Active stage arrow indicator */
        if (act) {
            glColor4f(1.0f, 0.94f, 0.42f, 1.0f);
            h18(BX + BW - 28.0f, by + BH - 26.0f, "<");
        }
    }

    /* Organ module label at bottom */
    glColor4f(0.60f, 0.50f, 0.38f, 0.80f);
    h12(BX + 12.0f, BOT - 18.0f, isL ? "[ Liver Module ]" : "[ Lungs Module ]");
}