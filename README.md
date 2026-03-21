# OpenGL Setup in VS Code using C++ (MinGW) — GLFW + GLAD

This project documents a working setup for running **OpenGL programs in VS Code using C++ (MinGW g++) on Windows**.

The setup was done by following a YouTube tutorial and then modifying the configuration so it works correctly with multiple `.cpp` files without build errors.

---

## Credit

**Video followed:**  
*C++ OpenGL setup for VSCode in 2min*  

**Channel:** Codeus  
**Link:** https://youtu.be/Y4F0tI7WlDs  

The original configuration from the video was modified to:

- Build only the **currently active file**
- Avoid **multiple definition of `main()`** errors
- Automatically create an executable with the same name as the source file

---

## Requirements

- VS Code
- MinGW-w64 (g++ compiler)
- OpenGL
- GLFW
- GLAD

---

## Step 1 — Install MinGW

Install MinGW-w64 and add this to system PATH: C:\mingw64-14.2.0\bin

Verify installation: g++ --version


---

## Step 2 — Download Libraries

### GLAD

Download from:

https://glad.dav1d.de/

Use these settings:

- Language: C/C++
- Specification: OpenGL
- Profile: Core
- Version: 3.3

After generating, extract and copy:

- `include` folder
- `src` folder

into your project folder.

---

### GLFW

Download **64-bit Windows binaries** from:

https://www.glfw.org/download.html

From: glfw-3.4.bin.WIN64/lib-mingw-w64

Copy: libglfw3dll.a

into: open gl/lib  [open gl = fresh folder containing your cpp files]

copy: glfw3.dll

into: open gl/src

---

## Project Folder Structure

open gl
│
├── include
├── lib
│ └── libglfw3dll.a
├── src
│ ├── glad.c
│ ├── main.cpp
│ └── glfw3.dll
└── .vscode
└── tasks.json

## Sample Program

- Basic OpenGL window initialization using GLFW and GLAD  
- Source code:  
  https://github.com/tejaswi-acharya/OpenGL-for-VS-Code/blob/main/src/main.cpp

---

## VS Code Build Configuration (tasks.json)

To compile the OpenGL programs using MinGW inside VS Code, a custom **tasks.json** file is used.

This configuration was adapted from the original tutorial and modified so that:

- Only the **currently active C++ file** is compiled
- Multiple `.cpp` files with different `main()` functions do not cause build errors
- The executable is automatically created with the **same name as the source file**

### tasks.json Source

You can view the complete configuration here:

https://github.com/tejaswi-acharya/OpenGL-for-VS-Code/blob/main/.vscode/tasks.json

---

### Important Change from Original Setup

The original configuration used: "${workspaceFolder}/src/*.cpp"

This caused the error:

when more than one `.cpp` file existed.

It was replaced with: "${file}"

This ensures that VS Code builds only the file currently open in the editor.

---

## How to Build the Program

1. Open the desired `.cpp` file inside the `src` folder  
   (for example `main.cpp` or `mid_circle.cpp`)

2. Press: Ctrl + Shift + B or Run C/C++ in the VS Code window itself

3. VS Code will compile the program using MinGW and create: filename.exe

inside the same folder as the source file.

Example:
main.cpp → main.exe
mid_circle.cpp → mid_circle.exe


---

## How to Run the Executable

Open the terminal and navigate to the `src` folder: cd src

Then run: .\main.exe or .\mid_circle.exe


This should open the OpenGL window.

---

## Important Runtime Requirement

The file: glfw3.dll

must be present in the same folder as the generated `.exe` file.

Without this DLL, the program may close immediately or fail to start.

---

## Current Implementations

- OpenGL window initialization using GLFW and GLAD  
- Basic rendering loop  
- Midpoint Circle Algorithm (OpenGL visualization)

Source files:

- Main demo program:  
  https://github.com/tejaswi-acharya/OpenGL-for-VS-Code/blob/main/src/main.cpp

- Midpoint Circle implementation:  
  https://github.com/tejaswi-acharya/OpenGL-for-VS-Code/blob/main/src/mid_circle.cpp

---

## Purpose of This Repository

This repository is created for:

- Understanding OpenGL setup in VS Code
- Learning basic Computer Graphics algorithms
- Visualizing pixel-based rendering techniques
- Academic lab work (DDA, Bresenham Line, Midpoint Circle)

Future additions will include:

- DDA Line Algorithm
- Bresenham Line Algorithm
- Further visualization projects using OpenGL
































