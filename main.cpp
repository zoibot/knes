#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "machine.h"
#include "rom.h"

void run(char *romfile, bool debug = true) {
	ifstream romf(romfile, ifstream::binary | ifstream::in);
	cout << "Loading Rom: " << romfile << endl;
    Rom rom(romf, string(romfile));
    Machine mach(&rom);
	mach.debug = debug;
    mach.run();
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        cout << "nope" << endl;
        exit(1);
    }
    bool debug = argc > 2;
	run(argv[1], debug);
    cin.get();
    return 0;
}
