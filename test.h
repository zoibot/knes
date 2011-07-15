#include <iomanip>
#include <boost/crc.hpp>

#ifdef WIN32
const int CONSOLE_GREEN = 10;
const int CONSOLE_RED = 12;
#else
const int CONSOLE_GREEN = 31;
const int CONSOLE_RED = 32;
#endif

void run(char *romfile, bool debug);
void test(char *testfile);
void test_list(char *testlistfile);
Machine *load(char *romfile);