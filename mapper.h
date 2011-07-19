#ifndef MAPPER_H
#define MAPPER_H

#include "util.h"

struct Rom;
class Machine;

struct mapper_state {
    byte prg_banks[128];
    byte chr_banks[128];
    byte registers[128];
};

//abstract Mapper class
class Mapper {
public:
    Rom *rom;
    virtual void prg_write(word addr, byte val) = 0;
    virtual void load() = 0;
    virtual void update(Machine *m) = 0;
    //TODO virtual mapper_state save_state() = 0;
    virtual string name() = 0;
    Mapper(Rom *rom);
};

class NROM : public Mapper {
public:
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    NROM(Rom *rom);
};

class UNROM : public Mapper {
public:
    int bank;
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    UNROM(Rom *rom);
};

class CNROM : public Mapper {
public:
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    CNROM(Rom *rom);
};

class MMC1 : public Mapper {
    byte loadr;
    byte control;
    byte shift;
    byte prg_bank;
    byte prg_bank_offset;
    byte prg_ram_bank;
    void update_prg_bank();
public:
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    MMC1(Rom *rom);
};

class MMC3 : public Mapper {
    byte bank_select;
    int current_chr_banks[6];
    int current_prg_banks[3];
    byte last_irq_counter;
    byte bank_config;
    byte irq_latch;
    bool irq_enabled;
    bool irq_waiting;
    byte irq_counter;
    bool a12high;
    void clock_counter();
    void update_prg_banks();
    void update_chr_banks();
public:
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    MMC3(Rom *rom);
};

class AXROM : public Mapper {
public:
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    AXROM(Rom *rom);
};

class Camerica : public Mapper {
public:
    int bank;
    void prg_write(word addr, byte val);
    void load();
    void update(Machine *m);
    string name();
    Camerica(Rom *rom);
};

#endif //MAPPER _H
