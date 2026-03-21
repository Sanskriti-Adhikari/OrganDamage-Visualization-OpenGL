/*
 * renderer.h  —  Shared OpenGL drawing utilities
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once

/* Windows needs this BEFORE <cmath> to get M_PI */
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <vector>

/* Use our own PI so it is always defined */
static const float R_PI = 3.14159265358979f;

/* ── Colour struct ─────────────────────────────────────────── */
struct Col3 { float r, g, b; };
inline Col3 R_lerp(Col3 a, Col3 b, float t){
    return {a.r+(b.r-a.r)*t, a.g+(b.g-a.g)*t, a.b+(b.b-a.b)*t};
}

/* ── Shader source ─────────────────────────────────────────── */
static const char* R_VERT = R"(
#version 330 core
layout(location=0) in vec2 pos;
uniform mat4 uP;
void main(){ gl_Position = uP * vec4(pos,0.0,1.0); }
)";
static const char* R_FRAG = R"(
#version 330 core
uniform vec4 uCol;
out vec4 fc;
void main(){ fc = uCol; }
)";

/* ── Globals (defined once in renderer.cpp) ────────────────── */
extern GLuint R_prog;
extern GLuint R_vao;
extern GLuint R_vbo;
extern float  R_ortho[16];

/* ── Init / cleanup ────────────────────────────────────────── */
inline GLuint R_mkShader(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    return s;
}
inline void R_init(){
    GLuint vs = R_mkShader(GL_VERTEX_SHADER,   R_VERT);
    GLuint fs = R_mkShader(GL_FRAGMENT_SHADER, R_FRAG);
    R_prog = glCreateProgram();
    glAttachShader(R_prog,vs); glAttachShader(R_prog,fs);
    glLinkProgram(R_prog);
    glDeleteShader(vs); glDeleteShader(fs);
    glGenVertexArrays(1,&R_vao);
    glGenBuffers(1,&R_vbo);
    glUseProgram(R_prog);
}
inline void R_cleanup(){
    glDeleteBuffers(1,&R_vbo);
    glDeleteVertexArrays(1,&R_vao);
    glDeleteProgram(R_prog);
}

/* ── Projection ────────────────────────────────────────────── */
inline void R_buildOrtho(int w, int h){
    float rat=(float)w/h, L=-rat,R_=rat,B=-1.f,T=1.f;
    memset(R_ortho,0,sizeof(R_ortho));
    R_ortho[0] = 2.f/(R_-L); R_ortho[5] = 2.f/(T-B);
    R_ortho[10]=-1.f;         R_ortho[15]= 1.f;
    R_ortho[12]=-(R_+L)/(R_-L); R_ortho[13]=-(T+B)/(T-B);
}
inline void R_uploadOrtho(){
    glUniformMatrix4fv(glGetUniformLocation(R_prog,"uP"),1,GL_FALSE,R_ortho);
}

/* ── Low-level draw ────────────────────────────────────────── */
inline void R_draw(GLenum mode, const float* v, int n){
    glBindVertexArray(R_vao);
    glBindBuffer(GL_ARRAY_BUFFER,R_vbo);
    glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(n*2*sizeof(float)),v,GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*(GLsizei)sizeof(float),(void*)0);
    glDrawArrays(mode,0,n);
}
inline void R_draw(GLenum mode, const std::vector<float>& v){
    if(!v.empty()) R_draw(mode,v.data(),(int)v.size()/2);
}

/* ── Colour ────────────────────────────────────────────────── */
inline void R_col(float r,float g,float b,float a=1.f){
    glUniform4f(glGetUniformLocation(R_prog,"uCol"),r,g,b,a);
}
inline void R_col(Col3 c,float a=1.f){ R_col(c.r,c.g,c.b,a); }

/* ── Shapes ────────────────────────────────────────────────── */
inline void R_filledEllipse(float cx,float cy,float rx,float ry,int seg=64){
    std::vector<float> v;
    v.push_back(cx); v.push_back(cy);
    for(int i=0;i<=seg;i++){
        float a=2.f*R_PI*i/seg;
        v.push_back(cx+rx*cosf(a)); v.push_back(cy+ry*sinf(a));
    }
    R_draw(GL_TRIANGLE_FAN,v);
}
inline void R_outlineEllipse(float cx,float cy,float rx,float ry,int seg=64){
    std::vector<float> v;
    for(int i=0;i<seg;i++){
        float a=2.f*R_PI*i/seg;
        v.push_back(cx+rx*cosf(a)); v.push_back(cy+ry*sinf(a));
    }
    R_draw(GL_LINE_LOOP,v);
}
inline void R_filledRect(float x1,float y1,float x2,float y2){
    float v[]={x1,y1,x2,y1,x2,y2,x1,y2};
    R_draw(GL_TRIANGLE_FAN,v,4);
}
inline void R_outlineRect(float x1,float y1,float x2,float y2){
    float v[]={x1,y1,x2,y1,x2,y2,x1,y2};
    R_draw(GL_LINE_LOOP,v,4);
}
inline void R_lineStrip(const float* v,int n){ R_draw(GL_LINE_STRIP,v,n); }
inline void R_lineStrip(const std::vector<float>& v){ R_draw(GL_LINE_STRIP,v); }
