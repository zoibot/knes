#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "machine.h"
#include "rom.h"
#include "test.h"
#include "log.h"

Machine *load(char *romfile) {
	ifstream romf(romfile, ifstream::binary | ifstream::in);
	cout << "Loading Rom: " << romfile << endl;
    return new Machine(new Rom(romf, string(romfile)));
}

void run(char *romfile, bool debug = false) {
	Machine *mach = load(romfile);
	mach->debug = debug;
	mach->set_input(new SFMLInputProvider());
    mach->run(0);
}

//TODO better error checking, "test/", (char*) casts
int main(int argc, char *argv[]) {
    // Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
    ("help", "produce help message")
    ("debug", po::value<int>(), "set debug flag")
	("test", po::value< vector<string> >(), "run a single test file")
	("test-list", po::value<string>(), "run a sequence of tests")
	("rom", po::value<string>(), "input rom")
	;
	po::positional_options_description p;
	p.add("rom", -1);
	
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).
			options(desc).positional(p).run(), vm);
		po::notify(vm);
	} catch(po::error e) {
		cout << "Error parsing program options" << endl;
		cout << e.what() << endl;
		return 1;
	}

	if(vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}
	if(vm.count("test")) {
        //run a set of tests listed on the command line
		vector<string> tests = vm["test"].as< vector<string> >();
		for(vector<string>::iterator i = tests.begin(); i != tests.end(); i++) {
			test((char*)(*i).c_str());
		}
		return 0;
	}
	if(vm.count("test-list")) {
        //run a set of tests listed in a file
		test_list((char*)("test/"+vm["test-list"].as<string>()).c_str());
		cin.get();
		return 0;
	}
    bool debug = argc > 2;
	//default run rom
	run((char*)(vm["rom"].as<string>()).c_str(), debug);
    cin.get();
    return 0;
}
