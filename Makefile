CC = g++
CFLAGS = -g -O1 -std=c++11 -lsfml-graphics -lsfml-window -lsfml-system
CPP_FILES = *.cpp

all: $(CPP_FILES)
	$(CC) $(CFLAGS) $(CPP_FILES) -o simulator
