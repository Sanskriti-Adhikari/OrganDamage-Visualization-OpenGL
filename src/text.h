/*
 * text.h  —  Stroke-based text renderer (no external library)
 *
 * Characters are drawn as GL_LINES segments in a 4-wide x 7-tall grid.
 * Lowercase is automatically converted to uppercase.
 *
 * API:
 *   T_drawText(x, y, charHeight, "Hello World")   — left-aligned
 *   T_drawCentred(cx, y, charHeight, "Hello")      — centred on cx
 *   T_drawRight(rx, y, charHeight, "Hello")        — right-aligned
 *   T_width(str, charHeight)                       — measure width
 *
 * Tribhuvan University – Organ Damage Visualization
 */
#pragma once
#include "renderer.h"
#include <cstring>
#include <cctype>
#include <cstdint>

struct GSeg { int8_t x0,y0,x1,y1; };
struct Glyph { GSeg s[12]; int n; };

#define G(ax,ay,bx,by) {(int8_t)(ax),(int8_t)(ay),(int8_t)(bx),(int8_t)(by)}

/* All chars defined for uppercase A-Z, 0-9, common symbols.
   Lowercase is mapped to uppercase before lookup.              */
static Glyph T_glyph(char c) {
    /* map lowercase -> uppercase */
    if (c >= 'a' && c <= 'z') c = (char)(c - 32);

    switch(c) {
    /* ── UPPERCASE ─────────────────────────────── */
    case 'A': return {{G(0,0,2,7),G(2,7,4,0),G(1,3,3,3)},3};
    case 'B': return {{G(0,0,0,7),G(0,7,3,7),G(3,7,4,6),G(4,6,3,4),
                       G(3,4,0,4),G(3,4,4,2),G(4,2,3,0),G(3,0,0,0)},8};
    case 'C': return {{G(4,6,3,7),G(3,7,1,7),G(1,7,0,6),G(0,6,0,1),
                       G(0,1,1,0),G(1,0,3,0),G(3,0,4,1)},7};
    case 'D': return {{G(0,0,0,7),G(0,7,2,7),G(2,7,4,5),G(4,5,4,2),
                       G(4,2,2,0),G(2,0,0,0)},6};
    case 'E': return {{G(0,0,0,7),G(0,7,4,7),G(0,4,3,4),G(0,0,4,0)},4};
    case 'F': return {{G(0,0,0,7),G(0,7,4,7),G(0,4,3,4)},3};
    case 'G': return {{G(4,6,3,7),G(3,7,1,7),G(1,7,0,6),G(0,6,0,1),
                       G(0,1,1,0),G(1,0,4,0),G(4,0,4,3),G(2,3,4,3)},8};
    case 'H': return {{G(0,0,0,7),G(4,0,4,7),G(0,4,4,4)},3};
    case 'I': return {{G(1,0,3,0),G(2,0,2,7),G(1,7,3,7)},3};
    case 'J': return {{G(0,2,1,0),G(1,0,3,0),G(3,0,3,7),G(1,7,3,7)},4};
    case 'K': return {{G(0,0,0,7),G(4,7,0,4),G(0,4,4,0)},3};
    case 'L': return {{G(0,7,0,0),G(0,0,4,0)},2};
    case 'M': return {{G(0,0,0,7),G(0,7,2,4),G(2,4,4,7),G(4,7,4,0)},4};
    case 'N': return {{G(0,0,0,7),G(0,7,4,0),G(4,0,4,7)},3};
    case 'O': return {{G(1,0,3,0),G(3,0,4,1),G(4,1,4,6),G(4,6,3,7),
                       G(3,7,1,7),G(1,7,0,6),G(0,6,0,1),G(0,1,1,0)},8};
    case 'P': return {{G(0,0,0,7),G(0,7,3,7),G(3,7,4,6),G(4,6,3,4),G(3,4,0,4)},5};
    case 'Q': return {{G(1,0,3,0),G(3,0,4,1),G(4,1,4,6),G(4,6,3,7),
                       G(3,7,1,7),G(1,7,0,6),G(0,6,0,1),G(0,1,1,0),G(3,2,4,0)},9};
    case 'R': return {{G(0,0,0,7),G(0,7,3,7),G(3,7,4,6),G(4,6,3,4),
                       G(3,4,0,4),G(2,4,4,0)},6};
    case 'S': return {{G(4,6,3,7),G(3,7,1,7),G(1,7,0,6),G(0,6,0,4),
                       G(0,4,4,3),G(4,3,4,1),G(4,1,3,0),G(3,0,0,0)},8};
    case 'T': return {{G(0,7,4,7),G(2,7,2,0)},2};
    case 'U': return {{G(0,7,0,1),G(0,1,1,0),G(1,0,3,0),G(3,0,4,1),G(4,1,4,7)},5};
    case 'V': return {{G(0,7,2,0),G(2,0,4,7)},2};
    case 'W': return {{G(0,7,1,0),G(1,0,2,4),G(2,4,3,0),G(3,0,4,7)},4};
    case 'X': return {{G(0,7,4,0),G(0,0,4,7)},2};
    case 'Y': return {{G(0,7,2,4),G(4,7,2,4),G(2,4,2,0)},3};
    case 'Z': return {{G(0,7,4,7),G(4,7,0,0),G(0,0,4,0)},3};
    /* ── DIGITS ─────────────────────────────────── */
    case '0': return {{G(1,0,3,0),G(3,0,4,1),G(4,1,4,6),G(4,6,3,7),
                       G(3,7,1,7),G(1,7,0,6),G(0,6,0,1),G(0,1,1,0),G(1,2,3,5)},9};
    case '1': return {{G(1,6,2,7),G(2,7,2,0),G(1,0,3,0)},3};
    case '2': return {{G(0,6,1,7),G(1,7,3,7),G(3,7,4,6),G(4,6,4,5),
                       G(4,5,0,1),G(0,1,0,0),G(0,0,4,0)},7};
    case '3': return {{G(0,6,1,7),G(1,7,3,7),G(3,7,4,6),G(4,6,3,4),
                       G(1,4,3,4),G(3,4,4,2),G(4,2,3,0),G(3,0,0,1)},8};
    case '4': return {{G(0,7,0,4),G(0,4,4,4),G(3,7,3,0)},3};
    case '5': return {{G(4,7,0,7),G(0,7,0,4),G(0,4,3,4),G(3,4,4,3),
                       G(4,3,4,1),G(4,1,3,0),G(3,0,0,0)},7};
    case '6': return {{G(4,7,1,7),G(1,7,0,6),G(0,6,0,1),G(0,1,1,0),
                       G(1,0,3,0),G(3,0,4,1),G(4,1,4,3),G(4,3,0,3)},8};
    case '7': return {{G(0,7,4,7),G(4,7,2,0)},2};
    case '8': return {{G(1,4,0,5),G(0,5,0,6),G(0,6,1,7),G(1,7,3,7),G(3,7,4,6),
                       G(4,6,4,5),G(4,5,3,4),G(3,4,1,4),G(1,4,0,3),G(0,3,0,1),
                       G(0,1,1,0),G(1,0,3,0)},12};
    case '9': return {{G(0,2,1,0),G(1,0,3,0),G(3,0,4,1),G(4,1,4,6),G(4,6,3,7),
                       G(3,7,1,7),G(1,7,0,6),G(0,6,0,4),G(0,4,4,4)},9};
    /* ── SYMBOLS ─────────────────────────────────── */
    case ' ':  return {{},0};
    case '.':  return {{G(1,0,3,0),G(3,0,3,1),G(3,1,1,1),G(1,1,1,0)},4};
    case ',':  return {{G(2,1,2,2),G(2,2,1,2),G(1,2,2,0)},3};
    case '-':  return {{G(0,4,4,4)},1};
    case '+':  return {{G(2,1,2,6),G(0,4,4,4)},2};
    case ':':  return {{G(1,2,2,2),G(2,2,2,3),G(2,3,1,3),G(1,3,1,2),
                        G(1,5,2,5),G(2,5,2,6),G(2,6,1,6),G(1,6,1,5)},8};
    case '|':  return {{G(2,0,2,7)},1};
    case '/':  return {{G(0,0,4,7)},1};
    case '\\': return {{G(0,7,4,0)},1};
    case '(':  return {{G(3,7,1,5),G(1,5,1,2),G(1,2,3,0)},3};
    case ')':  return {{G(1,7,3,5),G(3,5,3,2),G(3,2,1,0)},3};
    case '>':  return {{G(0,6,4,4),G(4,4,0,2)},2};
    case '<':  return {{G(4,6,0,4),G(0,4,4,2)},2};
    case '!':  return {{G(2,2,2,7),G(1,0,2,0),G(2,0,2,1),G(2,1,1,1),G(1,1,1,0)},5};
    case '?':  return {{G(0,6,1,7),G(1,7,3,7),G(3,7,4,6),G(4,6,4,5),G(4,5,2,4),
                        G(2,4,2,2),G(1,1,2,1),G(2,1,2,0),G(2,0,1,0),G(1,0,1,1)},10};
    default:   return {{},0};
    }
}
#undef G

/* ── Metrics ────────────────────────────────────────────────── */
static inline float T_charW(float h) { return h * 0.58f;  }
static inline float T_gap(float h)   { return h * 0.18f;  }
static inline float T_spaceW(float h){ return h * 0.40f;  }

/* ── Measure string width ───────────────────────────────────── */
inline float T_width(const char* str, float charH) {
    if (!str || !str[0]) return 0.f;
    float cw = T_charW(charH), gap = T_gap(charH), sw = T_spaceW(charH);
    float w = 0.f;
    int n = 0;
    for (int i = 0; str[i]; i++) {
        w += (str[i] == ' ') ? sw : cw + gap;
        n++;
    }
    if (n > 0 && str[n-1] != ' ') w -= gap; /* trim trailing gap */
    return w;
}

/* ── Draw one character ─────────────────────────────────────── */
inline void T_drawChar(char c, float cx, float cy, float charH) {
    Glyph g = T_glyph(c);
    if (g.n == 0) return;
    float cw = T_charW(charH);
    glLineWidth(1.8f);
    for (int i = 0; i < g.n; i++) {
        float x0 = cx + g.s[i].x0 / 4.f * cw;
        float y0 = cy + g.s[i].y0 / 7.f * charH;
        float x1 = cx + g.s[i].x1 / 4.f * cw;
        float y1 = cy + g.s[i].y1 / 7.f * charH;
        float v[] = {x0,y0, x1,y1};
        R_draw(GL_LINES, v, 2);
    }
    glLineWidth(1.f);
}

/* ── Draw string, left-aligned at (x, y=baseline) ──────────── */
inline void T_drawText(float x, float y, float charH, const char* str) {
    if (!str) return;
    float cw = T_charW(charH), gap = T_gap(charH), sw = T_spaceW(charH);
    float cur = x;
    for (int i = 0; str[i]; i++) {
        if (str[i] == ' ') { cur += sw; continue; }
        T_drawChar(str[i], cur, y, charH);
        cur += cw + gap;
    }
}

/* ── Draw string centred on cx ──────────────────────────────── */
inline void T_drawCentred(float cx, float y, float charH, const char* str) {
    T_drawText(cx - T_width(str, charH) * 0.5f, y, charH, str);
}

/* ── Draw string right-aligned ending at rx ─────────────────── */
inline void T_drawRight(float rx, float y, float charH, const char* str) {
    T_drawText(rx - T_width(str, charH), y, charH, str);
}