#ifndef CPU_H
#define CPU_H

#include "util.h"
#include "instruction.h"

//flags
static const byte N = 1 << 7;
static const byte V = 1 << 6;
static const byte B = 1 << 4;
static const byte D = 1 << 3;
static const byte I = 1 << 2;
static const byte Z = 1 << 1;
static const byte C = 1 << 0;

class CPU {
private:
	Machine *m;
    void set_nz(byte val);

	void branch(bool cond, Instruction &inst);
	void compare(byte a, byte b);

	byte get_mem(word addr);
	void set_mem(word addr, byte value);

    void push2(word val);
    word pop2();
    void push(byte val);
    byte pop();
public:
	CPU(Machine *m);
	int pc;
	void set_flag(byte flag, bool val);
    bool get_flag(byte flag);
    byte a, s, p;
	byte x, y;
	int cycle_count, prev_cycles;
	
	void irq();
	void nmi();

	Instruction next_instruction();
	int execute_inst(Instruction inst);
	void reset();

	byte next_byte();
    word next_word();

};

#endif //MACHINE_H