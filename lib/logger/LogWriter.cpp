#include "LogWriter.h"

/*!
**** Wildcards

* %s	replace with an string (char*)
* %c	replace with an character
* %d	replace with an integer value
* %l	replace with an long value
* %x	replace and convert integer value into hex
* %X	like %x but combine with 0x123AB
* %b	replace and convert integer value into binary
* %B	like %x but combine with 0b10100011
* %t	replace and convert boolean value into "t" or "f"
* %T	like %t but convert into "true" or "false"
*/

/*******************
 * LogWriter       *
 *******************/
void LogWriter::print(const char *format, va_list args) {
	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			printFormat(*format, &args);
		} else {
			logOutput->print(*format);
		}
		yield();
	}
}

void LogWriter::printFormat(const char format, va_list *args) {
	if (format == '\0') return;

	if (format == '%') {
		logOutput->print(format);
		return;
	}

	if( format == 's' ) {
		register char *s = (char *)va_arg( *args, int );
		logOutput->print(s);
		return;
	}

	if( format == 'S' ) {
		register __FlashStringHelper *s = (__FlashStringHelper *)va_arg( *args, int );
		logOutput->print(s);
		return;
	}
  
	if( format == 'd' || format == 'i') {
		logOutput->print(va_arg( *args, int ),DEC);
		return;
	}

	if( format == 'D' || format == 'F') {
		logOutput->print(va_arg( *args, double ));
		return;
	}

	if( format == 'x' ) {
		logOutput->print(va_arg( *args, int ),HEX);
		return;
	}

	if( format == 'X' ) {
		logOutput->print("0x");
		logOutput->print(va_arg( *args, int ),HEX);
		return;
	}

	if( format == 'b' ) {
		logOutput->print(va_arg( *args, int ),BIN);
		return;
	}

	if( format == 'B' ) {
		logOutput->print("0b");
		logOutput->print(va_arg( *args, int ),BIN);
		return;
	}

	if( format == 'l' ) {
		logOutput->print(va_arg( *args, long ),DEC);
		return;
	}

	if( format == 'c' ) {
		logOutput->print((char) va_arg( *args, int ));
		return;
	}

	if( format == 't' ) {
		if (va_arg( *args, int ) == 1) {
			logOutput->print("T");
		}
		else {
			logOutput->print("F");
		}
		return;
	}

	if( format == 'T' ) {
		if (va_arg( *args, int ) == 1) {
		logOutput->print(F("true"));
	}
	else {
		logOutput->print(F("false"));
	}
		return;
	}
}

void LogWriter::printDateTime(){
	char timeBuffer[20];
	time_t ct = now();

	sprintf(timeBuffer, "%u.%02u.%02u %02u:%02u:%02u ", year(ct), month(ct), day(ct), hour(ct), minute(ct), second(ct) );

	logOutput->print(timeBuffer);
}

/*******************
 * SerialLogWriter *
 *******************/
SerialLogWriter::SerialLogWriter(Stream* logOutput){
	this->logOutput = logOutput;
}

/*******************
 * UdpLogWriter    *
 *******************/
UdpLogWriter::UdpLogWriter(WiFiUDP* udp, char* ipAddress, uint remotePort){
	this->logOutput = udp;
	this->ipAddress = ipAddress;
	this->remotePort = remotePort;
}

void UdpLogWriter::beginLogEntry(){
	static_cast<WiFiUDP*>(logOutput)->beginPacket(ipAddress, remotePort);
	yield();
}

void UdpLogWriter::endLogEntry(){
	static_cast<WiFiUDP*>(logOutput)->endPacket();
	delay(2);
}
