#include <fstream>
#include <string>
#include <sstream>
#include <boost/crc.hpp>

#include "machine.h"
#include "test.h"

using namespace std;

void test(char *testfile) {
	cout << "running test " << testfile << endl;
	ifstream file(testfile);
	if(!file.is_open()) {
		cout << "Failed to open test file: " << testfile << endl;
	}
	string line, command;
	istringstream iline;
	getline(file, line);
	Machine *m = load((char*)("test/"+line).c_str());
	while(getline(file, line)) {
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
			cout << crc.checksum() << endl;
			i.SaveToFile("last_test.png");
		}
	}
	delete m;
}

void test_list(char *testlistfile) {
	ifstream file(testlistfile);
	string line;
	while(getline(file, line)) {
		test((char*)("test/"+line).c_str());
	}
}