#include <cstring>

#include "ppu.h"
#include "machine.h"

PPU::PPU(Machine *mach, wxWindow* wind) {
    this->mach = mach;
    this->wind = wind;
    screen.Create(256, 240);
    screen_data = screen.GetData();
    vaddr = 0;
    taddr = next_taddr = false;
    obj_addr = 0;
    mem = new byte[0x4000];
    obj_mem = new byte[0x100];
    cycle_count = 0;
    odd_frame = false;
    last_nmi = 0;
    vbl_off = 0;
    nmi_occurred = false;
    a12high = false;
    horiz_scroll = vert_scroll = false;
    sl = -2;
    cyc = 0;
    pmask = 0;
    pctrl = 0;
    pstat = 0;
    last_vblank_end = last_vblank_start = 0;
    memset(mem, 0xff, 0x4000);
    memset(obj_mem, 0xff, 0x100);
    mirror_table = new word[0x4000];
    for(int i = 0; i < 0x4000; i++)
        mirror_table[i] = i;
    set_mirror(0x3000, 0x2000, 0xf00);
    if(mach->rom->flags6 & 8) {
        cout << "4 screen!!!!" << endl;
    }
    current_mirroring = FOUR_SCREEN;
    set_mirroring(mach->rom->mirror);
}

byte PPU::read_register(byte num) {
    byte ret;
    int cycles;
    switch(num) {
    case 2:
        ret = pstat;
        pstat &= ~(1 << 7);
        latch = false;
        cycles = mach->cpu->get_cycle_count() * 3;
        if(cycles - last_nmi < 3) {
            mach->suppress_nmi();
            if(cycles - last_nmi == 0) {
                ret = pstat;
            }
        }
        return ret;
    case 4:
        ret = obj_mem[obj_addr];
        return ret;
    case 7:
        if(vaddr < 0x3f00) {
            ret = mem_buf;
            mem_buf = get_mem(vaddr);
        } else {
            mem_buf = get_mem(vaddr-0x1000);
            ret = get_mem(vaddr);
        }
        if(pctrl & (1 << 2)) {
            vaddr += 32;
        } else {
            vaddr += 1;
        }
        vaddr &= 0x3fff;
        a12high = vaddr & 0x1000;
        return ret;
    }
    return 0;
}

void PPU::write_register(byte num, byte val) {
    int cycles;
    switch(num) {
    case 0:
        pctrl = val;
        cycles = mach->cpu->get_cycle_count() * 3;
        if(pctrl & (1<<7)) {
            if(((pstat & (1<<7)) || (cycles - vbl_off <= 2)) && !nmi_occurred) {
                mach->request_nmi();
                nmi_occurred = true;
            }
        } else {
            nmi_occurred = false;
            if(cycles - last_nmi < 6) {
                mach->suppress_nmi();
            }
        }
        taddr &= (~(0x3 << 10));
        taddr |= (val & 0x3) << 10;
        break;
    case 1:
        pmask = val;
        break;
    case 3:
        obj_addr = val;
        break;
    case 4:
        obj_mem[obj_addr] = val;
        obj_addr += 1;
        obj_addr &= 0xff;
        break;
    case 5:
        if(latch) {
            taddr &= (~0x73e0);
            taddr |= (val >> 3) << 5;
            taddr |= (val & 0x7) << 12;
        } else {
            taddr &= ~0x1f;
            taddr |= val >> 3;
            xoff = val & 0x7;
            fine_x = val & 0x7;
        }
        latch = !latch;
        break;
    case 6:
        if(latch) {
            taddr &= ~0xff;
            taddr |= val;
            vaddr = taddr;
            a12high = vaddr & 0x1000;
        } else {
            taddr &= 0xff;
            taddr |= (val & 0x3f) << 8;
        }
        latch = !latch;
        break;
    case 7:
        set_mem(vaddr, val);
        if(pctrl & (1 << 2)) {
            vaddr += 32;
        } else {
            vaddr += 1;
        }
        vaddr &= 0x3fff;
        a12high = vaddr & 0x1000;
        break;
    }
}

void PPU::set_mirror(word from, word to, word size) {
    for(word i = 0; i < size; i++) {
        mirror_table[from+i] = to+i;
    }
}

byte PPU::get_mem_mirrored(word addr) {
    return get_mem(mirror_table[addr]);
}

byte PPU::get_mem(word addr) {
    if(addr < 0x2000) {
        a12high = addr & 0x1000;
        int bank = (addr&mach->rom->chr_bank_mask)>>mach->rom->chr_bank_shift;
        int bank_offset = addr&((1<<mach->rom->chr_bank_shift)-1);
        return mach->rom->chr_rom[bank][bank_offset];
    } else if(addr < 0x3000) {
        return mem[mirror_table[addr]];
    } else if(addr < 0x3f00) {
        return get_mem(addr - 0x1000);
    } else {
        if((addr & 0xf) == 0) addr = 0;
        return mem[0x3f00 + (addr&0x1f)];
    }
}

void PPU::set_mem(word addr, byte val) {
    if(addr < 0x2000) {
        a12high = addr & 0x1000;
        mach->rom->chr_rom[(addr&0x1000)>>12][addr&0xfff] = val;
    } else if(addr < 0x3f00) {
        mem[mirror_table[addr]] = val;
    } else {
        if((addr & 0xf) == 0) addr = 0;
        mem[0x3f00 + (addr&0x1f)] = val;
    }
}

void PPU::prefetch_bytes(int start, int cycles) {
    bool bg_enabled = pmask&(1<<3);
    bool sprite_enabled = pmask&(1<<4);
    if(!bg_enabled && !sprite_enabled) {
        return;
    }
    int base_pt_addr = 0;
    if(pctrl & (1<<4)) base_pt_addr = 0x1000;
    int base_spr_addr = 0;
    if(pctrl & (1<<3)) base_spr_addr = 0x1000;
    for(int j = start; j < (start+cycles); j++) {
        if(!(j&1)) continue;
        int i = j/2;
        if(i < 124) {
            int at_base, pt_addr;
            int fineY = (vaddr >> 12) & 7;
            int nt_addr = 0x2000 + (vaddr & 0xfff);
            buf_tile.nt_addr = nt_addr;
            switch(i%4) {
            case 0:
                buf_tile.nt_val = get_mem(nt_addr);
                break;
            case 1:
                at_base = (nt_addr & (~0x3ff)) + 0x3c0;
                buf_tile.attr = get_mem(at_base + ((nt_addr & 0x1f)>>2)
                                                + ((nt_addr&0x3e0)>>7)*8);
                break;
            case 2:
                pt_addr = (buf_tile.nt_val << 4) + base_pt_addr;
                buf_tile.pattern_lo = get_mem(pt_addr + fineY);
                break;
            case 3:
                pt_addr = (buf_tile.nt_val << 4) + base_pt_addr;
                buf_tile.pattern_hi = get_mem(pt_addr + 8 + fineY);
                bg_prefetch.push_back(buf_tile);
                if((vaddr & 0x1f) == 0x1f) {
                    vaddr ^= 0x400;
                    vaddr -= 0x1f;
                } else {
                    vaddr++;
                }
                break;
            }
        } else if(128 <= i && i < 160) {
            Sprite cur = next_sprs[(i-128)/4];
            int tile = 0;
            int ysoff = sl - cur.y;
            if(pctrl&(1<<5)) { //8x16
                if(cur.attrs&(1<<7)) {
                    ysoff = 15-ysoff;
                }
                tile = cur.tile;
                base_spr_addr = (tile&1) << 12;
                tile &= ~1;
                if(ysoff > 7) {
                    ysoff -= 8;
                    tile |= 1;
                }
            } else {
                tile = cur.tile;
                if(cur.attrs & (1<<7)) {
                    ysoff = 7-ysoff;
                }
            }
            int pat = (tile<<4) + base_spr_addr;
            switch(i%4) {
            case 0:
                break;
            case 2:
                next_sprs[(i-128)/4].pattern_lo = get_mem(pat+ysoff);
                break;
            case 3:
                next_sprs[(i-128)/4].pattern_hi = get_mem(pat+8+ysoff);
                break;
            }
        } else if(160 <= i && i < 168) {
            int at_base, pt_addr;
            int fineY = (vaddr >> 12) & 7;
            int nt_addr = 0x2000 + (vaddr & 0xfff);
            buf_tile.nt_addr = nt_addr;
            switch(i%4) {
            case 0:
                buf_tile.nt_val = get_mem(nt_addr);
                break;
            case 1:
                at_base = (nt_addr & (~0x3ff)) + 0x3c0;
                buf_tile.attr = get_mem(at_base + ((nt_addr & 0x1f)>>2)
                                                + ((nt_addr&0x3e0)>>7)*8);
                break;
            case 2:
                pt_addr = (buf_tile.nt_val << 4) + base_pt_addr;
                buf_tile.pattern_lo = get_mem(pt_addr + fineY);
                break;
            case 3:
                pt_addr = (buf_tile.nt_val << 4) + base_pt_addr;
                buf_tile.pattern_hi = get_mem(pt_addr + 8 + fineY);
                bg_prefetch.push_back(buf_tile);
                if((vaddr & 0x1f) == 0x1f) {
                    vaddr ^= 0x400;
                    vaddr -= 0x1f;
                } else {
                    vaddr++;
                }
                break;
            }
        } 
        if(i == 68) {
            if(num_next_sprs == 8) {
                pstat |= 1<<5;
            }
        } else if(i == 126) {
            update_vert_scroll();
        } else if(i == 128) {
            //horizontal scroll
            vaddr &= ~0x41f;
            vaddr |= taddr & 0x1f;
            vaddr |= taddr & 0x400;
            //fineX = xoff; ? TODO
        }
    }
}

void PPU::update_vert_scroll() {
    int fineY = (vaddr & 0x7000) >> 12;
    if(fineY == 7) {
        if((vaddr&0x3ff) >= 0x3e0) {
            vaddr &= ~0x3ff;
        } else {
            vaddr += 0x20;
            if((vaddr&0x3ff) >= 0x3c0) {
                vaddr &= ~0x3ff;
                vaddr ^= 0x800;
            }
        }
    }
    vaddr &= ~0x7000;
    vaddr |= ((fineY + 1) & 7) << 12;
}

void PPU::new_scanline() {
    vert_scroll = false;
    horiz_scroll = false;
    fine_x = xoff;
    if(bg_prefetch.size() < 1) {
        cout << "ran out of tiles" << endl;
    } else {
        while(bg_prefetch.size() > 2) {
            bg_prefetch.pop_front();
        }
        cur_tile = bg_prefetch.front();
        bg_prefetch.pop_front();
    }
    //sprites
    num_sprs = num_next_sprs;
    for(int i = 0; i < num_sprs; i++) {
        cur_sprs[i] = next_sprs[i];
    }
    num_next_sprs = 0;
    int cury = sl;
    if(cury == 240) {
        num_next_sprs = 0;
        return;
    }
    for(int i = 0; i < 64; i++) {
        Sprite *s = (Sprite*)(obj_mem+ (4*i));
        if(s->y <= cury && ((cury < (s->y+8)) || ((pctrl & (1<<5)) && (cury < (s->y+16))))) {
            if(num_next_sprs == 8) {
                break;
            }
            next_sprs[num_next_sprs] = *s;
            next_sprs[num_next_sprs++].index = i; 
        }
    }
}

void PPU::do_vblank(bool rendering_enabled) {
    int cycles = mach->cpu->get_cycle_count() * 3 - cycle_count;
    if(341 - cyc > cycles) {
        cyc += cycles;
        cycle_count += cycles;
    } else {
        cycle_count += 341 - cyc;
        cyc = 0;
        sl += 1;
        if(rendering_enabled) {
            fine_x = xoff;
        }
    }
}

void PPU::render_pixels(byte x, byte y, byte num) {
    bool bg_enabled = pmask & (1 << 3);
    bool sprite_enabled = pmask & (1 << 4);
    int xoff = cyc;
    while(num) {
        byte row = (cur_tile.nt_addr >> 6) & 1;
        byte col = (cur_tile.nt_addr & 2) >> 1;
        byte at_val = cur_tile.attr;
        at_val >>= 4 * row + 2 * col;
        at_val &= 3;
        at_val <<= 2;
        byte hi = cur_tile.pattern_hi;
        byte lo = cur_tile.pattern_lo;
        hi >>= (7-fine_x);
        hi &= 1;
        hi <<= 1;
        lo >>= (7-fine_x);
        lo &= 1;
        word coli = 0x3f00;
        if((hi|lo) && bg_enabled && !(xoff < 8 && !(pmask & 2)))
            coli |= at_val | hi | lo;
        if(sprite_enabled && !(xoff < 8 && !(pmask & 4))) {
            Sprite *cur = NULL;
            for(int i = 0; i < num_sprs; i++) {
                if((cur_sprs[i].x <= xoff) && (xoff < (cur_sprs[i].x+8))) {
                    cur = &cur_sprs[i];
                    byte pal = (1<<4) | ((cur->attrs & 3) << 2);
                    byte xsoff = xoff-cur->x;
                    if(cur->attrs & (1<<6))
                        xsoff = 7-xsoff;
                    byte shi = cur->pattern_hi;
                    byte slo = cur->pattern_lo;
                    shi >>= (7-xsoff);
                    shi &= 1;
                    shi <<= 1;
                    slo >>= (7-xsoff);
                    slo &= 1;
                    if((cur->index == 0) && (shi|slo) && (hi|lo) && bg_enabled && !(xoff < 8 && !(pmask & 2)) && xoff < 255) {
                        pstat |= 1<<6; // spr hit 0
                    }
                    if((!(hi|lo) && (shi|slo)) || !(cur->attrs & (1<<5))) {
                        if(shi|slo) {
                            coli = 0x3f00 | pal | shi | slo;
                            break;
                        }
                    }
                }
            }
        }
        int color = colors[get_mem(coli)];
        unsigned char *pixel = screen_data+3*(xoff + 256 * y);
        pixel[0] = (color & 0xff0000)>>16;
        pixel[1] = (color & 0x00ff00)>> 8;
        pixel[2] = (color & 0x0000ff);
        fine_x++;
        fine_x &= 7;
        xoff++;
        num--;
        if(!fine_x) {
            if(bg_prefetch.size() < 1) {
            } else {
                cur_tile = bg_prefetch.front();
                bg_prefetch.pop_front();
            }
        }
    }
}

void PPU::draw_frame() {
    sl = -2;
    num_frames++;
    //process events
    /*bool paused = false;
    do {
        while (wind->) {
            if (event.Type == sf::Event::Closed) {
                mach->save();
                wind->Close();
                exit(0);
            } else if (event.Type == sf::Event::KeyReleased) {
                if(event.Key.Code == sf::Keyboard::T) {
                    screen.SaveToFile("sshot.jpg");
                } else if(event.Key.Code == sf::Keyboard::N) {
                    dump_nts();
                } else if(event.Key.Code == sf::Keyboard::P) {
                    paused = !paused;
                } else if(event.Key.Code == sf::Keyboard::Y) {
                    for(int i = 0; i < 64; i++) {
                        Sprite *s = ((Sprite*)obj_mem)+i;
                        if(s->y < 16) {
                            cout << (int)s->y << endl;
                        }
                    }
                } else if(event.Key.Code == sf::Keyboard::Q) {
                    //ZOOOM
                    wind->SetSize(1024, 960);
                } else if(event.Key.Code == sf::Keyboard::F) {
                    stringstream fps;
                    fps << "fps: " << 1000.0f/float(wind->GetFrameTime());
                    log(fps.str());
                }
            }
        }
    } while (paused);*/
}

void PPU::run() {
    bool bg_enabled = pmask & (1 << 3);
    bool sprite_enabled = pmask & (1 << 4);
    bool rendering_enabled = bg_enabled || sprite_enabled;
    while(cycle_count < mach->cpu->get_cycle_count() * 3) {
        int cycles = mach->cpu->get_cycle_count() * 3 - cycle_count;
        if(sl == -2) {
            do_vblank(rendering_enabled);
        } else if(sl == -1) {
            switch(cyc) {
            case 0:
                pstat &= ~(1 << 7);
                pstat &= ~(1 << 6);
                pstat &= ~(1 << 5);
                vbl_off = cycle_count;
                cycle_count += 304;
                cyc += 304;
                break;
            case 304:
                if(bg_enabled) {
                    vaddr = taddr;
                }
                cycle_count += 36;
                cyc += 36;
                if(bg_enabled)
                    get_mem(0x1000); //hack for MMC3
                break;
            case 340:
                if(bg_enabled) {
                    if(odd_frame) cycle_count -= 1;
                }
                odd_frame = !odd_frame;
                cyc++;
                cycle_count++;
                break;
            case 341:
                if(bg_enabled) {
                    prefetch_bytes(320, 21);
                }
                cyc = 0;
                sl++;
                if(bg_enabled) 
                    new_scanline();
                break;
            }
        } else if(sl < 240) {
            int todo = 0;
            if(341 - cyc > cycles) {
                todo = cycles;
            } else {
                todo = 341 - cyc;
            }
            int y = sl;
            if(rendering_enabled) {
                prefetch_bytes(cyc, todo);
                if(cyc < 256)
                    render_pixels(cyc, y, min(todo, 256-cyc));
            }
            cyc += todo;
            cycle_count += todo;
            if(cyc == 341) {
                cyc = 0;
                sl += 1;
                if(rendering_enabled) {
                    new_scanline();
                }
            }
        } else if(sl == 240) {
            if(341 - cyc > cycles) {
                cyc += cycles;
                cycle_count += cycles;
            } else {
                cycle_count += 341 - cyc;
                cyc = 0;
                sl += 1;
                pstat |= (1 << 7);
                last_nmi = cycle_count;
                last_vblank_start = mach->cpu->get_cycle_count();
                if(pctrl & (1 << 7)) {
                    mach->request_nmi();
                    nmi_occurred = true;
                } else {
                    nmi_occurred = false;
                }
            }
        } else if(sl == 241) {
            if(341 - cyc > cycles) {
                cycle_count += cycles;
                cyc += cycles;
            } else {
                cycle_count += 341 - cyc;
                cyc = 0;
                sl++;
            }
        } else {
            cycle_count += 341 * 18;
            draw_frame();
        }
    }           
}

void PPU::set_mirroring(NTMirroring mirror) {
    if(mirror == current_mirroring) return;
    switch(mirror) {
    case VERTICAL:
        set_mirror(0x2000, 0x2000, 0x400);
        set_mirror(0x2400, 0x2400, 0x400);
        set_mirror(0x2800, 0x2000, 0x400);
        set_mirror(0x2c00, 0x2400, 0x400);
        break;
    case HORIZONTAL:
        set_mirror(0x2000, 0x2000, 0x400);
        set_mirror(0x2400, 0x2000, 0x400);
        set_mirror(0x2800, 0x2400, 0x400);
        set_mirror(0x2c00, 0x2400, 0x400);
        break;
    case SINGLE_LOWER:
        set_mirror(0x2000, 0x2000, 0x400);
        set_mirror(0x2400, 0x2000, 0x400);
        set_mirror(0x2800, 0x2000, 0x400);
        set_mirror(0x2c00, 0x2000, 0x400);
        break;
    case SINGLE_UPPER:
        set_mirror(0x2000, 0x2400, 0x400);
        set_mirror(0x2400, 0x2400, 0x400);
        set_mirror(0x2800, 0x2400, 0x400);
        set_mirror(0x2c00, 0x2400, 0x400);
        break;
    case SINGLE_THIRD:
        set_mirror(0x2000, 0x2800, 0x400);
        set_mirror(0x2400, 0x2800, 0x400);
        set_mirror(0x2800, 0x2800, 0x400);
        set_mirror(0x2c00, 0x2800, 0x400);
        break;
    case SINGLE_FOURTH:
        set_mirror(0x2000, 0x2c00, 0x400);
        set_mirror(0x2400, 0x2c00, 0x400);
        set_mirror(0x2800, 0x2c00, 0x400);
        set_mirror(0x2c00, 0x2c00, 0x400);
        break;
    default:
        break;
    }
}

void PPU::log(string message, LogLevel level) {
    stringstream x;
    x << "sl: " << sl << " cyc: " << cyc << " vaddr: " << HEX4(vaddr) << " " << message;
    Logger::get_logger("main")->log(x.str(), "ppu", level);
}

void PPU::dump_nts() {
    /*debug.Create(sf::VideoMode(512, 480), "debug");
    word base_pt_addr;
    if(pctrl & (1<<4)) {
        base_pt_addr = 0x1000;
    } else {
        base_pt_addr = 0x0;
    }
    int x = 0;
    int y = 0;
    for(int nt = 0x2000; nt < 0x3000; nt+=0x400) {
        word at_base = nt + 0x3c0;
        for(int ntaddr = nt; ntaddr < nt+0x3c0; ntaddr++) {
            byte nt_val = get_mem(ntaddr);
            word pt_addr = (nt_val << 4) + base_pt_addr;
            byte row = (ntaddr >> 6) & 1;
            byte col = (ntaddr & 2) >> 1;
            byte at_val = get_mem(at_base + ((ntaddr & 0x1f)>>2) + ((ntaddr & 0x3e0) >> 7)*8);
            at_val >>= 4 * row + 2 * col;
            at_val &= 3;
            at_val <<= 2;
            for(int fy = 0; fy < 8; fy++) {
                for(int fx = 0; fx < 8; fx++) {
                    byte hi = get_mem(pt_addr+8+fy);
                    byte lo = get_mem(pt_addr+fy);
                    hi >>= (7-fx);
                    hi &= 1;
                    hi <<= 1;
                    lo >>= (7-fx);
                    lo &= 1;
                    word coli = 0x3f00;
                    if(hi|lo)
                        coli |= at_val | hi | lo;
                    int color = colors[get_mem(coli)];
                    debugi.SetPixel(x+fx, y+fy, sf::Color((color & 0xff0000)>>16, (color & 0x00ff00) >> 8, color & 0x0000ff));
                }
            }
            x += 8;
            if(x % 256 == 0) {
                x -= 256;
                y += 8;
            }
        }
        x += 256;
        y -= 240;
        if(x == 512) {
            x = 0;
            y = 240;
        }
    }
    debug.Draw(sf::Sprite(debugi));
    debug.Display(); */
}
