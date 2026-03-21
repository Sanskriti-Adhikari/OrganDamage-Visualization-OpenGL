// /*
//  * main.cpp  —  Dashboard + window loop
//  *
//  * Compile (from src/ folder on Windows):
//  *   g++ main.cpp renderer.cpp geometry.cpp animation.cpp particles.cpp ^
//  *       lighting.cpp lungs.cpp liver.cpp glad.c ^
//  *       -o organ_viz ^
//  *       -I../include -L../lib -lglfw3dll -lopengl32 -lgdi32 ^
//  *       -std=c++17 -O2
//  *
//  * Controls (inside simulation):
//  *   LEFT / RIGHT  —  change stage manually
//  *   SPACE         —  toggle auto-advance
//  *   R             —  reset to stage 1
//  *   ESC           —  back to dashboard (ESC again to quit)
//  *
//  * Authors: Sanskriti Adhikari (081BCT075)
//  *          Tejaswi Acharya    (081BCT088)
//  * Tribhuvan University – Organ Damage Visualization
//  */

// #ifndef _USE_MATH_DEFINES
// #define _USE_MATH_DEFINES
// #endif

// #include "renderer.h"
// #include "geometry.h"
// #include "lungs.h"
// #include "liver.h"
// #include "text.h"
// #include <cmath>

// /* ── Scene ──────────────────────────────────────────────────── */
// enum class Scene { DASHBOARD, LUNGS, LIVER };
// static Scene gScene = Scene::DASHBOARD;

// static LungSimulation  gLungs;
// static LiverSimulation gLiver;

// static int    gWinW=1200, gWinH=720;
// static float  gTime=0.f;
// static double gMouseX=0.0, gMouseY=0.0;

// /* ── Coordinate helpers ─────────────────────────────────────── */
// static void toWorld(double px,double py,float& wx,float& wy){
//     float ratio=(float)gWinW/gWinH;
//     wx=((float)px/gWinW*2.f-1.f)*ratio;
//     wy= 1.f-(float)py/gWinH*2.f;
// }
// static bool inBox(float wx,float wy,float x1,float y1,float x2,float y2){
//     return wx>=x1&&wx<=x2&&wy>=y1&&wy<=y2;
// }

// /* ── Dashboard layout ───────────────────────────────────────── */
// static const float CW=0.74f, CH=0.54f;
// static const float CY1=-CH/2.f, CY2=CH/2.f;
// static const float LGX1=-0.88f, LGX2=LGX1+CW;   /* lung card  */
// static const float LVX1= 0.14f, LVX2=LVX1+CW;   /* liver card */

// /* ── Mini organ icons ───────────────────────────────────────── */
// static void drawMiniLung(float cx,float cy,float sc,int side,float t){
//     float m=(float)side;
//     float b=1.f+0.04f*sinf(t*1.1f);
//     R_col(0.82f,0.20f,0.22f,0.90f);
//     R_filledEllipse(cx+0.05f*m*sc,cy-0.02f*sc,0.20f*sc,0.28f*sc*b,48);
//     R_col(0.90f,0.30f,0.30f,0.45f);
//     R_filledEllipse(cx+0.06f*m*sc,cy+0.16f*sc,0.13f*sc,0.13f*sc,32);
//     R_col(0.50f,0.08f,0.09f); glLineWidth(1.5f);
//     R_outlineEllipse(cx+0.05f*m*sc,cy-0.02f*sc,0.20f*sc,0.28f*sc,48);
//     glLineWidth(1.f);
//     R_col(0.88f,0.85f,0.82f,0.65f); glLineWidth(2.f);
//     float b1[]={cx+0.02f*m*sc,cy+0.20f*sc,cx+0.05f*m*sc,cy+0.06f*sc,cx+0.08f*m*sc,cy-0.10f*sc};
//     R_lineStrip(b1,3);
//     float b2[]={cx+0.05f*m*sc,cy+0.06f*sc,cx+0.14f*m*sc,cy+0.05f*sc};
//     R_lineStrip(b2,2);
//     glLineWidth(1.f);
// }
// static void drawMiniTrachea(float cx,float cy,float sc){
//     R_col(0.88f,0.86f,0.83f,0.80f); glLineWidth(4.f);
//     float tr[]={cx,cy+0.36f*sc,cx,cy+0.20f*sc};
//     R_lineStrip(tr,2);
//     auto lb=G_bezier(cx,cy+0.20f*sc,cx-0.02f*sc,cy+0.14f*sc,
//                      cx-0.23f*sc,cy+0.16f*sc,cx-0.25f*sc,cy+0.14f*sc);
//     auto rb=G_bezier(cx,cy+0.20f*sc,cx+0.02f*sc,cy+0.14f*sc,
//                      cx+0.23f*sc,cy+0.16f*sc,cx+0.25f*sc,cy+0.14f*sc);
//     glLineWidth(3.5f);
//     R_lineStrip(lb); R_lineStrip(rb);
//     glLineWidth(1.f);
// }
// static void drawMiniLiver(float cx,float cy,float sc){
//     R_col(0.72f,0.25f,0.08f,0.90f);
//     R_filledEllipse(cx-0.04f*sc,cy,0.28f*sc,0.18f*sc,48);
//     R_filledEllipse(cx+0.18f*sc,cy+0.04f*sc,0.12f*sc,0.10f*sc,32);
//     R_col(0.90f,0.45f,0.18f,0.35f);
//     R_filledEllipse(cx-0.10f*sc,cy+0.06f*sc,0.12f*sc,0.07f*sc,32);
//     R_col(0.45f,0.12f,0.04f); glLineWidth(1.5f);
//     R_outlineEllipse(cx-0.04f*sc,cy,0.28f*sc,0.18f*sc,48);
//     glLineWidth(1.f);
// }

// /* ── Dashboard card ─────────────────────────────────────────── */
// static void drawCard(float x1,float y1,float x2,float y2,
//                      bool hov, Col3 accent)
// {
//     float br=hov?1.20f:1.f;
//     R_col(0,0,0,0.50f);
//     R_filledRect(x1+0.018f,y1-0.018f,x2+0.018f,y2-0.018f);
//     R_col(accent.r*0.18f*br,accent.g*0.18f*br,accent.b*0.18f*br);
//     R_filledRect(x1,y1,x2,y2);
//     R_col(accent.r*0.32f*br,accent.g*0.32f*br,accent.b*0.32f*br,0.60f);
//     R_filledRect(x1,y2-0.12f,x2,y2);
//     R_col(accent.r*br,accent.g*br,accent.b*br,0.80f);
//     R_filledRect(x1,y1,x2,y1+0.025f);
//     R_col(accent.r*br,accent.g*br,accent.b*br,hov?0.90f:0.35f);
//     glLineWidth(hov?2.5f:1.5f);
//     R_outlineRect(x1,y1,x2,y2);
//     glLineWidth(1.f);
// }

// /* ── Draw dashboard ─────────────────────────────────────────── */
// static void drawDashboard(){
//     R_uploadOrtho();
//     float ratio=(float)gWinW/gWinH;

//     /* Background */
//     R_col(0.06f,0.05f,0.09f);
//     R_filledRect(-ratio,-1.f,ratio,1.f);

//     /* Animated ambient glows */
//     float p1=0.5f+0.5f*sinf(gTime*0.7f);
//     float p2=0.5f+0.5f*sinf(gTime*0.5f+1.f);
//     R_col(0.30f,0.08f,0.08f,0.06f*p1);
//     R_filledEllipse(-0.5f,0.1f,0.80f,0.55f,64);
//     R_col(0.08f,0.20f,0.35f,0.06f*p2);
//     R_filledEllipse(0.5f,-0.1f,0.80f,0.55f,64);

//     /* Header */
//     R_col(0.10f,0.09f,0.14f);
//     R_filledRect(-ratio,0.82f,ratio,1.f);
//     R_col(0.35f,0.25f,0.55f,0.80f);
//     R_filledRect(-ratio,0.80f,ratio,0.83f);
//     /* Title bar (text proxy) */
//     R_col(0.80f,0.76f,0.95f);
//     R_filledRect(-0.50f,0.872f,0.50f,0.888f);
//     R_col(0.50f,0.48f,0.62f,0.70f);
//     R_filledRect(-0.28f,0.878f,0.28f,0.884f);
//     /* Subtitle */
//     R_col(0.42f,0.40f,0.54f,0.70f);
//     R_filledRect(-0.30f,0.842f,0.30f,0.852f);

//     /* Footer */
//     R_col(0.08f,0.07f,0.12f);
//     R_filledRect(-ratio,-1.f,ratio,-0.82f);
//     R_col(0.22f,0.20f,0.32f,0.60f);
//     R_filledRect(-ratio,-0.83f,ratio,-0.80f);
//     R_col(0.28f,0.26f,0.38f,0.70f);
//     R_filledRect(-0.55f,-0.93f,0.55f,-0.920f);

//     float wx,wy;
//     toWorld(gMouseX,gMouseY,wx,wy);
//     bool hovLung  = inBox(wx,wy,LGX1,CY1,LGX2,CY2);
//     bool hovLiver = inBox(wx,wy,LVX1,CY1,LVX2,CY2);

//     /* ── LUNGS card ─────────────────────────────────────────── */
//     Col3 la={0.75f,0.16f,0.18f};
//     drawCard(LGX1,CY1,LGX2,CY2,hovLung,la);

//     float lcx=(LGX1+LGX2)*0.5f, lcy=(CY1+CY2)*0.5f+0.02f, lsc=0.46f;
//     drawMiniTrachea(lcx,lcy,lsc);
//     drawMiniLung(lcx-0.10f*lsc,lcy,lsc,-1,gTime);
//     drawMiniLung(lcx+0.10f*lsc,lcy,lsc, 1,gTime);

//     /* Card labels */
//     float lb=hovLung?1.f:0.78f;
//     R_col(lb,lb*0.85f,lb*0.85f);
//     R_filledRect(LGX1+0.06f,CY1+0.082f,LGX1+0.30f,CY1+0.096f);
//     R_col(0.72f,0.55f,0.55f,0.70f);
//     R_filledRect(LGX1+0.06f,CY1+0.057f,LGX1+0.38f,CY1+0.068f);
//     if(hovLung){
//         R_col(la.r,la.g,la.b,0.90f);
//         R_filledRect(LGX1+0.06f,CY1+0.022f,LGX1+0.22f,CY1+0.038f);
//     }

//     /* ── LIVER card ─────────────────────────────────────────── */
//     Col3 lva={0.62f,0.28f,0.06f};
//     drawCard(LVX1,CY1,LVX2,CY2,hovLiver,lva);

//     drawMiniLiver((LVX1+LVX2)*0.5f,(CY1+CY2)*0.5f+0.04f,0.56f);

//     float lvb=hovLiver?1.f:0.78f;
//     R_col(lvb,lvb*0.75f,lvb*0.60f);
//     R_filledRect(LVX1+0.06f,CY1+0.082f,LVX1+0.22f,CY1+0.096f);
//     R_col(0.72f,0.55f,0.40f,0.70f);
//     R_filledRect(LVX1+0.06f,CY1+0.057f,LVX1+0.38f,CY1+0.068f);
//     if(hovLiver){
//         R_col(lva.r,lva.g,lva.b,0.90f);
//         R_filledRect(LVX1+0.06f,CY1+0.022f,LVX1+0.22f,CY1+0.038f);
//     }
// }

// /* ── Back button ────────────────────────────────────────────── */
// static const float BX1=-1.60f,BY1=0.89f,BX2=-1.05f,BY2=0.98f;
// static void drawBackButton(){
//     float wx,wy; toWorld(gMouseX,gMouseY,wx,wy);
//     bool hov=inBox(wx,wy,BX1,BY1,BX2,BY2);
//     R_col(0.14f,0.12f,0.20f,hov?1.f:0.75f);
//     R_filledRect(BX1,BY1,BX2,BY2);
//     R_col(0.50f,0.45f,0.72f,hov?1.f:0.60f);
//     glLineWidth(1.8f);
//     R_outlineRect(BX1,BY1,BX2,BY2);
//     glLineWidth(1.f);
//     float mid=(BY1+BY2)*0.5f;
//     float arr[]={BX1+0.10f,mid, BX1+0.20f,mid+0.025f, BX1+0.20f,mid-0.025f};
//     R_col(0.85f,0.82f,0.95f,hov?1.f:0.70f);
//     R_draw(GL_TRIANGLE_FAN,arr,3);
//     R_col(0.65f,0.62f,0.80f,hov?0.90f:0.55f);
//     R_filledRect(BX1+0.22f,mid-0.010f,BX1+0.48f,mid+0.010f);
// }

// /* ── GLFW callbacks ─────────────────────────────────────────── */
// static void cbResize(GLFWwindow*,int w,int h){
//     gWinW=w; gWinH=h;
//     glViewport(0,0,w,h);
//     R_buildOrtho(w,h);
//     R_uploadOrtho();
// }
// static void cbCursor(GLFWwindow*,double x,double y){ gMouseX=x; gMouseY=y; }

// static void cbMouse(GLFWwindow*,int btn,int action,int){
//     if(btn!=GLFW_MOUSE_BUTTON_LEFT||action!=GLFW_PRESS) return;
//     float wx,wy; toWorld(gMouseX,gMouseY,wx,wy);
//     if(gScene==Scene::DASHBOARD){
//         if(inBox(wx,wy,LGX1,CY1,LGX2,CY2)){ gLungs.reset(); gScene=Scene::LUNGS; }
//         if(inBox(wx,wy,LVX1,CY1,LVX2,CY2)){ gLiver.reset(); gScene=Scene::LIVER; }
//     } else {
//         if(inBox(wx,wy,BX1,BY1,BX2,BY2)) gScene=Scene::DASHBOARD;
//     }
// }

// static void cbKey(GLFWwindow* win,int key,int,int action,int){
//     if(action!=GLFW_PRESS) return;
//     if(key==GLFW_KEY_ESCAPE){
//         if(gScene!=Scene::DASHBOARD) gScene=Scene::DASHBOARD;
//         else glfwSetWindowShouldClose(win,1);
//     }
//     if(gScene==Scene::LUNGS){
//         if(key==GLFW_KEY_RIGHT) gLungs.onKeyRight();
//         if(key==GLFW_KEY_LEFT)  gLungs.onKeyLeft();
//         if(key==GLFW_KEY_SPACE) gLungs.toggleAuto();
//         if(key==GLFW_KEY_R)     gLungs.reset();
//     }
//     if(gScene==Scene::LIVER){
//         if(key==GLFW_KEY_RIGHT) gLiver.onKeyRight();
//         if(key==GLFW_KEY_LEFT)  gLiver.onKeyLeft();
//         if(key==GLFW_KEY_SPACE) gLiver.toggleAuto();
//         if(key==GLFW_KEY_R)     gLiver.reset();
//     }
// }

// /* ── Main ───────────────────────────────────────────────────── */
// int main(){
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
//     glfwWindowHint(GLFW_SAMPLES,8);

//     GLFWwindow* win=glfwCreateWindow(gWinW,gWinH,
//         "Organ Damage Visualization | Tribhuvan University",
//         nullptr,nullptr);
//     if(!win){ glfwTerminate(); return -1; }

//     glfwMakeContextCurrent(win);
//     glfwSetFramebufferSizeCallback(win,cbResize);
//     glfwSetCursorPosCallback(win,cbCursor);
//     glfwSetMouseButtonCallback(win,cbMouse);
//     glfwSetKeyCallback(win,cbKey);
//     gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

//     glEnable(GL_MULTISAMPLE);
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

//     R_init();
//     R_buildOrtho(gWinW,gWinH);
//     R_uploadOrtho();

//     float last=(float)glfwGetTime();
//     while(!glfwWindowShouldClose(win)){
//         float now=(float)glfwGetTime(), dt=now-last; last=now; gTime+=dt;
//         glClearColor(0.06f,0.05f,0.09f,1.f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         switch(gScene){
//             case Scene::DASHBOARD: drawDashboard(); break;
//             case Scene::LUNGS:
//                 gLungs.update(dt); gLungs.render();
//                 drawBackButton(); break;
//             case Scene::LIVER:
//                 gLiver.update(dt); gLiver.render();
//                 drawBackButton(); break;
//         }
//         glfwSwapBuffers(win);
//         glfwPollEvents();
//     }
//     R_cleanup();
//     glfwTerminate();
//     return 0;
// }


/*
 * main.cpp  —  Dashboard + window loop
 *
 * Compile (from src/ folder on Windows):
 *   g++ main.cpp renderer.cpp geometry.cpp animation.cpp particles.cpp ^
 *       lighting.cpp lungs.cpp liver.cpp glad.c ^
 *       -o organ_viz ^
 *       -I../include -L../lib -lglfw3dll -lopengl32 -lgdi32 ^
 *       -std=c++17 -O2
 *
 * Controls (inside simulation):
 *   LEFT / RIGHT  —  change stage manually
 *   SPACE         —  toggle auto-advance
 *   R             —  reset to stage 1
 *   ESC           —  back to dashboard (ESC again to quit)
 *
 * Authors: Sanskriti Adhikari (081BCT075)
 *          Tejaswi Acharya    (081BCT088)
 * Tribhuvan University – Organ Damage Visualization
 */
 
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
 
#include "renderer.h"
#include "geometry.h"
#include "lungs.h"
#include "liver.h"
#include "text.h"
#include <cmath>
 
/* ── Scene ──────────────────────────────────────────────────── */
enum class Scene { DASHBOARD, LUNGS, LIVER };
static Scene gScene = Scene::DASHBOARD;
 
static LungSimulation  gLungs;
static LiverSimulation gLiver;
 
static int    gWinW=1200, gWinH=720;
static float  gTime=0.f;
static double gMouseX=0.0, gMouseY=0.0;
 
/* ── Coordinate helpers ─────────────────────────────────────── */
static void toWorld(double px,double py,float& wx,float& wy){
    float ratio=(float)gWinW/gWinH;
    wx=((float)px/gWinW*2.f-1.f)*ratio;
    wy= 1.f-(float)py/gWinH*2.f;
}
static bool inBox(float wx,float wy,float x1,float y1,float x2,float y2){
    return wx>=x1&&wx<=x2&&wy>=y1&&wy<=y2;
}
 
/* ── Dashboard layout ───────────────────────────────────────── */
static const float CW=0.74f, CH=0.54f;
static const float CY1=-CH/2.f, CY2=CH/2.f;
static const float LGX1=-0.88f, LGX2=LGX1+CW;   /* lung card  */
static const float LVX1= 0.14f, LVX2=LVX1+CW;   /* liver card */
 
/* ── Mini organ icons ───────────────────────────────────────── */
static void drawMiniLung(float cx,float cy,float sc,int side,float t){
    float m=(float)side;
    float b=1.f+0.04f*sinf(t*1.1f);
    R_col(0.82f,0.20f,0.22f,0.90f);
    R_filledEllipse(cx+0.05f*m*sc,cy-0.02f*sc,0.20f*sc,0.28f*sc*b,48);
    R_col(0.90f,0.30f,0.30f,0.45f);
    R_filledEllipse(cx+0.06f*m*sc,cy+0.16f*sc,0.13f*sc,0.13f*sc,32);
    R_col(0.50f,0.08f,0.09f); glLineWidth(1.5f);
    R_outlineEllipse(cx+0.05f*m*sc,cy-0.02f*sc,0.20f*sc,0.28f*sc,48);
    glLineWidth(1.f);
    R_col(0.88f,0.85f,0.82f,0.65f); glLineWidth(2.f);
    float b1[]={cx+0.02f*m*sc,cy+0.20f*sc,cx+0.05f*m*sc,cy+0.06f*sc,cx+0.08f*m*sc,cy-0.10f*sc};
    R_lineStrip(b1,3);
    float b2[]={cx+0.05f*m*sc,cy+0.06f*sc,cx+0.14f*m*sc,cy+0.05f*sc};
    R_lineStrip(b2,2);
    glLineWidth(1.f);
}
static void drawMiniTrachea(float cx,float cy,float sc){
    R_col(0.88f,0.86f,0.83f,0.80f); glLineWidth(4.f);
    float tr[]={cx,cy+0.36f*sc,cx,cy+0.20f*sc};
    R_lineStrip(tr,2);
    auto lb=G_bezier(cx,cy+0.20f*sc,cx-0.02f*sc,cy+0.14f*sc,
                     cx-0.23f*sc,cy+0.16f*sc,cx-0.25f*sc,cy+0.14f*sc);
    auto rb=G_bezier(cx,cy+0.20f*sc,cx+0.02f*sc,cy+0.14f*sc,
                     cx+0.23f*sc,cy+0.16f*sc,cx+0.25f*sc,cy+0.14f*sc);
    glLineWidth(3.5f);
    R_lineStrip(lb); R_lineStrip(rb);
    glLineWidth(1.f);
}
static void drawMiniLiver(float cx,float cy,float sc){
    R_col(0.72f,0.25f,0.08f,0.90f);
    R_filledEllipse(cx-0.04f*sc,cy,0.28f*sc,0.18f*sc,48);
    R_filledEllipse(cx+0.18f*sc,cy+0.04f*sc,0.12f*sc,0.10f*sc,32);
    R_col(0.90f,0.45f,0.18f,0.35f);
    R_filledEllipse(cx-0.10f*sc,cy+0.06f*sc,0.12f*sc,0.07f*sc,32);
    R_col(0.45f,0.12f,0.04f); glLineWidth(1.5f);
    R_outlineEllipse(cx-0.04f*sc,cy,0.28f*sc,0.18f*sc,48);
    glLineWidth(1.f);
}
 
/* ── Dashboard card ─────────────────────────────────────────── */
static void drawCard(float x1,float y1,float x2,float y2,
                     bool hov, Col3 accent)
{
    float br=hov?1.20f:1.f;
    R_col(0,0,0,0.50f);
    R_filledRect(x1+0.018f,y1-0.018f,x2+0.018f,y2-0.018f);
    R_col(accent.r*0.18f*br,accent.g*0.18f*br,accent.b*0.18f*br);
    R_filledRect(x1,y1,x2,y2);
    R_col(accent.r*0.32f*br,accent.g*0.32f*br,accent.b*0.32f*br,0.60f);
    R_filledRect(x1,y2-0.12f,x2,y2);
    R_col(accent.r*br,accent.g*br,accent.b*br,0.80f);
    R_filledRect(x1,y1,x2,y1+0.025f);
    R_col(accent.r*br,accent.g*br,accent.b*br,hov?0.90f:0.35f);
    glLineWidth(hov?2.5f:1.5f);
    R_outlineRect(x1,y1,x2,y2);
    glLineWidth(1.f);
}
 
/* ── Draw dashboard ─────────────────────────────────────────── */
static void drawDashboard(){
    R_uploadOrtho();
    float ratio=(float)gWinW/gWinH;
 
    /* Background */
    R_col(0.06f,0.05f,0.09f);
    R_filledRect(-ratio,-1.f,ratio,1.f);
 
    /* Animated ambient glows */
    float p1=0.5f+0.5f*sinf(gTime*0.7f);
    float p2=0.5f+0.5f*sinf(gTime*0.5f+1.f);
    R_col(0.30f,0.08f,0.08f,0.06f*p1);
    R_filledEllipse(-0.5f,0.1f,0.80f,0.55f,64);
    R_col(0.08f,0.20f,0.35f,0.06f*p2);
    R_filledEllipse(0.5f,-0.1f,0.80f,0.55f,64);
 
    /* Header */
    R_col(0.10f,0.09f,0.14f);
    R_filledRect(-ratio,0.82f,ratio,1.f);
    R_col(0.35f,0.25f,0.55f,0.80f);
    R_filledRect(-ratio,0.80f,ratio,0.83f);
    /* Title text */
    R_col(0.90f,0.86f,1.00f);
    T_drawCentred(0.f, 0.876f, 0.050f, "ORGAN DAMAGE VISUALIZATION");
    R_col(0.55f,0.52f,0.70f);
    T_drawCentred(0.f, 0.838f, 0.026f, "Tribhuvan University | ENCT 201");
 
    /* Footer */
    R_col(0.08f,0.07f,0.12f);
    R_filledRect(-ratio,-1.f,ratio,-0.82f);
    R_col(0.22f,0.20f,0.32f,0.60f);
    R_filledRect(-ratio,-0.83f,ratio,-0.80f);
    R_col(0.32f,0.30f,0.42f);
    T_drawCentred(0.f,-0.935f,0.022f,"Click a card to explore | ESC to quit");
 
    float wx,wy;
    toWorld(gMouseX,gMouseY,wx,wy);
    bool hovLung  = inBox(wx,wy,LGX1,CY1,LGX2,CY2);
    bool hovLiver = inBox(wx,wy,LVX1,CY1,LVX2,CY2);
 
    /* ── LUNGS card ─────────────────────────────────────────── */
    Col3 la={0.75f,0.16f,0.18f};
    drawCard(LGX1,CY1,LGX2,CY2,hovLung,la);
 
    float lcx=(LGX1+LGX2)*0.5f, lcy=(CY1+CY2)*0.5f+0.02f, lsc=0.46f;
    drawMiniTrachea(lcx,lcy,lsc);
    drawMiniLung(lcx-0.10f*lsc,lcy,lsc,-1,gTime);
    drawMiniLung(lcx+0.10f*lsc,lcy,lsc, 1,gTime);
 
    /* Card labels — LUNGS */
    float lb=hovLung?1.f:0.75f;
    R_col(lb,lb*0.85f,lb*0.85f);
    T_drawText(LGX1+0.06f, CY1+0.082f, 0.034f, "LUNGS");
    R_col(0.72f*lb,0.55f*lb,0.55f*lb);
    T_drawText(LGX1+0.06f, CY1+0.046f, 0.022f, "Smoking Damage");
    if(hovLung){
        R_col(la.r,la.g,la.b,0.90f);
        T_drawText(LGX1+0.06f, CY1+0.016f, 0.020f, "> Click to open");
    }
 
    /* ── LIVER card ─────────────────────────────────────────── */
    Col3 lva={0.62f,0.28f,0.06f};
    drawCard(LVX1,CY1,LVX2,CY2,hovLiver,lva);
 
    drawMiniLiver((LVX1+LVX2)*0.5f,(CY1+CY2)*0.5f+0.04f,0.56f);
 
    float lvb=hovLiver?1.f:0.75f;
    R_col(lvb,lvb*0.75f,lvb*0.60f);
    T_drawText(LVX1+0.06f, CY1+0.082f, 0.034f, "LIVER");
    R_col(0.72f*lvb,0.55f*lvb,0.40f*lvb);
    T_drawText(LVX1+0.06f, CY1+0.046f, 0.022f, "Alcohol Damage");
    if(hovLiver){
        R_col(lva.r,lva.g,lva.b,0.90f);
        T_drawText(LVX1+0.06f, CY1+0.016f, 0.020f, "> Click to open");
    }
}
 
/* ── Back button ────────────────────────────────────────────── */
static const float BX1=-1.60f,BY1=0.89f,BX2=-1.05f,BY2=0.98f;
static void drawBackButton(){
    float wx,wy; toWorld(gMouseX,gMouseY,wx,wy);
    bool hov=inBox(wx,wy,BX1,BY1,BX2,BY2);
    R_col(0.14f,0.12f,0.20f,hov?1.f:0.75f);
    R_filledRect(BX1,BY1,BX2,BY2);
    R_col(0.50f,0.45f,0.72f,hov?1.f:0.60f);
    glLineWidth(1.8f);
    R_outlineRect(BX1,BY1,BX2,BY2);
    glLineWidth(1.f);
    float mid=(BY1+BY2)*0.5f;
    float arr[]={BX1+0.10f,mid, BX1+0.20f,mid+0.025f, BX1+0.20f,mid-0.025f};
    R_col(0.85f,0.82f,0.95f,hov?1.f:0.70f);
    R_draw(GL_TRIANGLE_FAN,arr,3);
    R_col(0.65f,0.62f,0.80f,hov?0.90f:0.55f);
    T_drawText(BX1+0.22f,mid-0.012f,0.024f,"MENU");
}
 
/* ── GLFW callbacks ─────────────────────────────────────────── */
static void cbResize(GLFWwindow*,int w,int h){
    gWinW=w; gWinH=h;
    glViewport(0,0,w,h);
    R_buildOrtho(w,h);
    R_uploadOrtho();
}
static void cbCursor(GLFWwindow*,double x,double y){ gMouseX=x; gMouseY=y; }
 
static void cbMouse(GLFWwindow*,int btn,int action,int){
    if(btn!=GLFW_MOUSE_BUTTON_LEFT||action!=GLFW_PRESS) return;
    float wx,wy; toWorld(gMouseX,gMouseY,wx,wy);
    if(gScene==Scene::DASHBOARD){
        if(inBox(wx,wy,LGX1,CY1,LGX2,CY2)){ gLungs.reset(); gScene=Scene::LUNGS; }
        if(inBox(wx,wy,LVX1,CY1,LVX2,CY2)){ gLiver.reset(); gScene=Scene::LIVER; }
    } else {
        if(inBox(wx,wy,BX1,BY1,BX2,BY2)) gScene=Scene::DASHBOARD;
    }
}
 
static void cbKey(GLFWwindow* win,int key,int,int action,int){
    if(action!=GLFW_PRESS) return;
    if(key==GLFW_KEY_ESCAPE){
        if(gScene!=Scene::DASHBOARD) gScene=Scene::DASHBOARD;
        else glfwSetWindowShouldClose(win,1);
    }
    if(gScene==Scene::LUNGS){
        if(key==GLFW_KEY_RIGHT) gLungs.onKeyRight();
        if(key==GLFW_KEY_LEFT)  gLungs.onKeyLeft();
        if(key==GLFW_KEY_SPACE) gLungs.toggleAuto();
        if(key==GLFW_KEY_R)     gLungs.reset();
    }
    if(gScene==Scene::LIVER){
        if(key==GLFW_KEY_RIGHT) gLiver.onKeyRight();
        if(key==GLFW_KEY_LEFT)  gLiver.onKeyLeft();
        if(key==GLFW_KEY_SPACE) gLiver.toggleAuto();
        if(key==GLFW_KEY_R)     gLiver.reset();
    }
}
 
/* ── Main ───────────────────────────────────────────────────── */
int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,8);
 
    GLFWwindow* win=glfwCreateWindow(gWinW,gWinH,
        "Organ Damage Visualization | Tribhuvan University",
        nullptr,nullptr);
    if(!win){ glfwTerminate(); return -1; }
 
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win,cbResize);
    glfwSetCursorPosCallback(win,cbCursor);
    glfwSetMouseButtonCallback(win,cbMouse);
    glfwSetKeyCallback(win,cbKey);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
 
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 
    R_init();
    R_buildOrtho(gWinW,gWinH);
    R_uploadOrtho();
 
    float last=(float)glfwGetTime();
    while(!glfwWindowShouldClose(win)){
        float now=(float)glfwGetTime(), dt=now-last; last=now; gTime+=dt;
        glClearColor(0.06f,0.05f,0.09f,1.f);
        glClear(GL_COLOR_BUFFER_BIT);
 
        switch(gScene){
            case Scene::DASHBOARD: drawDashboard(); break;
            case Scene::LUNGS:
                gLungs.update(dt); gLungs.render();
                drawBackButton(); break;
            case Scene::LIVER:
                gLiver.update(dt); gLiver.render();
                drawBackButton(); break;
        }
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    R_cleanup();
    glfwTerminate();
    return 0;
}