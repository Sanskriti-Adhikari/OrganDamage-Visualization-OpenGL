# Organ Damage Visualizer
### ENCT 201 - Computer Graphics and Visualization
**IOE, Tribhuvan University | Year II, Part I**

A real-time OpenGL/C++ simulation visualizing progressive organ damage across 6 stages.
- **Liver** — alcohol damage with animated wine glass pouring
- **Lungs** — smoking damage with animated cigarette and smoke

---

## File Structure

All files in one flat folder — no subfolders:

```
Organ_Visualization/
├── Makefile
├── README.md
├── .gitignore
├── globals.h
├── main.cpp
├── utils.cpp
├── ui.cpp
├── side_bar.cpp
├── bottom_bar.cpp
├── liver.cpp
├── animation.cpp
├── lungs.cpp
└── lungs_animation.cpp
```

---

## Prerequisites

- **MSYS2** from https://www.msys2.org/
- **MinGW64** at `D:/Mingw/mingw64/`
- **FreeGLUT** at `D:/Mingw/freeglut-mingw-master/`

---

## Build and Run

Open **MSYS2 MinGW 64-bit** terminal:

```bash
cd /d/Mingw/Organ_Visualization
make
make run
```

Clean build artifacts:
```bash
make clean
```

> `freeglut.dll` is copied automatically by `make`. No manual copying needed.

---

## Controls

| Key | Action |
|-----|--------|
| `TAB` | Switch between Liver and Lungs view |
| `R` | Restart animation from Stage 1 |
| `ESC` | Quit |

---

## Algorithms Implemented

| Chapter | Algorithm | Where Used |
|---------|-----------|------------|
| Ch.2 | Bresenham Line | All UI borders — `utils.cpp` |
| Ch.2 | Scan-line Polygon Fill | Organ silhouettes — `liver.cpp`, `lungs.cpp` |
| Ch.2 | Mid-point Circle Fill | Damage spots, tar spots |
| Ch.3 | 2D Transforms (translate, rotate, scale) | Animations — `animation.cpp`, `lungs_animation.cpp` |
| Ch.4 | Quadratic Bezier | Pour stream, smoke plumes |
| Ch.4 | Cubic Bezier | Bronchial tree — `lungs.cpp` |
| Ch.4 | Catmull-Rom Spline | Lung lobe outline — `lungs.cpp` |
| Ch.4 | Polar Parametric Curve | Liver outline — `liver.cpp` |
| Ch.6 | Gouraud Shading | Dome highlights on organs |
| Ch.6 | Specular Reflection | Glass bowl, ember tip |
| Ch.7 | Key-frame Animation | 7-state machines with smoothStep easing |
| Ch.7 | Direct-motion Specification | Sinusoidal breathing on lungs |

---

## Damage Stages

| Stage | Time | Liver | Lungs |
|-------|------|-------|-------|
| 1 | 1 Month | Mild fat deposits | Mild irritation |
| 2 | 1 Year | Fatty liver (steatosis) | Chronic bronchitis |
| 3 | 5 Years | Alcoholic hepatitis | Early COPD |
| 4 | 10 Years | Fibrosis | Moderate COPD |
| 5 | 20 Years | Cirrhosis | Severe emphysema |
| 6 | 30 Years | End-stage / failure | Lung failure |

---

## References

1. Hearn D., Baker M.P. (1997). *Computer Graphics C Version* (2nd ed.), Prentice Hall.
2. Theoharis T. et al. (2008). *Graphics and Visualization: Principles & Algorithms*, CRC Press.
3. Foley J.D. (1995). *Computer Graphics: Principles and Practice in C*, Addison-Wesley.
4. ENCT 201 Course Syllabus — IOE, Tribhuvan University.