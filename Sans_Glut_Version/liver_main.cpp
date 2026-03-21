/*
 * liver_main.cpp
 * ==============
 * Entry point — 1000 x 680 canvas.
 *
 * Compile (Windows MinGW / MSYS2):
 *   g++ *.cpp -o LiverVisualizer.exe -lfreeglut -lopengl32 -lglu32
 *
 * Controls:  R = restart   ESC = quit
 */

#include <GL/glut.h>
#include <stdio.h>

void drawLiver();
void drawAnimation();
void drawUI();
void drawSideBar();
void drawBottomBar();
void updateAnimation(int);
void initStages();
void resetSimulation();

static const int WIN_W = 1000;
static const int WIN_H = 680;
static const int FPS_MS = 16;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawLiver();
    drawAnimation();
    drawUI();
    drawSideBar();
    drawBottomBar();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

void timerCB(int v) {
    updateAnimation(v);
    glutPostRedisplay();
    glutTimerFunc(FPS_MS, timerCB, v+1);
}

void keyboard(unsigned char k, int, int) {
    if (k == 27)             exit(0);
    if (k=='r' || k=='R')    resetSimulation();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Liver Deterioration - Alcohol Damage Over Time");

    glClearColor(0.11f, 0.07f, 0.09f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    initStages();
    resetSimulation();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(FPS_MS, timerCB, 0);

    printf("===========================================\n");
    printf("  Liver Deterioration Visualizer\n");
    printf("  ENCT 201  Computer Graphics Project\n");
    printf("  Press R to restart  |  ESC to quit\n");
    printf("===========================================\n");

    glutMainLoop();
    return 0;
}