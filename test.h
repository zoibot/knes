#include <boost/crc.hpp>

void run(char *romfile, bool debug);
void test(char *testfile);
void test_list(char *testlistfile);
Machine *load(char *romfile);