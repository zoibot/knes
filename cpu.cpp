#include "cpu.h"
#include "machine.h"

CPU::CPU(Machine *m) {
	this->m = m;
	a = x = y = s = 0;
	p = 0x24;
	cycle_count = 0;
	prev_cycles = 0;
}
void CPU::set_flag(byte flag, bool val) {
    if(val) {
        p |= flag;
    } else {
        p &= ~flag;
    }
}
bool CPU::get_flag(byte flag) {
    return flag & p;
}
void CPU::set_nz(byte val) {
    set_flag(Z, val == 0);
    set_flag(N, val & 0x80);
}

byte CPU::get_mem(word addr) {
	cycle_count++;
	return m->get_mem(addr);
}

void CPU::set_mem(word addr, byte val) {
	cycle_count++;
	m->set_mem(addr, val);
}

byte CPU::next_byte() {
    return get_mem(pc++);
}
word CPU::next_word() {
    return next_byte() | (next_byte() << 8);
}

void CPU::nmi() {
    push2(pc);
    push(p);
	set_flag(I, true);
    pc = get_mem(0xfffa) + (get_mem(0xfffb)<<8);
}

void CPU::irq() {
	push2(pc);
    push(p);
	set_flag(I, true);
    pc = get_mem(0xfffe) + (get_mem(0xffff)<<8);
}

void CPU::reset() {
    cycle_count = 0;
    a = x = y = 0;
    p = 0x24;
    s -= 3;
    s &= 0xff;
    pc = get_mem(0xfffc) + (get_mem(0xfffd) << 8);
}

void CPU::push2(word val) {
    s -= 2;
    word ss = 0x0100;
    set_mem(ss | (s + 1), val & 0xff);
    set_mem(ss | (s + 2), val >> 8);
}
word CPU::pop2() {
    s += 2;
    return get_mem(((s-1) & 0xff) | 0x100) + (get_mem(s | 0x100) << 8);
}
void CPU::push(byte val) {
    set_mem(s-- | 0x100, val);
}
byte CPU::pop() {
    return get_mem(++s | 0x100);
}

void CPU::branch(bool cond, Instruction &inst) {
    if(cond) {
		get_mem(pc);
        inst.extra_cycles += 1;
        if ((inst.addr & 0xff00) != (pc & 0xff00)) {
			get_mem((inst.addr & 0xff) | (pc & 0xff00));
            inst.extra_cycles += 1;
        }
        pc = inst.addr;
    }
}

void CPU::compare(byte a, byte b) {
    char sa = a;
    char sb = b;
    set_flag(N, (sa-sb) & (1 << 7));
    set_flag(Z, sa == sb);
    set_flag(C, a >= b);
}

int CPU::execute_inst(Instruction inst) {
    byte t;
    byte a7, m7, r7;
    word result;
    switch(inst.op.op) {
    case NOP:
	case DOP:
    case TOP:
        break;
    case JMP:
        pc = inst.addr;
        break;
    case JSR:
        push2(pc-1);
        pc = inst.addr;
        break;
    case RTS:
		get_mem(0x100 | s);
        pc = pop2()+1;
		get_mem(pc);
        break;
    case RTI:
		get_mem(0x100 | s);
        p = (pop() | (1<<5)) & (~B);
        pc = pop2();
		if(get_flag(I))
			m->scheduled_irq = 0;
		else if(m->irq_waiting)
			m->scheduled_irq = 1;
        break;
    case BRK:
        pc += 1;
        p |= B;
        irq(); 
        break;
    case BCS:
        branch(get_flag(C), inst);
        break;
    case BCC:
        branch(!get_flag(C), inst);
        break;
    case BEQ:
        branch(get_flag(Z), inst);
        break;
    case BNE:
        branch(!get_flag(Z), inst);
        break;
    case BVS:
        branch(get_flag(V), inst);
        break;
    case BVC:
        branch(!get_flag(V), inst);
        break;
    case BPL:
        branch(!get_flag(N), inst);
        break;
    case BMI:
        branch(get_flag(N), inst);
        break;
    case BIT:
        t = inst.operand;
        set_flag(N, t & (1 << 7));
        set_flag(V, t & (1 << 6));
        set_flag(Z, (t & a) == 0);
        break;
    case CMP:
        compare(a, inst.operand);
        break;
    case CPY:
        compare(y, inst.operand);
        break;
    case CPX:
        compare(x, inst.operand);
        break;
    case CLC:
        set_flag(C, false);
        break;
    case CLD:
        set_flag(D, false);
        break;
    case CLV:
        set_flag(V, false);
        break;
    case CLI:
        set_flag(I, false);
        break;
    case SED:
        set_flag(D, true);
        break;
    case SEC:
        set_flag(C, true);
        break;
    case SEI:
        set_flag(I, true);
        break;
    case LDA:
        a = inst.operand;
        set_nz(a);
        break;
    case STA:
        set_mem(inst.addr, a);
        break;
    case LDX:
        x = inst.operand;
        set_nz(x);
        break;
    case STX:
        set_mem(inst.addr, x);
        break;
    case LDY:
        y = inst.operand;
        set_nz(y);
        break;
    case STY:
        set_mem(inst.addr, y);
        break;
    case LAX:
        a = inst.operand;
        x = inst.operand;
        set_nz(a);
        break;
    case SAX:
        set_mem(inst.addr, a & x);
        break;
    case PHP:
        push(p | B);
        break;
    case PLP:
		get_mem(0x100|s);
        p = (pop() | (1 << 5)) & (~B);
        break;
    case PLA:
		get_mem(0x100|s);
        a = pop();
        set_nz(a);
        break;
    case PHA:
        push(a);
        break;
    case AND:
        a &= inst.operand;
        set_nz(a);
        break;
	case AAC:
		a &= inst.operand;
		set_nz(a);
		set_flag(C, a & 0x80);
		break;
    case ORA:
        a |= inst.operand;
        set_nz(a);
        break;
    case EOR:
        a ^= inst.operand;
        set_nz(a);
        break;
    case ADC:
        a7 = a & (1 << 7);
        m7 = inst.operand & (1 << 7);
        result = a + inst.operand;
        if(get_flag(C)) {
            result += 1;
        }
        a = result & 0xff;
        set_flag(C, result > 0xff);
        set_nz(a);
        r7 = a & (1 << 7);
        set_flag(V, !((a7 != m7) || ((a7 == m7) && (m7 == r7))));
        break;
    case SBC:
        a7 = a & (1 << 7);
        m7 = inst.operand & (1 << 7);
        result = a - inst.operand;
        if(!get_flag(C)) {
            result -= 1;
        }
        a = result & 0xff;
        set_flag(C, result < 0x100);
        set_nz(a);
        r7 = a & (1 << 7);
        set_flag(V, !((a7 == m7) || ((a7 != m7) && (r7 == a7))));
        break;
    case INX:
        x += 1;
        set_nz(x);
        break;
    case INY:
        y += 1;
        set_nz(y);
        break;
    case DEX:
        x -= 1;
        set_nz(x);
        break;
    case DEY:
        y -= 1;
        set_nz(y);
        break;
    case INC:
		set_mem(inst.addr, inst.operand);
        inst.operand += 1;
        inst.operand &= 0xff;
        set_nz(inst.operand);
        set_mem(inst.addr, inst.operand);
        break;
    case DEC:
		set_mem(inst.addr, inst.operand);
        inst.operand -= 1;
        inst.operand &= 0xff;
        set_nz(inst.operand);
        set_mem(inst.addr, inst.operand);
        break;
    case DCP:
		set_mem(inst.addr, inst.operand);
        set_mem(inst.addr, (inst.operand -1) & 0xff);
        compare(a, (inst.operand-1)&0xff);
        break;
    case ISB:
		set_mem(inst.addr, inst.operand);
        inst.operand = (inst.operand + 1) & 0xff;
        set_mem(inst.addr, inst.operand);
		a7 = a & (1 << 7);
        m7 = inst.operand & (1 << 7);
        result = a - inst.operand;
        if(!get_flag(C)) {
            result -= 1;
        }
        a = result & 0xff;
        set_flag(C, result < 0x100);
        set_nz(a);
        r7 = a & (1 << 7);
        set_flag(V, !((a7 == m7) || ((a7 != m7) && (r7 == a7))));
        break;
    case LSR_A:
        set_flag(C, a & 1);
        a >>= 1;
        set_nz(a);
        break;
    case LSR:
        set_flag(C, inst.operand & 1);
		set_mem(inst.addr, inst.operand);
        inst.operand >>= 1;
        set_mem(inst.addr, inst.operand);
        set_nz(inst.operand);
        break;
    case ASL_A:
        set_flag(C, a & (1 << 7));
        a <<= 1;
        set_nz(a);
        break;
    case ASL:
        set_flag(C, inst.operand & (1 << 7));
		set_mem(inst.addr, inst.operand);
        inst.operand <<= 1;
        set_mem(inst.addr, inst.operand);
        set_nz(inst.operand);
        break;
	case ASR:
		a &= inst.operand;
		set_flag(C, a & 1);
		a >>= 1;
		set_nz(a);
		break;
	case ARR:
		a &= inst.operand;
		a >>= 1;
		if(get_flag(C))
			a |= 0x80;
		set_flag(C, a & (1<<6));
		set_flag(V, ((a & (1<<5))<<1)  ^ (a & (1<<6)));
		set_nz(a);
		break;
    case TSX:
        x = s;
        set_nz(x);
        break;
    case TXS:
        s = x;
        break;
    case TYA:
        a = y;
        set_nz(a);
        break;
    case TXA:
        a = x;
        set_nz(a);
        break;
	case ATX:
		a |= 0xff;
		a &= inst.operand;
		x = a;
		set_nz(x);
		break;
	case AXS:
		x = a & x;
		result = x - inst.operand;
		set_flag(C, result < 0x100);
		x = result & 0xff;
		set_nz(x);
		break;
	case SYA:
		t = y & (inst.args[1] + 1);
		if(!inst.extra_cycles)
			set_mem(inst.addr, t);
		else
			inst.extra_cycles = 0;
		break;
	case SXA:
		t = x & (inst.args[1] + 1);
		if(!inst.extra_cycles)
			set_mem(inst.addr, t);
		else
			inst.extra_cycles = 0;
		break;
    case ROR_A:
        t = a & 1;
        a >>= 1;
        if(get_flag(C))
            a |= 1 << 7;
        set_flag(C, t);
        set_nz(a);
        break;
    case ROR:
        t = inst.operand & 1;
		set_mem(inst.addr, inst.operand);
        inst.operand >>= 1;
        if(get_flag(C))
            inst.operand |= 1 << 7;
        set_flag(C, t);
        set_mem(inst.addr, inst.operand);
        set_nz(inst.operand);
        break;
    case ROL_A:
        t = a & (1 << 7);
        a <<= 1;
        if(get_flag(C))
            a |= 1;
        set_flag(C, t);
        set_nz(a);
        break;
    case ROL:
        t = inst.operand & (1 << 7);
		set_mem(inst.addr, inst.operand);
        inst.operand <<= 1;
        if(get_flag(C))
            inst.operand |= 1;
        set_flag(C, t);
        set_mem(inst.addr, inst.operand);
        set_nz(inst.operand);
        break;
    case TAY:
        y = a;
        set_nz(y);
        break;
    case TAX:
        x = a;
        set_nz(x);
        break;
    case RLA:
        t = inst.operand & (1 << 7);
		set_mem(inst.addr, inst.operand);
        inst.operand <<= 1;
        if(get_flag(C))
            inst.operand |= 1;
        set_flag(C, t);
        set_mem(inst.addr, inst.operand);
        a &= inst.operand;
        set_nz(a);
        break;
    case SLO:
        set_flag(C, inst.operand & (1 << 7));
		set_mem(inst.addr, inst.operand);
        inst.operand <<= 1;
        set_mem(inst.addr, inst.operand);
        a |= inst.operand;
        set_nz(a);
        break;
	case SRE:
		set_flag(C, inst.operand & 1);
		set_mem(inst.addr, inst.operand);
        inst.operand >>= 1;
        set_mem(inst.addr, inst.operand);
        a ^= inst.operand;
        set_nz(a);
		break;
    case RRA:
        t = inst.operand & 1;
		set_mem(inst.addr, inst.operand);
        inst.operand >>= 1;
        if(get_flag(C))
            inst.operand |= 1 << 7;
        set_mem(inst.addr, inst.operand);
		a7 = a & (1 << 7);
        m7 = inst.operand & (1 << 7);
        result = a + inst.operand;
        if(t) {
            result += 1;
        }
        a = result & 0xff;
        set_flag(C, result > 0xff);
        set_nz(a);
        r7 = a & (1 << 7);
        set_flag(V, !((a7 != m7) || ((a7 == m7) && (m7 == r7))));
		break;
    case XAA:
    case AXA:
    case XAS:
    case LAR:
        break;
    default:
        cout << "Unsupported opcode! " << int(inst.opcode) << endl;
        cout << inst.op.op << endl;
		cout << opnames[inst.op.op] << endl;
        throw new runtime_error("Unsupported opcode");
        break;
    }

    int inst_cycles = cycle_count - prev_cycles;
	prev_cycles = cycle_count;
	//cycle_count += inst.op.cycles + inst.extra_cycles;
	return inst_cycles;
}
