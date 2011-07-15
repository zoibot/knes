#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <map>

using namespace std;

enum LogLevel {
	lINFO,
	lWARNING,
	lERROR,
};

class Logger {
	static map<string, Logger*> logger_map;
	ostream &out;
	string name;
public:
	static Logger *get_logger(string name, ostream &out = cout);
	Logger(string name, ostream &out);
	void log(string message, string category = "default", LogLevel level = lINFO);
	void info(string message, string category = "default");
	void warning(string message, string category = "default");
	void error(string message, string category = "default");
};

#endif //LOG_H