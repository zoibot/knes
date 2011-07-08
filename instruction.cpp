#include "instruction.h"
#include "machine.h"

Opcode::Opcode() {
	this->invalid = true;
}

Opcode::Opcode(Op op, AddrMode amode, int cycles) {
	this->op = op;
	this->addr_mode = amode;
	this->cycles = cycles;
    this->extra_page_cross = 1;
	if(op == STA || op == STX || op == STY)
		store = true;
}

Opcode::Opcode(Op op, AddrMode amode, int cycles, int extra_page_cross) { 
	this->op = op;
	this->addr_mode = amode;
	this->cycles = cycles;
	if(op == STA || op == STX || op == STY)
		store = true;
    this->extra_page_cross = extra_page_cross;
}

Instruction::Instruction() {
	arglen = 0;
}

ostream& operator <<(ostream &out, Instruction &inst) {
	out << HEX2(inst.opcode);
	out << " ";
	if(inst.arglen > 0) {
		out << HEX2(inst.args[0]);
	} else {
		out << "  ";
	}
	out << " ";
	if(inst.arglen > 1) {
		out << HEX2(inst.args[1]);
	} else {
		out << "  ";
	}
	out << " ";
	if(inst.op.illegal) {
		out << "*";
	} else {
		out << " ";
	}
	out << opnames[inst.op.op];
	out << " ";
	switch (inst.op.addr_mode) {
	case IMM:
		out << "#$" << HEX2(inst.operand) << "                        ";
		break;
	case REL:
		out << "$" << HEX4(inst.addr) << "                       ";
		break;
	case ZP:
	case ZP_ST:
		out << "$" << HEX2(inst.addr) << "                         ";
		break;
	case ABS:
	case ABS_ST:
		out << "$" << HEX4(inst.addr) << "                       ";
		break;
    case A:
        out << "A" << "                           ";
        break;
    case ZPX:
        out << "($" << HEX2(inst.args[0]) << ",X) @ " << HEX2(inst.addr) << "                ";
        break;
	case ZPY:
        out << "($" << HEX2(inst.args[0]) << ",Y) @ " << HEX2(inst.addr) << "                ";
        break;
    case IDIX:
        out << "($" << HEX2(inst.args[0]) << "),Y     " << HEX2(inst.operand) << " @ " << HEX2(inst.i_addr) << "         ";
        break;
    case IXID:
        out << "($" << HEX2(inst.args[0]) << ",X) @ " << HEX2(inst.i_addr) << "                ";
        break;
	default:
		out << "                            ";
	}
	return out;
}

Instruction CPU::next_instruction() {
	char off;
	Instruction next;
	next.opcode = next_byte();
	int extra_cycles = 0;
	next.op = ops[next.opcode];
	switch (next.op.addr_mode) {
	case IMM:
		next.operand = next_byte();
		next.args[0] = next.operand;
		next.arglen = 1;
		break;
	case ZP:
		next.addr = next_byte();
		next.operand = get_mem(next.addr);
		next.args[0] = next.addr;
		next.arglen = 1;
		break;
	case ZP_ST:
		next.addr = next_byte();
		next.args[0] = next.addr;
		next.arglen = 1;
		break;
	case ABS:
		next.addr = next_word();
		next.operand = get_mem(next.addr);
		next.args[0] = next.addr & 0xff;
		next.args[1] = (next.addr & 0xff00) >> 8;
		next.arglen = 2;
		break;
	case ABS_ST:
		next.addr = next_word();
		next.args[0] = next.addr & 0xff;
		next.args[1] = (next.addr & 0xff00) >> 8;
		next.arglen = 2;
		break;
	case ABSI:
		next.i_addr = next_word();
    	next.addr = get_mem(next.i_addr) + (get_mem(((next.i_addr+1) & 0xff) | (next.i_addr & 0xff00)) << 8);
		next.args[0] = next.i_addr & 0xff;
		next.args[1] = (next.i_addr & 0xff00) >> 8;
		next.arglen = 2;
		break;
	case ABSY:
		next.i_addr = next_word();
		next.addr = (next.i_addr + y) & 0xffff;
		if(!next.op.store) {
			next.operand = get_mem(next.addr);
		}
		if((next.i_addr & 0xff00) != (next.addr & 0xff00)) {
			extra_cycles += next.op.extra_page_cross;
		}
		next.args[0] = next.i_addr & 0xff;
		next.args[1] = (next.i_addr & 0xff00) >> 8;
		next.arglen = 2;
		break;
	case ABSX:
		next.i_addr = next_word();
		next.addr = (next.i_addr + x) & 0xffff;
		if(!next.op.store) {
			next.operand = get_mem(next.addr);
		}
		if((next.i_addr & 0xff00) != (next.addr & 0xff00)) {
			extra_cycles += next.op.extra_page_cross;
		}
		next.args[0] = next.i_addr & 0xff;
		next.args[1] = (next.i_addr & 0xff00) >> 8;
		next.arglen = 2;
		break;
	case REL:
		off = next_byte();
		next.addr = off + pc;
		next.args[0] = off;
		next.arglen = 1;
		break;
	case IXID:
		next.args[0] = next_byte();
		next.i_addr = (next.args[0] + x) & 0xff;
		next.addr = (get_mem(next.i_addr) + (get_mem((next.i_addr+1) & 0xff) << 8));
		if(!next.op.store) {
			next.operand = get_mem(next.addr);
		}
		next.arglen = 1;
		break;
	case IDIX:
		next.i_addr = next_byte();
		next.addr = (get_mem(next.i_addr) + (get_mem((next.i_addr+1)&0xff)<<8)) + y;
		next.addr &= 0xffff;
		if(!next.op.store) {
			next.operand = get_mem(next.addr);
		}
		if((next.addr & 0xff00) != ((next.addr - y) & 0xff00)) {
			extra_cycles += next.op.extra_page_cross;
		}
		next.args[0] = next.i_addr;
		next.arglen = 1;
		break;
	case ZPX:
		next.i_addr = next_byte();
		next.addr = (next.i_addr + x) & 0xff;
		if(!next.op.store) {
			next.operand = get_mem(next.addr);
		}
		next.args[0] = next.i_addr;
		next.arglen = 1;
		break;
	case ZPY:
		next.i_addr = next_byte();
		next.addr = (next.i_addr + y) & 0xff;
		if(!next.op.store) {
			next.operand = get_mem(next.addr);
		}
		next.args[0] = next.i_addr;
		next.arglen = 1;
		break;
	default:
		next.arglen = 0;
		break;
	}
	return next;
}
