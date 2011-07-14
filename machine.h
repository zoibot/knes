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

const sf::Keyboard::Key keymap[8] = { 
                  sf::Keyboard::Z, //a
                  sf::Keyboard::X, //b
                  sf::Keyboard::S, //Select
                  sf::Keyboard::Return, //Start
                  sf::Keyboard::Up,
                  sf::Keyboard::Down,
				  sf::Keyboard::Left,
                  sf::Keyboard::Right,
                };

class Machine {
    byte *mem;
    sf::RenderWindow wind;
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
    //APU
    APU *apu;
    //ppu
	void sync_ppu(int cycles);
    PPU *ppu;

	void reset();
    void run(int frames);
    void save();
	sf::Image screenshot();

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
