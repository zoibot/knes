#ifndef PPU_H
#define PPU_H

#include <list>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "util.h"

class Machine;

struct Sprite {
    byte y;
    byte tile;
    byte attrs;
    byte x;
	byte index;
    byte pattern_lo;
    byte pattern_hi;
};

struct Tile {
    word nt_addr;
    byte nt_val;
    byte attr;
    byte pattern_lo;
    byte pattern_hi;
};

class PPU {
private:
	bool debug_flag;
    Machine *mach;
    sf::RenderWindow *wind;
	sf::RenderWindow debug;
	sf::Image debugi;
	//cycles
	int cycle_count;
	bool nmi_occurred;
	bool odd_frame;
	int last_nmi;
	int vbl_off;
    //memory
    byte* mem;
    byte mem_buf;
    word* mirror_table;
    bool latch;
    byte pmask;
    byte pstat;
    byte pctrl;
	//prefetch
    list<Tile> bg_prefetch;
    Tile buf_tile;
    Tile cur_tile;
    Sprite next_sprs[8];
    int num_next_sprs;
    Sprite cur_sprs[8];
    int num_sprs;
    //position
    byte xoff, fine_x;
	bool horiz_scroll, vert_scroll;
    //helpers
    void do_vblank(bool rendering_enabled);
    void render_pixels(byte x, byte y, byte num);
    void new_scanline();
    void update_vert_scroll();
    void draw_frame();
    void prefetch_bytes(int start, int cycles);
    byte get_mem_mirrored(word addr);
    void set_mirror(word from, word to, word size);
	NTMirroring current_mirroring;
public:
	sf::Image screen;
	int last_vblank_start;
	int last_vblank_end;
	int num_frames;
	word vaddr, taddr;
    word next_taddr;
    int sl;
    word cyc;
    byte* obj_mem;
    word obj_addr;
	bool a12high;
	void dump_nts();
    void run();
	void set_mirroring(NTMirroring mirror);
    void write_register(byte num, byte val);
    byte read_register(byte num);
    byte get_mem(word addr);
    void set_mem(word addr, byte val);
    PPU(Machine* mach, sf::RenderWindow* wind);
};

#endif //PPU_H
