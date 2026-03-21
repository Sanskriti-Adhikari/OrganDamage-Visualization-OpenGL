/*
 * lighting.h  —  Gouraud shading colour helpers (Ch.6)
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once
#include "renderer.h"

/* Lighter variant of a colour (highlight) */
inline Col3 L_highlight(Col3 c, float s=1.35f){
    auto clamp=[](float v){ return v>1.f?1.f:v; };
    return {clamp(c.r*s),clamp(c.g*s),clamp(c.b*s)};
}
/* Darker variant (shadow) */
inline Col3 L_shadow(Col3 c, float s=0.50f){
    return {c.r*s, c.g*s, c.b*s};
}