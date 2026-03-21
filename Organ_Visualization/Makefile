CXX = "D:/Mingw/mingw64/bin/g++.exe"
INC = -I"D:/Mingw/mingw64/include" -I"D:/Mingw/freeglut-mingw-master/include"
LIB = -L"D:/Mingw/freeglut-mingw-master/lib/x64" -lfreeglut -lopengl32 -lglu32 -lmingw32
FLG = -std=c++11 -O2 -Wno-unused-function -Wno-unused-result
OUT = OrganDamageVisualizer.exe
SRC = main.cpp utils.cpp ui.cpp side_bar.cpp bottom_bar.cpp liver.cpp animation.cpp lungs.cpp lungs_animation.cpp

all:
	cp "D:/Mingw/freeglut-mingw-master/bin/x64/freeglut.dll" . 2>/dev/null || true
	$(CXX) $(FLG) $(INC) $(SRC) -o $(OUT) $(LIB)

run: all
	./$(OUT)

clean:
	rm -f $(OUT) freeglut.dll

.PHONY: all run clean