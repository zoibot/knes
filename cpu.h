#ifndef CPU_H
#define CPU_H

#include "util.h"
#include "instruction.h"
#include "log.h"

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
    //machine that this CPU is a part of
    Machine *m;
    //registers
    int pc;
    byte a, s, p;
    byte x, y;
    //cycle timing information
    int cycle_count, prev_cycles;

    //instruction helper functions
    void branch(bool cond, Instruction &inst);
    void compare(byte a, byte b);

    //memory accessing functions
    byte get_mem(word addr);
    void set_mem(word addr, byte value);
    byte next_byte();
    word next_word();

    //stack helpers
    void push2(word val);
    word pop2();
    void push(byte val);
    byte pop();

    //flag helpers
    void set_nz(byte val);
    void set_flag(byte flag, bool val);

    void log(string message, LogLevel level = lINFO);
public:
    CPU(Machine *m);
    //accessors
    bool get_flag(byte flag);
    int get_cycle_count();
    void add_cycles(int c);
    word get_pc();
    void set_pc(word addr);
    
    //interrupts
    void reset();
    void irq();
    void nmi();

    //running
    Instruction next_instruction();
    int execute_inst(Instruction inst);

    //debugging
    string dump_regs();
};

#endif //CPU_H
