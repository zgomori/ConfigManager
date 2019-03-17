#ifndef LOG_WRITER_H
#define LOG_WRITER_H

#include "ESP8266WiFi.h"
#include "WiFiUdp.h"
#include "stdarg.h"
#include "time.h"


/*******************
 * LogWriter       *
 *******************/
class LogWriter{
	public:
		template <class T> 
		void writeLog(int level, T msg, ...){
			beginLogEntry();
			
			printDateTime();
			logOutput->print(LOG_LEVEL_NAMES[level]);
			logOutput->print(' ');
			va_list args;
			va_start(args, msg);
			print(msg,args);

			logOutput->print('\n');

			endLogEntry();
			yield();
		}

	protected:
		const char* LOG_LEVEL_NAMES[5] = {"FATAL", "ERROR", "WARN ", "INFO ", "DEBUG"};

		Print* logOutput;
	
		LogWriter(){};

		virtual void print(const char *format, va_list args);
		virtual void printFormat(const char format, va_list *args);
		virtual void printDateTime();

		virtual void beginLogEntry(){};
		virtual void endLogEntry(){};
};

/*******************
 * DummyLogWriter  *
 *******************/
class DummyLogWriter : public LogWriter{
	public:
		DummyLogWriter(){};
		
		template <class T> 
		void writeLog(int level, T msg, ...){
		}
};

/*******************
 * SerialLogWriter *
 *******************/
class SerialLogWriter : public LogWriter{
	public:
		SerialLogWriter(Stream* logOutput);
};	

/*******************
 * UdpLogWriter    *
 *******************/
class UdpLogWriter : public LogWriter{
	public:
		UdpLogWriter(WiFiUDP* logOutput, char* ipAddress, uint remotePort);

	protected:
		char* ipAddress;
		uint remotePort;

		void beginLogEntry();
		void endLogEntry();	
};


#endif