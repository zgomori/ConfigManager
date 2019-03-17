#include "Logger.h"

Logger::Logger(){
	this->level = -1;
	this->logWriter = new DummyLogWriter();
}

void Logger::init(int level, Stream* logOutput){
    this->level     = level;
	 this->logWriter = new SerialLogWriter(logOutput);
}
 
void Logger::init(int level, WiFiUDP* logOutput, char* ipAddress, uint remotePort){
    this->level     = level;
	 this->logWriter = new UdpLogWriter(logOutput, ipAddress, remotePort);
}

void Logger::setLevel(int level){
	this->level = level;
}

int Logger::getLevel(){
	return this->level;
}