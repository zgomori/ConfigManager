#ifndef LOGGER_H
#define LOGGER_H

/*
Based on https://github.com/thijse/Arduino-Log
*/

#include <stdarg.h>
#include "Arduino.h"
#include "LogWriter.h"

enum LOG_LEVEL:uint8_t{
	FATAL   = 0,
	ERROR   = 1,
	WARNING = 2,
	INFO    = 3,
	DEBUG   = 4
};

class Logger {
	public:
		Logger();

		void init(int level, Stream* logOutput);
		void init(int level, WiFiUDP* logOutput, char* ipAddress, uint remotePort);

		void setLevel(int level);
		int getLevel();

		template <class T, typename... Args> 
		void fatal(T msg, Args... args){
			writeLog(LOG_LEVEL::FATAL, msg, args...);
		}
		
		template <class T, typename... Args> 
		void error(T msg, Args... args){
			writeLog(LOG_LEVEL::ERROR, msg, args...);
		}
		
		template <class T, typename... Args> 
		void warning(T msg, Args...args){
			writeLog(LOG_LEVEL::WARNING, msg, args...);
		}
		
		template <class T, typename... Args> 
		void info(T msg, Args...args){
			writeLog(LOG_LEVEL::INFO, msg, args...);
		}

		template <class T, typename... Args> 
		void debug(T msg, Args... args){
			writeLog(LOG_LEVEL::DEBUG, msg, args...);
		}

	private:
		int level;
		LogWriter* logWriter;

		template <class T, typename... Args> void writeLog(int level, T msg, Args... args){
			if (level <= this->level) {
				logWriter->writeLog(level, msg, args...);
				yield();
			}
		}
};

#endif
