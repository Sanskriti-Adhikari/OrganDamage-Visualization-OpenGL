/*
 * main.cpp
 * ========
 * Entry point for the Organ Damage Visualizer.
 * Window: 1100 x 700 pixels.
 *
 * Controls:
 *   TAB = switch between Liver / Lungs organ view
 *   R   = restart animation from Stage 1
 *   ESC = quit
 *
 * Algorithms from syllabus:
 *   Ch.3 - gluOrtho2D sets up 2D viewing pipeline
 *   Ch.7 - glutTimerFunc drives key-frame animation loop
 */

#include <GL/glut.h>
#include <stdio.h>
#include "globals.h"

/* ============================================================
 * SECTION 1 - Forward declarations
 * ============================================================ */
void drawLiver();         void drawLungsOrgan();
void drawAnimation();     void drawLungsAnimation();
void drawUI();            void drawSideBar();    void drawBottomBar();
void initStages();        void resetSimulation();
void updateAnimation(int);void updateLungsAnimation(int);

static const int FPS_MS = 16;  /* ~62 fps timer interval */

/* ============================================================
 * SECTION 2 - Display callback
 *   Ch.3 - gluOrtho2D: 2D window-to-viewport transform
 *   Draws active organ then shared UI panels on top.
 * ============================================================ */
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Ch.3 - 2D orthographic projection (window-to-viewport) */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Draw active organ and its animation object */
    if (g_activeOrgan == 0) {
        drawLiver();       /* liver silhouette      */
        drawAnimation();   /* wine glass animation  */
    } else {
        drawLungsOrgan();      /* lung lobes            */
        drawLungsAnimation();  /* cigarette animation   */
    }

    /* Shared UI panels drawn on top of organ */
    drawUI();        /* top title bar + organ tabs  */
    drawSideBar();   /* right stage progress boxes  */
    drawBottomBar(); /* bottom spectrum + info      */

    glutSwapBuffers();
}

/* ============================================================
 * SECTION 3 - Reshape callback
 * ============================================================ */
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

/* ============================================================
 * SECTION 4 - Timer callback (Ch.7 key-frame animation driver)
 *   Runs both state machines every FPS_MS milliseconds.
 * ============================================================ */
void timerCB(int v) {
    updateAnimation(v);       /* liver wine-glass state machine  */
    updateLungsAnimation(v);  /* lungs cigarette state machine   */
    glutPostRedisplay();
    glutTimerFunc(FPS_MS, timerCB, v + 1);
}

/* ============================================================
 * SECTION 5 - Keyboard callback
 * ============================================================ */
void keyboard(unsigned char k, int, int) {
    if (k == 27)             exit(0);            /* ESC - quit          */
    if (k == 'r' || k=='R')  resetSimulation();  /* R   - restart       */
    if (k == '\t') {                              /* TAB - switch organ  */
        g_activeOrgan = 1 - g_activeOrgan;
        resetSimulation();
    }
}

/* ============================================================
 * SECTION 6 - main: GLUT setup and event loop
 * ============================================================ */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Organ Damage Visualizer - ENCT 201 CG Project");

    /* OpenGL state */
    glClearColor(0.11f, 0.07f, 0.09f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    /* Initialise stage data and animation */
    initStages();
    resetSimulation();

    /* Register GLUT callbacks */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(FPS_MS, timerCB, 0);

    printf("==============================================\n");
    printf("  Organ Damage Visualizer - ENCT 201\n");
    printf("  TAB = switch organ\n");
    printf("  R   = restart\n");
    printf("  ESC = quit\n");
    printf("==============================================\n");

    glutMainLoop();
    return 0;
}