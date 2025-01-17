CC = clang
IFLAGS = -I./include
LFLAGS = `pkg-config sdl3 --cflags --libs` -lm
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 
SOURCES = ./*.c
TARGET = ./fluidsim

all: default

default: 
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $(SOURCES) -o $(TARGET)

andrun: default
	$(TARGET)

run: 
	$(TARGET)

debug:
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $(SOURCES) -g -o $(TARGET)

clean:
	rm -f $(TARGET)