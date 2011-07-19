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
#include "input.h"

/*
struct machine_state {
    cpu_state cpu_st;
    ppu_state ppu_st;
    apu_state apu_st;
    byte memory[0x800];
    int scheduled_nmi, scheduled_irq;
    bool irq_waiting;
    sf::Image screen;
}
*/

class Machine {
    byte *mem;
    //window
    sf::RenderWindow wind;
    //input
    byte read_input_state;
    bool keys[8];
	InputProvider *inp;
    //debug
    string dump_regs();
public:
    Machine(Rom *rom);
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
	void set_input(InputProvider *inp);
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
