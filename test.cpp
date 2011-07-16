#include <fstream>
#include <string>
#include <sstream>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#ifdef WIN32
#include <windows.h>
#endif

#include "machine.h"
#include "test.h"

using namespace std;

void print_colored(std::string s, int color) {
#ifdef WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	cout << s;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7 /*gray*/);
#else
    cout << "\e[" << color << "m";
	cout << s;
    cout << "\e[" << 0 << "m";
#endif
}

void pass(string message = "") {
	print_colored("pass", CONSOLE_GREEN);
	if(!message.empty())
		cout << " (" << message << ")";
	cout << endl;
}

void fail(string message = "") {
	print_colored("fail", CONSOLE_RED);
	if(!message.empty())
		cout << " (" << message << ")";
	cout << endl;
}

void test(char *testfile) {
	cout << "running test " << testfile << endl;
	ifstream file(testfile);
	if(!file.is_open()) {
		cout << "Failed to open test file: " << testfile << endl;
	}
	string line, command;
	istringstream iline(istringstream::in);
	getline(file, line);
	Machine *m = load((char*)("test/"+line).c_str());
	TestInputProvider *inp = new TestInputProvider();
	m->set_input(inp);
	while(true) {
		getline(file, line);
        if(file.eof()) break;
		iline.clear();
		iline.str(line);
		iline >> command;
		if(command == "wait") {
			int length;
			iline >> length;
			m->run(length);
		} else if(command == "screen") {
			sf::Image i = m->screenshot();
			i.SaveToFile("screen.png");
		} else if(command == "test_image") {
			sf::Image i = m->screenshot();
			boost::crc_optimal<64, 0xFADEEAB39483FCD0> crc;
			crc.process_bytes(i.GetPixelsPtr(), i.GetWidth() * i.GetHeight() * 4);
			boost::uint_fast64_t expected;
			iline >> hex >> expected;
			string message;
			iline >> message;
			cout << message << ": ";
			if(expected == crc.checksum()) {
				pass();
			} else {
				fail();
				stringstream checksum;
				checksum << hex << crc.checksum();
				i.SaveToFile(checksum.str() + ".png");
				cout << "expected " << hex << expected << " got " << checksum.str() << endl;
			}
		} else if(command == "press") {
			char button;
			int button_code;
			iline >> button;
			switch(button) {
			case 'A':
				button_code = 0;
				break;
			case 'B':
				button_code = 1;
				break;
			case 'S':
				button_code = 2;
				break;
			case 'T':
				button_code = 3;
				break;
			case 'U':
				button_code = 4;
				break;
			case 'D':
				button_code = 5;
				break;
			case 'L':
				button_code = 6;
				break;
			case 'R':
				button_code = 7;
				break;
			default:
				cout << "Invalid key: " << button << endl;
				return;
			}
			inp->set_button(button_code, 1, true);
			m->run(2);
			inp->set_button(button_code, 1, false);
		} else if(command == "blargg_test") {
			m->run(0);
			if(m->rom->prg_ram[1] == 0xde && m->rom->prg_ram[2] == 0xb0) {//&& mem[0x6003] == 0x61) {
				switch(m->rom->prg_ram[0]) {
				case 0x80:
					//running
					break;
				case 0x81:
					//need reset
					break;
				case 0x0:
					pass();
					break;
				default:
					fail((char*)(m->rom->prg_ram + 4));
					break;
				}
			}
		}
	}
	delete m;
}

void test_list(char *testlistfile) {
	ifstream file(testlistfile);
	string line;
	while(true) {
		getline(file, line);
        if(file.eof()) break;
		test((char*)("test/"+line).c_str());
	}
}
