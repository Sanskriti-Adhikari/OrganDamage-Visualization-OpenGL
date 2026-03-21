/*
 * geometry.h  —  Bezier curves & Catmull-Rom splines
 *
 * Algorithms:
 *   Cubic Bezier    (Ch.4)  B(t)=(1-t)^3 P0 + 3(1-t)^2t P1 + ...
 *   Catmull-Rom     (Ch.4)  Hermite basis, tension = 0.5
 *
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once
#include <vector>
#include <cmath>

struct G_Point { float x, y; };

/* ── Cubic Bezier ──────────────────────────────────────────── */
inline std::vector<float> G_bezier(
    float x0,float y0, float x1,float y1,
    float x2,float y2, float x3,float y3, int segs=24)
{
    std::vector<float> pts;
    pts.reserve((segs+1)*2);
    for(int i=0;i<=segs;i++){
        float t=(float)i/segs, u=1.f-t;
        pts.push_back(u*u*u*x0+3*u*u*t*x1+3*u*t*t*x2+t*t*t*x3);
        pts.push_back(u*u*u*y0+3*u*u*t*y1+3*u*t*t*y2+t*t*t*y3);
    }
    return pts;
}

/* ── Catmull-Rom — returns flat xy fan array ───────────────── */
/* centX/centY are in LOCAL space (before scale/mirror) */
inline std::vector<float> G_catmullRomFan(
    const std::vector<G_Point>& kp,
    float centX, float centY,
    int segsPerSpan=14)
{
    int N=(int)kp.size();
    std::vector<float> out;
    out.push_back(centX);
    out.push_back(centY);
    for(int i=0;i<N-1;i++){
        G_Point p0=kp[(i-1+N)%N], p1=kp[i],
                p2=kp[i+1],       p3=kp[(i+2)%N];
        for(int j=0;j<segsPerSpan;j++){
            float s=(float)j/segsPerSpan, s2=s*s, s3=s2*s;
            float h1=0.5f*(-s3+2*s2-s);
            float h2=0.5f*(3*s3-5*s2+2);
            float h3=0.5f*(-3*s3+4*s2+s);
            float h4=0.5f*(s3-s2);
            out.push_back(h1*p0.x+h2*p1.x+h3*p2.x+h4*p3.x);
            out.push_back(h1*p0.y+h2*p1.y+h3*p2.y+h4*p3.y);
        }
    }
    out.push_back(out[2]); out.push_back(out[3]); // close
    return out;
}