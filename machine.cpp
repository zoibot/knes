#include <cstring>
#include <fstream>

#include "machine.h"

void Machine::reset() {
	memset(mem, 0xff, 0x800);
    mem[0x0008] = 0xf7;
    mem[0x0009] = 0xef;
    mem[0x000a] = 0xdf;
    mem[0x000f] = 0xbf;
}

void Machine::save() {
	if(!(rom->flags6 & 2)) return;
	cout << " saving to " << (rom->fname + ".sav") << endl;
	ofstream save((rom->fname + ".sav").c_str());
    save.write((char*)rom->prg_ram, 0x2000);
	save.close();
}

byte Machine::get_mem(word addr) {
    if(addr < 0x2000) {
        return mem[addr & 0x7ff];
    } else if(addr < 0x4000) {
		ppu->run();
        return ppu->read_register((addr - 0x2000)&7);
    } else if(addr < 0x4018) {
        switch(addr) {
        case 0x4016:
            if(read_input_state < 8) {
                return keys[read_input_state++];
            } else {
                return 1;
            }
        break;
        default:
            return apu->read_register(addr - 0x4000);
            break;
        }
    } else if(addr < 0x8000) {
        return rom->prg_ram[addr-0x6000];
    } else {
        return rom->prg_rom[(addr & 0x4000)>>14][addr&0x3fff];
    }
}

void Machine::set_mem(word addr, byte val) {
    if(addr < 0x2000) {
        mem[addr & 0x7ff] = val;
    } else if(addr < 0x4000) {
		ppu->run();
        ppu->write_register((addr - 0x2000)&7, val);
    } else if(addr < 0x4018) {
        switch(addr) {
        case 0x4016:
            if(val & 1) {
                for(int i = 0; i < 8; i++) {
                    keys[i] = wind.GetInput().IsKeyDown(keymap[i]);
                }
            }
            read_input_state = 0;
            break;
        case 0x4014:
            for(word v = 0; v < 0x100; v++) {
                byte addr = v + ppu->obj_addr;
                ppu->obj_mem[addr] = mem[(val << 8)+v];
            }
            //TODO cycles???
            break;
        default:
            apu->write_register(addr - 0x4000, val);
            break;
        }
    } else if(addr < 0x8000) {
        rom->prg_ram[addr-0x6000] = val;
    } else {
		rom->mapper->prg_write(addr, val);
		ppu->set_mirroring(rom->mirror);
	}
}

Machine::Machine(Rom *rom) {
    this->rom = rom;
	debug = false;
    //Display
    wind.Create(sf::VideoMode(256, 240), "asdfNES", sf::Style::Close);
    //wind.SetFramerateLimit(60);// what is the right limit??
    wind.EnableVerticalSync(false);
    //print surface bits
    cout << "Depth Bits: " << wind.GetSettings().DepthBits << endl;
	cpu = new CPU(this);
    ppu = new PPU(this, &wind);
	apu = new APU(this);
	scheduled_nmi = scheduled_irq = 0;
	irq_waiting = false;
    //clock???
    mem = new byte[0x800];
    memset(mem, 0xff, 0x800);
}

void Machine::request_nmi() {
	scheduled_nmi = 2;
}

void Machine::suppress_nmi() {
	scheduled_nmi = -1;
}

void Machine::request_irq() {
	if(scheduled_irq < 0 && !cpu->get_flag(I)) {
		scheduled_irq = 2;
	}
	irq_waiting = true;
}

void Machine::run_interrupts() {
	if(scheduled_irq >= 0) {
		scheduled_irq--;
	}
	if(scheduled_nmi >= 0) {
		scheduled_nmi--;
	}
	if(scheduled_nmi == 0) {
		cpu->nmi();
		irq_waiting = false;
		scheduled_irq = -1;
	} else if(scheduled_irq == 0) {
		cpu->irq();
		irq_waiting = false;
	}
}

void Machine::sync_ppu(int cycles) {
	cpu->cycle_count += cycles;
	ppu->run();
}

void Machine::run() {
    cpu->reset();
    ofstream cout("LOG.TXT");
	cout << uppercase << setfill('0');
	Instruction inst;
    //debug = true;
    while(1) {
        try {
            if(debug)
                cout << HEX4(cpu->pc) << "  ";
            inst = cpu->next_instruction();
            if(debug)
                cout << inst << dump_regs() << endl; 
            int cycles = cpu->execute_inst(inst);
			ppu->run();
			apu->update(cycles);
			//rom->mapper->update();
			run_interrupts();
			//special handling for blargg tests
			if(rom->prg_ram[1] == 0xde && rom->prg_ram[2] == 0xb0) {//&& mem[0x6003] == 0x61) {
				switch(rom->prg_ram[0]) {
				case 0x80:
					//running
					break;
				case 0x81:
					//need reset
					break;
				default:
					cout << "test done: " << endl;
					cout << (char*)(rom->prg_ram + 4) << endl;
                    exit(0);
					break;
				}
			}
        } catch(...) {
            break;
        }
    }
}

string Machine::dump_regs() {
    stringstream out;
    out << hex << uppercase;
    out << "A:" << HEX2(cpu->a);
    out << " X:" << HEX2(cpu->x);
    out << " Y:" << HEX2(cpu->y);
    out << " P:" << HEX2(cpu->p);
    out << " SP:" << HEX2(cpu->s);
	out << dec;
    out << " CC:" << cpu->cycle_count;
    out << " CYC: " << ppu->cyc;
    out << " SL:" << ppu->sl;
	out << " VADDR: " << HEX4(ppu->vaddr);
	int end;
	if(ppu->last_vblank_end > ppu->last_vblank_start) {
		end = ppu->last_vblank_end;
	} else {
		end = cpu->cycle_count;
	}
	out << " FC: " << dec << (end - ppu->last_vblank_start);
    return out.str();
}
