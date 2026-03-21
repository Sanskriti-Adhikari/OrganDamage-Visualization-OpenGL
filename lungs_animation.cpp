/*
 * lungs_animation.cpp
 * ===================
 * Cigarette animation state machine for the lungs module.
 *
 * Algorithms from syllabus:
 *   Ch.3 - glTranslatef, glRotatef, glScalef composite transform
 *   Ch.4 - Quadratic Bezier smoke plume curves
 *   Ch.6 - Specular glow on cigarette ember tip
 *   Ch.7 - Key-frame smooth-step easing state machine
 *
 * KEY-FRAMES (tick counts):
 *   Frame 0   : off-screen, scale=0               [start]
 *   Frame 40  : appeared, scale=1                 [APPEAR]
 *   Frame 115 : above lungs                       [MOVE_ABOVE]
 *   Frame 145 : slight lean (-18 degrees)         [TILT]
 *   Frame 230 : smoke complete, lungs darkened    [SMOKE]
 *   Frame 265 : cigarette upright again           [UPRIGHT]
 *   Frame 300 : disappeared                       [DISAPPEAR]
 */

#include <GL/glut.h>
#include <math.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Animation state enum and variables
 * ============================================================ */
enum LAnimState {
    LA_IDLE = 0,
    LA_APPEAR,      /* key-frame: scale 0 -> 1             */
    LA_MOVE,        /* key-frame: position start -> above  */
    LA_TILT,        /* key-frame: tilt 0 -> -18 degrees    */
    LA_SMOKE,       /* key-frame: smoke stream grows       */
    LA_UPRIGHT,     /* key-frame: tilt -18 -> 0 degrees    */
    LA_DISAPPEAR,   /* key-frame: scale 1 -> 0             */
    LA_ADVANCE
};

static LAnimState ls_state    = LA_IDLE;
static int        ls_tick     = 0;
static int        ls_idleWait = 0;

/* Cigarette position and transform state */
static float ls_cx      = 860.0f;  /* current x position  */
static float ls_cy      = 560.0f;  /* current y position  */
static float ls_scale   = 0.0f;    /* appear/disappear    */
static float ls_tilt    = 0.0f;    /* rotation angle      */
static float ls_smoke   = 0.0f;    /* smoke stream length */
static bool  ls_smoking = false;

/* Key positions */
static const float LSX = 860.0f, LSY = 560.0f;  /* start: bottom right */
static const float LTX = 420.0f, LTY = 195.0f;  /* target: above lungs */

/* ============================================================
 * SECTION 2 - Cigarette drawing
 *   Paper body, filter, gold band, tobacco end.
 *   Ch.6 Specular: glowing white-hot ember tip.
 * ============================================================ */
static void drawCigarette() {
    const float PI = 3.14159265f;

    /* Paper tube body - scan-line fill (Ch.2) */
    glColor4f(0.94f, 0.92f, 0.88f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(-8,-20); glVertex2f(8,-20);
    glVertex2f(8, 110); glVertex2f(-8,110);
    glEnd();

    /* Filter band - orange/tan */
    glColor4f(0.78f, 0.52f, 0.22f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(-8,-20); glVertex2f(8,-20);
    glVertex2f(8,  12); glVertex2f(-8,12);
    glEnd();

    /* Gold band at filter/paper join */
    glColor4f(0.88f, 0.72f, 0.30f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2f(-8,10); glVertex2f(8,10);
    glVertex2f(8, 16); glVertex2f(-8,16);
    glEnd();

    /* Outer ember glow - orange halo */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.55f, 0.08f, 0.25f);
    glVertex2f(0.0f, 110.0f);
    glColor4f(1.0f, 0.30f, 0.0f, 0.0f);
    for (int i = 0; i <= 24; i++) {
        float a = (float)i / 24.0f * 2.0f * PI;
        glVertex2f(14.0f * cosf(a), 110.0f + 14.0f * sinf(a));
    }
    glEnd();

    /* Specular white-hot ember core (Ch.6 Phong specular)
     * Simulates intense light source at the burning tip.   */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.95f, 0.60f, 0.90f);  /* white-hot centre */
    glVertex2f(0.0f, 110.0f);
    glColor4f(0.95f, 0.35f, 0.05f, 0.80f); /* orange edge      */
    for (int i = 0; i <= 20; i++) {
        float a = (float)i / 20.0f * 2.0f * PI;
        glVertex2f(8.0f * cosf(a), 110.0f + 8.0f * sinf(a));
    }
    glEnd();

    /* Outline - Bresenham border (Ch.2) */
    glColor4f(0.30f, 0.25f, 0.20f, 0.75f);
    glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-8,-20); glVertex2f(8,-20);
    glVertex2f(8, 110); glVertex2f(-8,110);
    glEnd();
    glLineWidth(1.0f);
}

/* ============================================================
 * SECTION 3 - Ch.4 Bezier smoke plumes
 *   Three curling smoke streams from ember tip.
 *   Each is a quadratic Bezier with offset control points.
 * ============================================================ */
static void drawSmokeStream(float len) {
    if (len < 4.0f) return;

    /* Base point: top of cigarette in world coords */
    float tx = ls_cx;
    float ty = ls_cy + 110.0f * ls_scale;

    /* Three plume paths: {control x, control y, end x, end y} */
    struct { float cx, cy, ex, ey; } p[3] = {
        {tx - 22.0f, ty + len*0.45f, tx - 38.0f, ty + len       },
        {tx +  8.0f, ty + len*0.50f, tx + 18.0f, ty + len       },
        {tx - 12.0f, ty + len*0.40f, tx -  8.0f, ty + len*0.85f }
    };

    for (int i = 0; i < 3; i++) {
        glLineWidth(i == 0 ? 4.5f : (i == 1 ? 3.2f : 2.4f));
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 22; j++) {
            float t = (float)j / 22.0f;
            float bx, by;
            /* Quadratic Bezier evaluation (Ch.4) */
            bezierQuad(tx, ty, p[i].cx, p[i].cy,
                       p[i].ex, p[i].ey, t, &bx, &by);
            glColor4f(0.30f, 0.28f, 0.26f, 0.50f * (1.0f - t * 0.55f));
            glVertex2f(bx, by);
        }
        glEnd();
    }
    glLineWidth(1.0f);
}

/* ============================================================
 * SECTION 4 - Ch.7 Key-frame animation state machine
 *   smoothStep gives ease-in/ease-out at each transition.
 * ============================================================ */
void updateLungsAnimation(int) {
    if (g_activeOrgan != 1) return;  /* only runs in lungs view */
    ls_tick++;

    switch (ls_state) {

    /* IDLE: wait before starting cycle */
    case LA_IDLE:
        if (--ls_idleWait <= 0) {
            ls_state = LA_APPEAR; ls_tick = 0;
            ls_cx = LSX; ls_cy = LSY; ls_scale = 0;
            ls_tilt = 0; ls_smoke = 0; g_smokeOverlay = 0;
        }
        break;

    /* KEY-FRAME: cigarette appears, scale 0->1 (40 ticks) */
    case LA_APPEAR:
        ls_scale = smoothStep(clampf((float)ls_tick / 40.0f, 0, 1));
        if (ls_tick >= 40) { ls_state = LA_MOVE; ls_tick = 0; }
        break;

    /* KEY-FRAME: moves above lungs (75 ticks) */
    case LA_MOVE: {
        float t = smoothStep(clampf((float)ls_tick / 75.0f, 0, 1));
        ls_cx = lerpf(LSX, LTX, t);
        ls_cy = lerpf(LSY, LTY, t);
        if (ls_tick >= 75) { ls_state = LA_TILT; ls_tick = 0; }
        break;
    }

    /* KEY-FRAME: slight lean for smoking pose (30 ticks) */
    case LA_TILT: {
        float t = smoothStep(clampf((float)ls_tick / 30.0f, 0, 1));
        ls_tilt = lerpf(0.0f, -18.0f, t);
        if (ls_tick >= 30) { ls_state = LA_SMOKE; ls_tick = 0; ls_smoking = true; }
        break;
    }

    /* KEY-FRAME: smoke flows over lungs (85 ticks) */
    case LA_SMOKE: {
        float t = clampf((float)ls_tick / 85.0f, 0, 1);
        ls_smoke       = lerpf(0.0f, 240.0f, t);
        g_smokeOverlay = lerpf(0.0f,   1.0f, t);
        if (ls_tick >= 85) { ls_state = LA_UPRIGHT; ls_tick = 0; ls_smoking = false; }
        break;
    }

    /* KEY-FRAME: cigarette returns upright (35 ticks) */
    case LA_UPRIGHT: {
        float t = smoothStep(clampf((float)ls_tick / 35.0f, 0, 1));
        ls_tilt  = lerpf(-18.0f, 0.0f,   t);
        ls_smoke = lerpf(240.0f, 0.0f,   t);
        if (ls_tick >= 35) { ls_state = LA_DISAPPEAR; ls_tick = 0; }
        break;
    }

    /* KEY-FRAME: disappears, scale 1->0 (35 ticks) */
    case LA_DISAPPEAR: {
        float t = smoothStep(clampf((float)ls_tick / 35.0f, 0, 1));
        ls_scale = 1.0f - t;
        if (ls_tick >= 35) { ls_state = LA_ADVANCE; ls_tick = 0; }
        break;
    }

    /* Advance to next damage stage then loop */
    case LA_ADVANCE:
        if (g_currentStage < 5) { g_currentStage++; ls_idleWait = 90;  }
        else                    { g_currentStage=0;  g_smokeOverlay=0; ls_idleWait=150; }
        ls_state = LA_IDLE; ls_tick = 0;
        break;
    }
}

/* ============================================================
 * SECTION 5 - drawLungsAnimation: composite 3D transform (Ch.3)
 *   1. glTranslatef - screen position
 *   2. glRotatef    - tilt angle
 *   3. glScalef     - appear/disappear scale
 * ============================================================ */
void drawLungsAnimation() {
    if (g_activeOrgan != 1) return;
    if (ls_state == LA_IDLE || ls_state == LA_ADVANCE) return;

    /* Draw Bezier smoke plumes (Ch.4) */
    if (ls_smoking || ls_state == LA_UPRIGHT)
        drawSmokeStream(ls_smoke);

    /* Ch.3 composite transform: translate -> rotate -> scale */
    glPushMatrix();
    glTranslatef(ls_cx, ls_cy, 0.0f);
    glRotatef(ls_tilt, 0.0f, 0.0f, 1.0f);
    glScalef(ls_scale, ls_scale, 1.0f);
    drawCigarette();
    glPopMatrix();
}