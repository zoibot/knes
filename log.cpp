#include <iostream>
using namespace std;

#include "log.h"

map<string,Logger*> Logger::logger_map;

Logger *Logger::get_logger(string name, ostream &out) {
	if(!logger_map.count(name)) {
		logger_map[name] = new Logger(name, out);
	}
	return logger_map[name];
}

Logger::Logger(string name, ostream &out) : out(out) {
	this->name = name;
}

void Logger::log(string message, string category, LogLevel level) {
	switch(level) {
	case lERROR:
		out << "ERROR ";
        break;
    default:
        break;
	}
	out << "[" << category << "] " << message << endl;
}
