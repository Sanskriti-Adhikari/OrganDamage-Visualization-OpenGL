# ─── LiverVisualizer Makefile (Windows / MinGW64 / MSYS2) ───────────────────
# Builds a 2-D liver visualizer using OpenGL / FreeGLUT
#
# Requirements:
#   - MinGW64 or MSYS2
#   - FreeGLUT: pacman -S mingw-w64-x86_64-freeglut
#
# Usage:
#   make        -> build the executable
#   make run    -> run the program
#   make clean  -> remove object files & executable
# ────────────────────────────────────────────────────────────────────────────

# Compiler & flags
CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall
LDFLAGS  = -lfreeglut -lopengl32 -lglu32

# Source files
SRCS     = liver_main.cpp liver.cpp animation.cpp ui.cpp \
           side_bar.cpp bottom_bar.cpp utils.cpp

# Object files (one .o per .cpp)
OBJS     = $(SRCS:.cpp=.o)

# Target executable
TARGET   = LiverVisualizer.exe

# ─── Default target ──────────────────────────────────────────────────────────
all: $(TARGET)
	@echo "Build complete: $(TARGET) is ready"

# ─── Link all object files to create executable ──────────────────────────────
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# ─── Compile each .cpp to .o ─────────────────────────────────────────────────
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ─── Run program ─────────────────────────────────────────────────────────────
run: $(TARGET)
	./$(TARGET)

# ─── Clean build artifacts ──────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Clean complete: objects and executable removed"

# ─── Phony targets ───────────────────────────────────────────────────────────
.PHONY: all run clean