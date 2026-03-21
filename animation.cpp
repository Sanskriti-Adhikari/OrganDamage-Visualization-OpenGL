/*
 * animation.cpp
 * =============
 * Wine-glass animation state machine for the liver module.
 *
 * Algorithms from syllabus:
 *   Ch.3 - glTranslatef, glRotatef, glScalef composite transform
 *   Ch.4 - Quadratic Bezier pour stream curve
 *   Ch.6 - Specular highlight on glass bowl (Phong-style)
 *   Ch.7 - Key-frame smooth-step easing state machine
 *
 * KEY-FRAMES (tick counts):
 *   Frame 0   : glass off-screen, scale=0          [start]
 *   Frame 40  : glass fully appeared, scale=1      [APPEAR]
 *   Frame 115 : glass above liver                  [MOVE_ABOVE]
 *   Frame 165 : glass tilted 115 degrees           [ROTATE_TILT]
 *   Frame 250 : pour complete, liver washed red    [POUR]
 *   Frame 292 : glass uprighted                    [UPRIGHT]
 *   Frame 327 : glass disappeared                  [DISAPPEAR]
 */

#include <GL/glut.h>
#include <math.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Animation state enum and variables
 * ============================================================ */
enum AnimState {
    ANIM_IDLE = 0,
    ANIM_APPEAR,        /* key-frame: scale 0 -> 1              */
    ANIM_MOVE_ABOVE,    /* key-frame: position start -> target  */
    ANIM_ROTATE_TILT,   /* key-frame: tilt 0 -> 115 degrees     */
    ANIM_POUR,          /* key-frame: stream 0 -> 185px         */
    ANIM_UPRIGHT,       /* key-frame: tilt 115 -> 0 degrees     */
    ANIM_DISAPPEAR,     /* key-frame: scale 1 -> 0              */
    ANIM_ADVANCE_STAGE
};

static AnimState s_state    = ANIM_IDLE;
static int       s_tick     = 0;
static int       s_idleWait = 0;

/* Glass position and transform state */
static float s_gx     = 850.0f;  /* current x position    */
static float s_gy     = 590.0f;  /* current y position    */
static float s_scale  = 0.0f;    /* appear/disappear scale */
static float s_tilt   = 0.0f;    /* rotation angle degrees */
static float s_stream = 0.0f;    /* pour stream length px  */
static bool  s_pour   = false;

/* Key positions for animation path */
static const float SX = 850.0f, SY = 590.0f;  /* start: bottom right */
static const float TX = 448.0f, TY = 500.0f;  /* target: above liver */

/* ============================================================
 * SECTION 2 - resetSimulation: called on R key or stage wrap
 * ============================================================ */
void resetSimulation() {
    g_currentStage = 0;
    g_pourOverlay  = 0.0f;
    g_smokeOverlay = 0.0f;
    s_state     = ANIM_IDLE;
    s_tick      = 0;
    s_idleWait  = 80;
    s_gx = SX; s_gy = SY;
    s_scale = 0.0f; s_tilt = 0.0f; s_stream = 0.0f; s_pour = false;
}

/* ============================================================
 * SECTION 3 - Wine glass drawing
 *   Parametric ellipse for rim and base (Ch.2 mid-point ellipse)
 *   Specular highlight streak on bowl face (Ch.6)
 * ============================================================ */
static void drawWineGlass(float fill) {
    const float PI = 3.14159265f;

    /* Base ellipse - parametric circle (Ch.2) */
    glColor4f(0.68f, 0.84f, 0.92f, 0.72f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, -120);
    for (int i = 0; i <= 36; i++) {
        float a = (float)i / 36.0f * 2.0f * PI;
        glVertex2f(50.0f * cosf(a), -120.0f + 9.0f * sinf(a));
    }
    glEnd();

    /* Stem */
    glColor4f(0.64f, 0.82f, 0.90f, 0.68f);
    glBegin(GL_QUADS);
    glVertex2f(-7,-116); glVertex2f(7,-116);
    glVertex2f(10,-62);  glVertex2f(-10,-62);
    glEnd();

    /* Bowl body - scan-line fill (Ch.2) */
    glColor4f(0.54f, 0.74f, 0.84f, 0.50f);
    glBegin(GL_QUADS);
    glVertex2f(-28,-62); glVertex2f(28,-62);
    glVertex2f(52, 44);  glVertex2f(-52,44);
    glEnd();

    /* Wine fill level */
    if (fill > 0.01f) {
        float bot = -62.0f, top = 44.0f;
        float ft  = bot + (top - bot) * fill;
        float t   = (ft - bot) / (top - bot);
        float wf  = lerpf(28.0f, 52.0f, t);
        glColor4f(0.58f, 0.04f, 0.08f, 0.82f);
        glBegin(GL_QUADS);
        glVertex2f(-28, bot); glVertex2f(28, bot);
        glVertex2f(wf,  ft);  glVertex2f(-wf, ft);
        glEnd();
    }

    /* Bowl outline - Bresenham polyline (Ch.2) */
    glColor4f(0.82f, 0.93f, 1.0f, 0.86f);
    glLineWidth(2.4f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-28,-62); glVertex2f(-52,44);
    glVertex2f(52, 44);  glVertex2f(28,-62);
    glEnd();

    /* Rim ellipse - parametric circle (Ch.2) */
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 36; i++) {
        float a = (float)i / 36.0f * 2.0f * PI;
        glVertex2f(52.0f * cosf(a), 44.0f + 10.0f * sinf(a));
    }
    glEnd();

    /* Specular highlight on glass bowl (Ch.6 Phong specular)
     * Bright elliptical streak simulating overhead light.    */
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 1.0f, 0.36f);  /* bright specular centre */
    glVertex2f(-28.0f, 10.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);   /* fade to transparent     */
    for (int i = 0; i <= 20; i++) {
        float a = (float)i / 20.0f * 2.0f * PI;
        glVertex2f(-28.0f + 14.0f * cosf(a), 10.0f + 28.0f * sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);
}

/* ============================================================
 * SECTION 4 - Ch.4 Bezier pour stream
 *   Wine stream flows from tilted rim toward the liver.
 *   Quadratic Bezier with one control point for natural arc.
 * ============================================================ */
static void drawPourStream(float len) {
    if (len < 2.0f) return;
    const float PI = 3.14159265f;

    /* Start point: glass rim position in world coords */
    float ang = (s_tilt + 90.0f) * PI / 180.0f;
    float tx = s_gx + 55.0f * cosf(ang);
    float ty = s_gy + 55.0f * sinf(ang);

    /* End and control points for Bezier arc */
    float ex = tx + 18.0f, ey = ty - len;
    float cx = tx + 32.0f, cy = ty - len * 0.5f;

    glLineWidth(6.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 28; i++) {
        float t = (float)i / 28.0f;
        float bx, by;
        bezierQuad(tx, ty, cx, cy, ex, ey, t, &bx, &by);
        /* Fade alpha along stream */
        glColor4f(0.60f, 0.04f, 0.08f, 0.92f * (1.0f - t * 0.4f));
        glVertex2f(bx, by);
    }
    glEnd();
    glLineWidth(1.0f);
}

/* ============================================================
 * SECTION 5 - Ch.7 Key-frame animation state machine
 *   smoothStep(t) = t^2*(3-2t) gives ease-in/ease-out.
 *   Each state runs for a fixed tick count then transitions.
 * ============================================================ */
void updateAnimation(int) {
    if (g_activeOrgan != 0) return;  /* only runs in liver view */
    s_tick++;

    switch (s_state) {

    /* IDLE: wait before starting cycle */
    case ANIM_IDLE:
        if (--s_idleWait <= 0) {
            s_state = ANIM_APPEAR; s_tick = 0;
            s_gx = SX; s_gy = SY; s_scale = 0;
            s_tilt = 0; s_stream = 0; g_pourOverlay = 0;
        }
        break;

    /* KEY-FRAME: glass appears, scale 0->1 (40 ticks) */
    case ANIM_APPEAR:
        s_scale = smoothStep(clampf((float)s_tick / 40.0f, 0, 1));
        if (s_tick >= 40) { s_state = ANIM_MOVE_ABOVE; s_tick = 0; }
        break;

    /* KEY-FRAME: glass moves from start to above liver (75 ticks) */
    case ANIM_MOVE_ABOVE: {
        float t = smoothStep(clampf((float)s_tick / 75.0f, 0, 1));
        s_gx = lerpf(SX, TX, t);
        s_gy = lerpf(SY, TY, t);
        if (s_tick >= 75) { s_state = ANIM_ROTATE_TILT; s_tick = 0; }
        break;
    }

    /* KEY-FRAME: glass tilts 0->115 degrees (50 ticks) */
    case ANIM_ROTATE_TILT: {
        float t = smoothStep(clampf((float)s_tick / 50.0f, 0, 1));
        s_tilt = lerpf(0.0f, 115.0f, t);
        if (s_tick >= 50) { s_state = ANIM_POUR; s_tick = 0; s_pour = true; }
        break;
    }

    /* KEY-FRAME: wine pours out, stream grows (85 ticks) */
    case ANIM_POUR: {
        float t = clampf((float)s_tick / 85.0f, 0, 1);
        s_stream       = lerpf(0.0f, 185.0f, t);
        g_pourOverlay  = lerpf(0.0f, 1.0f,   t);
        if (s_tick >= 85) { s_state = ANIM_UPRIGHT; s_tick = 0; s_pour = false; }
        break;
    }

    /* KEY-FRAME: glass returns upright (42 ticks) */
    case ANIM_UPRIGHT: {
        float t = smoothStep(clampf((float)s_tick / 42.0f, 0, 1));
        s_tilt   = lerpf(115.0f, 0.0f,   t);
        s_stream = lerpf(185.0f, 0.0f,   t);
        if (s_tick >= 42) { s_state = ANIM_DISAPPEAR; s_tick = 0; }
        break;
    }

    /* KEY-FRAME: glass disappears, scale 1->0 (35 ticks) */
    case ANIM_DISAPPEAR: {
        float t = smoothStep(clampf((float)s_tick / 35.0f, 0, 1));
        s_scale = 1.0f - t;
        if (s_tick >= 35) { s_state = ANIM_ADVANCE_STAGE; s_tick = 0; }
        break;
    }

    /* Advance damage stage then loop */
    case ANIM_ADVANCE_STAGE:
        if (g_currentStage < 5) { g_currentStage++; s_idleWait = 90;  }
        else                    { g_currentStage=0;  g_pourOverlay=0; s_idleWait=150; }
        s_state = ANIM_IDLE; s_tick = 0;
        break;
    }
}

/* ============================================================
 * SECTION 6 - drawAnimation: composite 3D transform (Ch.3)
 *   1. glTranslatef - screen position
 *   2. glRotatef    - tilt angle
 *   3. glScalef     - appear/disappear scale
 * ============================================================ */
void drawAnimation() {
    if (g_activeOrgan != 0) return;
    if (s_state == ANIM_IDLE || s_state == ANIM_ADVANCE_STAGE) return;

    float fill = 1.0f;
    if (s_state == ANIM_POUR)
        fill = 1.0f - clampf((float)s_tick / 85.0f, 0, 1);
    else if (s_state == ANIM_UPRIGHT || s_state == ANIM_DISAPPEAR)
        fill = 0.0f;

    /* Draw Bezier pour stream (Ch.4) */
    if (s_pour || s_state == ANIM_UPRIGHT)
        drawPourStream(s_stream);

    /* Ch.3 composite transform: translate -> rotate -> scale */
    glPushMatrix();
    glTranslatef(s_gx, s_gy, 0.0f);
    glRotatef(s_tilt, 0.0f, 0.0f, 1.0f);
    glScalef(s_scale, s_scale, 1.0f);
    drawWineGlass(fill);
    glPopMatrix();
}