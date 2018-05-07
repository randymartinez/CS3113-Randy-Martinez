SRC=main.cpp Matrix.cpp ShaderProgram.cpp FlareMap.cpp
LDFLAGS=-lstdc++ -lm -lSDL2 -lSDL2_mixer -lGLEW -lGL
CFLAGS=-Wall -I/usr/include/SDL2
default:
	g++ -std=c++11 $(CFLAGS) $(SRC) $(LDFLAGS) -DLINUX -o game
#./Matrix.h ./ShaderProgram.h ./Matrix.cpp ./ShaderProgram.cpp 
