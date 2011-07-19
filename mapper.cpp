#include "mapper.h"
#include "rom.h"
#include "machine.h"

Mapper::Mapper(Rom *rom) {
	this->rom = rom;
}

NROM::NROM(Rom *rom) : Mapper(rom) {
}

void NROM::prg_write(word addr, byte val) {};
void NROM::load() {
	rom->prg_rom[0] = rom->prg_banks;
	rom->prg_rom[1] = rom->prg_banks + 0x4000 * (rom->prg_size - 1);
	rom->chr_rom[0] = rom->chr_banks;
	rom->chr_rom[1] = rom->chr_banks + 0x1000;
    rom->prg_bank_mask = 0x4000;
    rom->prg_bank_shift = 14;
    rom->chr_bank_mask = 0x1000;
    rom->chr_bank_shift = 12;
}
void NROM::update(Machine *m) {};
string NROM::name() {
	return "NROM";
};

UNROM::UNROM(Rom *rom) : Mapper(rom) {
	this->rom = rom;
}
void UNROM::prg_write(word addr, byte val) {
	bank = val & 7;
	rom->prg_rom[0] = rom->prg_banks + (0x4000 * bank);
};
void UNROM::load() {
	rom->prg_rom[0] = rom->prg_banks;
	rom->prg_rom[1] = rom->prg_banks + (rom->prg_size - 1) * 0x4000;
	rom->chr_rom[0] = rom->chr_banks;
	rom->chr_rom[1] = rom->chr_banks + 0x1000;
    rom->prg_bank_mask = 0x4000;
    rom->prg_bank_shift = 14;
    rom->chr_bank_mask = 0x1000;
    rom->chr_bank_shift = 12;
}
void UNROM::update(Machine *m) {};
string UNROM::name() {
	return "UNROM";
};

CNROM::CNROM(Rom *rom) : Mapper(rom) {
}
void CNROM::prg_write(word addr, byte val) {
	rom->chr_rom[0] = rom->chr_banks + (0x2000 * (val & 3));
	rom->chr_rom[1] = rom->chr_banks + (0x2000 * (val & 3)) + 0x1000;
};
void CNROM::load() {
	rom->prg_rom[0] = rom->prg_banks;
	rom->prg_rom[1] = rom->prg_banks;
	if(rom->prg_size > 1) {
		rom->prg_rom[1] += 0x4000;
	}
	rom->chr_rom[0] = rom->chr_banks;
	rom->chr_rom[1] = rom->chr_banks + 0x1000;
    rom->prg_bank_mask = 0x4000;
    rom->prg_bank_shift = 14;
    rom->chr_bank_mask = 0x1000;
    rom->chr_bank_shift = 12;
}
void CNROM::update(Machine *m) {};
string CNROM::name() {
	return "CNROM";
};

MMC1::MMC1(Rom *rom) : Mapper(rom) {
	control = 0xc;
	loadr = 0;
	shift = 0;
	prg_bank = 0;
    prg_bank_offset = 0;
}
void MMC1::prg_write(word addr, byte val) {
	loadr |= (val & 1) << shift;
	shift += 1;
	if(val & 0x80) {
		loadr = 0;
		shift = 0;
		control |= 0xc;
		return;
	}
	if(shift == 5) {
		if(addr < 0xa000) {
			switch(loadr & 3) {
			case 0:
				rom->mirror = SINGLE_LOWER;
				break;
			case 1:
				rom->mirror = SINGLE_UPPER;
				break;
			case 2:
				rom->mirror = VERTICAL;
				break;
			case 3:
				rom->mirror = HORIZONTAL;
				break;
			}
			if((control & 0xc) != (loadr & 0xc)) {
				control = loadr;
				update_prg_bank();
			}
			control = loadr;
		} else if(addr < 0xc000) {
			//chr bank 0
			if(control & (1<<4)) {
				//4kb mode
                loadr = loadr & (rom->chr_size - 1);
				rom->chr_rom[0] = rom->chr_banks + 0x1000 * loadr;
			} else {
                if(rom->chr_size == 0 && rom->prg_size == 32) {
                    //SUROM
                    prg_bank_offset = loadr & 0x10;
                    prg_ram_bank = ((loadr & 0xc) >> 2);
                    update_prg_bank();
                }
                if(rom->chr_size != 0) {
				    rom->chr_rom[0] = rom->chr_banks + 0x1000 * (loadr & 0x1e);
				    rom->chr_rom[1] = rom->chr_banks + 0x1000 * (loadr | 1);
                }
			}
		} else if(addr < 0xe000) {
			//chr bank 1
			if(control & (1<<4)) {
				//4kb mode  
                loadr = loadr & (rom->chr_size - 1);
				rom->chr_rom[1] = rom->chr_banks + 0x1000 * loadr;
			} else {
				//8kb ignore
			}
		} else {
			//prg bank
			prg_bank = loadr & min((rom->prg_size - 1), 0xf);
			update_prg_bank();
		}
		shift = 0;
		loadr = 0;
	}
};
void MMC1::update_prg_bank() {
    byte pb = prg_bank + prg_bank_offset;
	switch(control & 0xc) {
		case 0x0:
		case 0x4:
			rom->prg_rom[0] = rom->prg_banks + 0x4000 * (pb & 0x1e);
			rom->prg_rom[1] = rom->prg_banks + 0x4000 * (pb | 1);
			break;
		case 0x8:
			rom->prg_rom[0] = rom->prg_banks + 0x4000 * prg_bank_offset;
			rom->prg_rom[1] = rom->prg_banks + 0x4000 * pb;
			break;
		case 0xc:
			rom->prg_rom[0] = rom->prg_banks + 0x4000 * pb;
			rom->prg_rom[1] = rom->prg_banks + 0x4000 * (prg_bank_offset + min((rom->prg_size - 1), 0xf));
			break;
	}
}
void MMC1::update(Machine *m) {};

void MMC1::load() {
	rom->prg_rom[0] = rom->prg_banks;
	rom->prg_rom[1] = rom->prg_banks + 0x4000 * (rom->prg_size - 1);
	rom->chr_rom[0] = rom->chr_banks;
	rom->chr_rom[1] = rom->chr_banks + 0x1000;
    rom->chr_bank_mask = 0x1000;
    rom->chr_bank_shift = 12;
    rom->prg_bank_mask = 0x4000;
    rom->prg_bank_shift = 14;
}
string MMC1::name() {
	return "MMC1";
};

MMC3::MMC3(Rom *rom) : Mapper(rom) {
    last_irq_counter = 0;
}
void MMC3::prg_write(word addr, byte val) {
    switch(addr&1) {
    case 0:
        if(addr < 0xa000) {
            //bank select
            bank_select = val & 7;
            bank_config = (val & 0xc0) >> 6;
        } else if(addr < 0xc000) {
            //mirroring
            if(!(val & 1)) {
                rom->mirror = VERTICAL;
            } else {
                rom->mirror = HORIZONTAL;
            }
        } else if(addr < 0xe000) {
            //irq latch
            irq_latch = val;
        } else {
            //irq disable
            irq_enabled = false;
            //acknowledge waiting
            irq_waiting = false;
        }
        break;
    case 1:
        if(addr < 0xa000) {
            int v = val;
            //set bank
            switch(bank_select) {
            case 0:
                current_chr_banks[0] = v & 0xfe;
                break;
            case 1:
                current_chr_banks[1] = v & 0xfe;
                break;
            case 2:
                current_chr_banks[2] = v;
                break;
            case 3:
                current_chr_banks[3] = v;
                break;
            case 4:
                current_chr_banks[4] = v;
                break;
            case 5:
                current_chr_banks[5] = v;
                break;
            case 6:
                current_prg_banks[0] = v & (rom->prg_size*2-1);
                break;
            case 7:
                current_prg_banks[1] = v & (rom->prg_size*2-1);
                break;
            }
            update_prg_banks();
            update_chr_banks();
        } else if(addr < 0xc000) {
            //prg ram
        } else if(addr < 0xe000) {
            //irq reload
            irq_counter = 0;
        } else {
            //irq enable
            irq_enabled = true;
        }
    }
};
void MMC3::update_prg_banks() {
    if(!(bank_config & 1)) {
        rom->prg_rom[0] = rom->prg_banks + 0x2000*current_prg_banks[0];
        rom->prg_rom[1] = rom->prg_banks + 0x2000*current_prg_banks[1];
        rom->prg_rom[2] = rom->prg_banks + 0x2000*(rom->prg_size*2-2);
    } else {
        rom->prg_rom[0] = rom->prg_banks + 0x2000*(rom->prg_size*2-2);
        rom->prg_rom[1] = rom->prg_banks + 0x2000*current_prg_banks[1];
        rom->prg_rom[2] = rom->prg_banks + 0x2000*current_prg_banks[0];
    }
}
void MMC3::update_chr_banks() {
    if(!(bank_config & 2)) {
        //two four
        rom->chr_rom[0] = rom->chr_banks + 0x400*current_chr_banks[0];
        rom->chr_rom[1] = rom->chr_banks + 0x400*current_chr_banks[0]+0x400;
        rom->chr_rom[2] = rom->chr_banks + 0x400*current_chr_banks[1];
        rom->chr_rom[3] = rom->chr_banks + 0x400*current_chr_banks[1]+0x400;
        rom->chr_rom[4] = rom->chr_banks + 0x400*current_chr_banks[2];
        rom->chr_rom[5] = rom->chr_banks + 0x400*current_chr_banks[3];
        rom->chr_rom[6] = rom->chr_banks + 0x400*current_chr_banks[4];
        rom->chr_rom[7] = rom->chr_banks + 0x400*current_chr_banks[5];
    } else {
        //four two
        rom->chr_rom[0] = rom->chr_banks + 0x400*current_chr_banks[2];
        rom->chr_rom[1] = rom->chr_banks + 0x400*current_chr_banks[3];
        rom->chr_rom[2] = rom->chr_banks + 0x400*current_chr_banks[4];
        rom->chr_rom[3] = rom->chr_banks + 0x400*current_chr_banks[5];
        rom->chr_rom[4] = rom->chr_banks + 0x400*current_chr_banks[0];
        rom->chr_rom[5] = rom->chr_banks + 0x400*current_chr_banks[0]+0x400;
        rom->chr_rom[6] = rom->chr_banks + 0x400*current_chr_banks[1];
        rom->chr_rom[7] = rom->chr_banks + 0x400*current_chr_banks[1]+0x400;
    }
}
void MMC3::update(Machine *m) {
    if(!a12high && m->ppu->a12high && !last_irq_counter) {
        clock_counter();
        last_irq_counter = 12;
    }
    a12high = m->ppu->a12high;
    if(irq_waiting && irq_enabled)
        m->request_irq();
    if(last_irq_counter)
        last_irq_counter--;
};

void MMC3::clock_counter() {
    if(irq_counter > 0) {
        irq_counter--;
    } else {
        irq_counter = irq_latch;
    }
    if(irq_counter == 0 && irq_enabled) {
        irq_waiting = true;
    }
}

void MMC3::load() {
    rom->prg_rom[3] = rom->prg_banks + 0x2000 * (rom->prg_size*2-1);
    for(int i = 0; i < 6; i++) {
        current_chr_banks[i] = i;
    }
    current_prg_banks[0] = 0;
    current_prg_banks[1] = 1;
    update_chr_banks();
    update_prg_banks();
    rom->chr_bank_mask = 0xfc00;
    rom->chr_bank_shift = 10;
    rom->prg_bank_mask = 0x6000;
    rom->prg_bank_shift = 13;
}
string MMC3::name() {
	return "MMC3";
};


AXROM::AXROM(Rom *rom) : Mapper(rom) {
}
void AXROM::prg_write(word addr, byte val) {
	rom->prg_rom[0] = rom->prg_banks + 0x8000 * (val & 7);
	rom->prg_rom[1] = rom->prg_banks + 0x8000 * (val & 7) + 0x4000;
	if(val & 0x10) {
		rom->mirror = SINGLE_LOWER;
	} else {
		rom->mirror = SINGLE_UPPER;
	}
};
void AXROM::load() {
	rom->prg_rom[0] = rom->prg_banks;
	rom->prg_rom[1] = rom->prg_banks + 0x4000;
	rom->chr_rom[0] = rom->chr_banks;
	rom->chr_rom[1] = rom->chr_banks + 0x1000;
    rom->prg_bank_mask = 0x4000;
    rom->prg_bank_shift = 14;
    rom->chr_bank_mask = 0x1000;
    rom->chr_bank_shift = 12;
}
void AXROM::update(Machine *m) {};
string AXROM::name() {
	return "AxROM";
};

Camerica::Camerica(Rom *rom) : Mapper(rom) {
	this->rom = rom;
}
void Camerica::prg_write(word addr, byte val) {
	if(addr < 0xc000) {
	}
	if(addr < 0xa000) {
		/*if(val & 0x10) {
			rom->mirror = SINGLE_LOWER;
		} else {
			rom->mirror = SINGLE_UPPER;
		}*/
	}
	if(addr >= 0xc000) {
		bank = val & (rom->prg_size-1);
		rom->prg_rom[0] = rom->prg_banks + (0x4000 * bank);
	}
};
void Camerica::load() {
	rom->prg_rom[0] = rom->prg_banks;
	rom->prg_rom[1] = rom->prg_banks + (rom->prg_size - 1) * 0x4000;
	rom->chr_rom[0] = rom->chr_banks;
	rom->chr_rom[1] = rom->chr_banks + 0x1000;
    rom->prg_bank_mask = 0x4000;
    rom->prg_bank_shift = 14;
    rom->chr_bank_mask = 0x1000;
    rom->chr_bank_shift = 12;
	rom->mirror = FOUR_SCREEN;
}
void Camerica::update(Machine *m) {};
string Camerica::name() {
	return "Camerica";
};
