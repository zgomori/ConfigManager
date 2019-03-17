#include "ConfigHandler.h"
#include "base64.hpp"

//public:
ConfigHandler::ConfigHandler(char* configRawContent, uint16_t configRawLength, char* configFileName){
	this->configRawContent = configRawContent;
	this->configRawLength = configRawLength;
	this->configFileName = configFileName;

	this->spiffsInitStatus = SPIFFS.begin();
	if(!this->spiffsInitStatus){
		Serial.println("ConfigHandler ERROR: SPIFFS.begin() failed");
	}	
}	


void ConfigHandler::softApSetup(const char* softApSsid, const char* softApPassword, IPAddress softApIp, IPAddress softApGateway, IPAddress softApSubnet){
	this->softApSsid = softApSsid;
	this->softApPassword = softApPassword;
	this->softApIp = softApIp;
	this->softApGateway = softApGateway;
	this->softApSubnet = softApSubnet;
}

void ConfigHandler::setConfigData(char* configRawContent, uint16_t configRawLength, char* configFileName){
	this->configRawContent = configRawContent;
	this->configRawLength = configRawLength;
	this->configFileName = configFileName;
}

void ConfigHandler::webServerListen(){
	if (isApWebServerActive){
	 	webServer->handleClient();
	}	 
}

void ConfigHandler::startApWebserver(){
	Log.debug("ConfigHandler starting softAP webserver...");
	
	if (!this->isApWebServerActive){
		if (!webServer){
			Log.debug("ConfigHandler setup webserver...");
			this->initWebserver();
		}

		if(WiFi.isConnected()){
			Log.debug("ConfigHandler set WiFi mode to WIFI_AP_STA");
			WiFi.mode(WIFI_AP_STA);
		} 
		else {
			Log.debug("ConfigHandler set WiFi mode to WIFI_AP");
			WiFi.persistent(false);
			WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
			WiFi.mode(WIFI_AP);
			WiFi.persistent(true);
		}

		Log.debug("ConfigHandler softAP IP=%d.%d.%d.%d", softApIp[0], softApIp[1], softApIp[2], softApIp[3]);
		Log.debug("ConfigHandler softAP gateway=%d.%d.%d.%d", softApGateway[0], softApGateway[1], softApGateway[2], softApGateway[3]);
		Log.debug("ConfigHandler softAP subnet=%d.%d.%d.%d", softApSubnet[0], softApSubnet[1], softApSubnet[2], softApSubnet[3]);
		WiFi.softAPConfig(softApIp, softApGateway, softApSubnet);  

		Log.debug("ConfigHandler softAP SSID=%s password=%s", softApSsid, softApPassword);
		WiFi.softAP(softApSsid, softApPassword);
		Log.debug("ConfigHandler starting webserver on port %d", _webServerPort );

		delay(500); // Without delay I've seen the IP address blank
		Log.info("ConfigHandler softAP IP: %d.%d.%d.%d", WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
		webServer->begin();
		this->isApWebServerActive = true;
	}
	else{
		Log.debug("ConfigHandler softAP webserver already active");
	}	
}

void ConfigHandler::stopApWebserver(){
	Log.debug("ConfigHandler stop webserver...");

	if (this->isApWebServerActive){
		webServer->stop();
		Log.debug("ConfigHandler stop softAP");
		WiFi.softAPdisconnect(true);
		this->isApWebServerActive = false;
	}
	else{
		Log.debug("ConfigHandler softAP webserver is already down");
	}	
}

/************************************************************************************
 * Read file from SPIFFS and set the config object 
 * Return values:
 * 	0 - success
 * 	1 - file not found
 * 	2 - configuration error (before readFromFile() must call setConfigData(...))
 * 	3 - invalid file size
 ***********************************************************************************/
uint8_t ConfigHandler::readFromFile(){
	if ( (!configRawContent) || (configRawLength == 0) || (!configFileName)){
		return 2;
	}
	File f = SPIFFS.open(configFileName, "r");
	if (f){
		if (f.size() == configRawLength){
			f.read((uint8_t *)configRawContent, configRawLength);
		}
		else{
			return 3;
		}
		f.close();
	}
	else{
		return 1;
	}

	return 0;
}


/************************************************************************************
 * Write config object content to SPIFFS file 
 * Return values:
 * 	0 - success
 * 	1 - can't create file
 * 	2 - SPIFFS mount failed
 ***********************************************************************************/
uint8_t ConfigHandler::writeToFile(){
	if ( (!configRawContent) || (configRawLength == 0) || (!configFileName)){
		return 3;
	}
	File f = SPIFFS.open(configFileName, "w");
	if (f){
		f.write((uint8_t *)configRawContent, configRawLength);
		f.close();
	}
	else{
		return 1;
	}
	return 0;
}

bool ConfigHandler::deleteFile(){
	return SPIFFS.remove(this->configFileName);
}	


 //private:

/***********************************************************************
 * 
 * 
 * 
 * 
 * 
 ***********************************************************************/


void ConfigHandler::initWebserver(){
	Log.debug("ConfigHandler initWebserver()...");

	webServer = new ESP8266WebServer(_webServerPort);
	
	webServer->serveStatic("/config.html", SPIFFS, "/webserver/config.html");
	webServer->serveStatic("/structjson.css", SPIFFS, "/webserver/structjson.css");
	webServer->serveStatic("/structjson.js", SPIFFS, "/webserver/structjson.js");

	webServer->on("/", [this](){
		Log.debug("ConfigHandler serves request /");
		webServer->send(200, "text/plain", "Config Manager\r\n\r\n" 
		"API endpoint: /config/api\r\n"
		"Methods:\r\n"
		"   getB64\r\n"
		"   updateB64\r\n"
		"   writeToFile\r\n"
		"   reloadFile\r\n"
		"   deleteFile\r\n"
		"   stopAP\r\n"
		"   resetDevice\r\n"
		);
	});

	webServer->on("/config/api/getB64", [this](){
		Log.debug("ConfigHandler serves request /config/api/getB64");
		uint16_t b64Length = encode_base64_length(this->configRawLength);
		char b64[b64Length];

		encode_base64((unsigned  char*)this->configRawContent, this->configRawLength, (unsigned  char*)b64);
		webServer->send(200, "text/plain", b64);
	});

	webServer->on("/config/api/updateB64", [this](){
		Log.debug("ConfigHandler serves request /config/api/updateB64");
		if (webServer->arg("b64")!= ""){
			uint16_t b64Length = webServer->arg("b64").length() + 1;
			char b64[b64Length];
			strcpy(b64, webServer->arg("b64").c_str()) ;

			uint16_t binaryLength = decode_base64_length((unsigned char*)b64);
			if (binaryLength == this->configRawLength){
				decode_base64((unsigned char*)b64, (unsigned char*)this->configRawContent);	
				webServer->send(200, "text/plain", "OK");
			}
			else{
				webServer->send(500, "text/plain", "Invalid content size");	
			}
		}
		else{
			webServer->send(500, "text/plain", "parameter b64 is required");
		}
	});

	webServer->on("/config/api/stopAP", [this](){
		Log.debug("ConfigHandler serves request /config/api/stopAP");		
		webServer->send(200, "text/plain", "OK");
		delay(100);
		this->stopApWebserver();
	});


	webServer->on("/config/api/writeToFile", [this](){
		Log.debug("ConfigHandler serves request /config/api/writeToFile");		
		if (this->writeToFile() == 0){
			webServer->send(200, "text/plain", "OK");
		}
		else{
			webServer->send(500, "text/plain", "ERROR");
		}
	});

	webServer->on("/config/api/reloadFile", [this](){
		Log.debug("ConfigHandler serve request /config/api/reloadFile");
		uint8_t fileReadResult = this->readFromFile();
		if (fileReadResult == 0){
			Log.debug("OK");
			webServer->send(200, "text/plain", "OK");
		}
		else{
			Log.error("Error reading file. Exit code: %u ", fileReadResult);
			webServer->send(500, "text/plain", "ERROR");
		}
	});


	webServer->on("/config/api/deleteFile", [this](){
		Log.debug("ConfigHandler serves request /config/api/deleteFile");
		if (this->deleteFile()){
			webServer->send(200, "text/plain", "OK");
		}
		else{
			webServer->send(500, "text/plain", "ERROR");
		}
	});

	webServer->on("/config/api/resetDevice", [this](){
		Log.debug("ConfigHandler serves request /config/api/deviceReset");
		webServer->send(200, "text/plain", "OK");
		delay(1000);
		this->stopApWebserver();
		delay(2000);
		Log.debug("ConfigHandler reset device, goodbye");
		ESP.reset();
		delay(2000);
	});

	Log.debug("ConfigHandler initWebserver() OK");
}
