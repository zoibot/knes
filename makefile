CC = g++
CFLAGS = `wx-config --version=2.9 --cppflags` -Wall -O2
OBJECTS = gui.o cpu.o ppu.o machine.o rom.o mapper.o apu.o instruction.o input.o test.o log.o
LIBS = `wx-config --version=2.9 --libs` 

default: nes

clean:
	rm $(OBJECTS) nes

nes : $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJECTS) -o nes

%.o : %.cpp *.h util.h
	$(CC) $(CFLAGS) -c $<

%.cpp : %.h util.h
