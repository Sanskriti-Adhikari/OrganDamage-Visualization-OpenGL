/*
 * animation.cpp
 * =============
 * Wine-glass animation state machine.
 *
 * Glass target position is above the liver centre at (400, 360).
 * Glass is drawn ~40% larger than a minimal wine glass.
 *
 * Algorithms from syllabus:
 *   - 3D Transforms: glTranslatef, glRotatef, glScalef (Ch.3)
 *   - Key-frame smooth-step easing (Ch.7)
 *   - Quadratic Bezier pour stream (Ch.4)
 *   - Parametric ellipse for bowl and base (Ch.2 mid-point ellipse)
 */

#include <GL/glut.h>
#include <math.h>

extern int   g_currentStage;
extern float g_pourOverlay;
extern float lerpf(float,float,float);
extern float clampf(float,float,float);
extern float smoothStep(float);
extern void  bezierQuad(float,float,float,float,float,float,float,float*,float*);

enum AnimState {
    ANIM_IDLE=0, ANIM_APPEAR, ANIM_MOVE_ABOVE,
    ANIM_ROTATE_TILT, ANIM_POUR, ANIM_UPRIGHT,
    ANIM_DISAPPEAR, ANIM_ADVANCE_STAGE
};

static AnimState s_state    = ANIM_IDLE;
static int       s_tick     = 0;
static int       s_idleWait = 0;

static float s_gx     = 820.0f;
static float s_gy     = 570.0f;
static float s_scale  = 0.0f;
static float s_tilt   = 0.0f;
static float s_stream = 0.0f;
static bool  s_pour   = false;

static const float SX = 820.0f, SY = 570.0f;   /* start position */
static const float TX = 438.0f, TY = 490.0f;   /* target — above liver */

void resetSimulation() {
    g_currentStage = 0;
    g_pourOverlay  = 0.0f;
    s_state = ANIM_IDLE; s_tick = 0; s_idleWait = 80;
    s_gx = SX; s_gy = SY;
    s_scale = 0.0f; s_tilt = 0.0f; s_stream = 0.0f; s_pour = false;
}

/*
 * drawWineGlass()
 * ---------------
 * Wine glass drawn in local coordinates.
 * Mid-point ellipse concept for base and rim (Ch.2).
 * Scale is 1.5× the minimal glass for visibility.
 */
static void drawWineGlass(float fill) {
    const float PI = 3.14159265f;

    /* Base ellipse */
    glColor4f(0.70f, 0.86f, 0.93f, 0.70f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, -115.0f);
    for (int i = 0; i <= 32; i++) {
        float a = (float)i/32.0f * 2.0f * PI;
        glVertex2f(46.0f*cosf(a), -115.0f + 8.0f*sinf(a));
    }
    glEnd();

    /* Stem */
    glColor4f(0.66f, 0.84f, 0.91f, 0.64f);
    glBegin(GL_QUADS);
    glVertex2f(-6.0f, -112.0f); glVertex2f( 6.0f, -112.0f);
    glVertex2f( 9.0f,  -60.0f); glVertex2f(-9.0f,  -60.0f);
    glEnd();

    /* Bowl back */
    glColor4f(0.56f, 0.76f, 0.85f, 0.54f);
    glBegin(GL_QUADS);
    glVertex2f(-26.0f, -60.0f); glVertex2f( 26.0f, -60.0f);
    glVertex2f( 48.0f,  40.0f); glVertex2f(-48.0f,  40.0f);
    glEnd();

    /* Wine fill */
    if (fill > 0.01f) {
        float bot=-60.0f, top=40.0f;
        float ft = bot + (top-bot)*fill;
        float t  = (ft-bot)/(top-bot);
        float wf = lerpf(26.0f, 48.0f, t);
        glColor4f(0.55f, 0.04f, 0.08f, 0.78f);
        glBegin(GL_QUADS);
        glVertex2f(-26.0f, bot); glVertex2f(26.0f, bot);
        glVertex2f( wf,    ft);  glVertex2f(-wf,   ft);
        glEnd();
    }

    /* Bowl outline */
    glColor4f(0.84f, 0.94f, 1.00f, 0.84f);
    glLineWidth(2.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-26.0f,-60.0f); glVertex2f(-48.0f, 40.0f);
    glVertex2f( 48.0f, 40.0f); glVertex2f( 26.0f,-60.0f);
    glEnd();

    /* Rim ellipse */
    glColor4f(0.88f, 0.96f, 1.00f, 0.80f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 32; i++) {
        float a = (float)i/32.0f * 2.0f * PI;
        glVertex2f(48.0f*cosf(a), 40.0f + 9.0f*sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);
}

/* Bezier pour stream */
static void drawPourStream(float len) {
    if (len < 1.0f) return;
    const float PI = 3.14159265f;
    float ang = (s_tilt + 90.0f) * PI / 180.0f;
    float tx = s_gx + 50.0f*cosf(ang);
    float ty = s_gy + 50.0f*sinf(ang);
    float ex = tx + 16.0f, ey = ty - len;
    float cx = tx + 30.0f, cy = ty - len * 0.5f;

    glLineWidth(5.5f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++) {
        float t = (float)i/24.0f;
        float bx, by;
        bezierQuad(tx,ty,cx,cy,ex,ey,t,&bx,&by);
        glColor4f(0.58f, 0.04f, 0.08f, 0.90f*(1.0f-t*0.45f));
        glVertex2f(bx, by);
    }
    glEnd();
    glLineWidth(1.0f);
}

/* State machine */
void updateAnimation(int) {
    s_tick++;
    switch (s_state) {

    case ANIM_IDLE:
        if (--s_idleWait <= 0) {
            s_state=ANIM_APPEAR; s_tick=0;
            s_gx=SX; s_gy=SY; s_scale=0.0f;
            s_tilt=0.0f; s_stream=0.0f; g_pourOverlay=0.0f;
        }
        break;

    case ANIM_APPEAR: {
        s_scale = smoothStep(clampf((float)s_tick/40.0f,0.0f,1.0f));
        if (s_tick>=40) { s_state=ANIM_MOVE_ABOVE; s_tick=0; }
        break;
    }
    case ANIM_MOVE_ABOVE: {
        float t = smoothStep(clampf((float)s_tick/75.0f,0.0f,1.0f));
        s_gx = lerpf(SX,TX,t); s_gy = lerpf(SY,TY,t);
        if (s_tick>=75) { s_state=ANIM_ROTATE_TILT; s_tick=0; }
        break;
    }
    case ANIM_ROTATE_TILT: {
        float t = smoothStep(clampf((float)s_tick/50.0f,0.0f,1.0f));
        s_tilt = lerpf(0.0f,115.0f,t);
        if (s_tick>=50) { s_state=ANIM_POUR; s_tick=0; s_pour=true; }
        break;
    }
    case ANIM_POUR: {
        float t = clampf((float)s_tick/85.0f,0.0f,1.0f);
        s_stream = lerpf(0.0f,180.0f,t);
        g_pourOverlay = lerpf(0.0f,1.0f,t);
        if (s_tick>=85) { s_state=ANIM_UPRIGHT; s_tick=0; s_pour=false; }
        break;
    }
    case ANIM_UPRIGHT: {
        float t = smoothStep(clampf((float)s_tick/42.0f,0.0f,1.0f));
        s_tilt  = lerpf(115.0f,0.0f,t);
        s_stream = lerpf(180.0f,0.0f,t);
        if (s_tick>=42) { s_state=ANIM_DISAPPEAR; s_tick=0; }
        break;
    }
    case ANIM_DISAPPEAR: {
        float t = smoothStep(clampf((float)s_tick/35.0f,0.0f,1.0f));
        s_scale = 1.0f - t;
        if (s_tick>=35) { s_state=ANIM_ADVANCE_STAGE; s_tick=0; }
        break;
    }
    case ANIM_ADVANCE_STAGE:
        if (g_currentStage < 5) { g_currentStage++; s_idleWait=90; }
        else { g_currentStage=0; g_pourOverlay=0.0f; s_idleWait=150; }
        s_state=ANIM_IDLE; s_tick=0;
        break;
    }
}

/*
 * drawAnimation()
 * ---------------
 * Composite 3D transform applied to glass (Ch.3):
 *   1. glTranslatef — screen position
 *   2. glRotatef    — tilt for pour
 *   3. glScalef     — appear/disappear
 */
void drawAnimation() {
    if (s_state==ANIM_IDLE || s_state==ANIM_ADVANCE_STAGE) return;

    float fill = 1.0f;
    if (s_state==ANIM_POUR)
        fill = 1.0f - clampf((float)s_tick/85.0f, 0.0f, 1.0f);
    else if (s_state==ANIM_UPRIGHT || s_state==ANIM_DISAPPEAR)
        fill = 0.0f;

    if (s_pour || s_state==ANIM_UPRIGHT) drawPourStream(s_stream);

    glPushMatrix();
    glTranslatef(s_gx, s_gy, 0.0f);
    glRotatef(s_tilt, 0.0f, 0.0f, 1.0f);
    glScalef(s_scale, s_scale, 1.0f);
    drawWineGlass(fill);
    glPopMatrix();
}