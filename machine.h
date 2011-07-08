#ifndef MACHINE_H
#define MACHINE_H

#include <sstream>
#include <stdexcept>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "ppu.h"
#include "apu.h"
#include "cpu.h"
#include "instruction.h"
#include "rom.h"
#include "util.h"

const sf::Key::Code keymap[8] = { 
                  sf::Key::Z, //a
                  sf::Key::X, //b
                  sf::Key::S, //Select
                  sf::Key::Return, //Start
                  sf::Key::Up,
                  sf::Key::Down,
				  sf::Key::Left,
                  sf::Key::Right,
                };

class Machine {
    byte *mem;
    //ppu
	void sync_ppu(int cycles);
    PPU *ppu;
    sf::RenderWindow wind;
    //APU
    APU *apu;
    //sound??
    //input
    byte read_input_state;
    bool keys[8];


    string dump_regs();

public:
    Machine(Rom *rom);
	bool debug;
	int testeroo;

	Rom *rom;
	//CPU
	CPU *cpu;

	void reset();
    void run();
    void save();

	byte get_mem(word addr);
    void set_mem(word addr, byte val);

	//interrupts
	void request_irq();
	void request_nmi();
	void suppress_nmi();
	void run_interrupts();
	int scheduled_nmi;
	int scheduled_irq;
	bool irq_waiting;
};

#endif //MACHINE_H
