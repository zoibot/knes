CC = g++
CFLAGS = -Wall -O2
OBJECTS = main.o cpu.o ppu.o machine.o rom.o mapper.o apu.o instruction.o
LIBS = -lsfml-window -lsfml-system -lsfml-graphics -lsfml-audio

default: nes

nes : $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJECTS) -o nes

%.o : %.cpp *.h util.h
	$(CC) $(CFLAGS) -c $<

%.cpp : %.h util.h