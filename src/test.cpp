#include "Arduino.h"
#include "ConfigHandler.h"
#include "Logger.h"


struct  WsnTSnodeConfig{ 
  char      name[16];
  char      thingSpeakReadKey[20];      // write API key
  char      thingSpeakChannel[8];
  int8_t    fieldMapping[5] = {-1, -1, -1, -1, -1}; // Mapping thingSpeak field numbers to sensor types. Fist element is temp, second is humidity etc. 
  uint8_t   nodeID;                   // first free nodeID is 6. (0 is the local sensor, 1-5 are the radio nodes)
  uint64_t  readFrequencyMs;
};

struct WsnReceiverConfig{
  uint64_t  radioNetworkAddress;        // 8 digit hex address. (0xA0A0A0FFLL)  Full tx address is radioNetworkAddress + nodeID (0xA0A0A0FFLL01)
  int   radioChannel;
  char    wifiSsid[20];
  char    wifiPass[20];
  char    ntpServerName[20];
  int8_t    timeZone;
  uint     localUdpPort; 
  char    thingSpeakAddress[20] = "api.thingspeak.com";
  char    thingSpeakAPIKeyArr[6][20];
  byte    sensorSet;                    // bits: messageCnt, voltage, pressure, humidity, temperature.  The least significant bit is temperature
  uint64_t  sensorReadFrequencyMs;        // sensor reading ms
  float   elevation;              // elevation for calculation of relative air pressure (sea level pressure);
  WsnTSnodeConfig tsNodeConfigArr[2];   // max array size: 4 
};

Logger Log;

WsnReceiverConfig cfg;

ConfigHandler configHandler((char*)&cfg, sizeof(cfg), (char*)"config/wsnconfig");

const char* wifiStatusToString(wl_status_t status);

void setup(){
	Serial.begin(115200);
	Serial.println("start");

	Log.init(LOG_LEVEL::DEBUG, &Serial);

	configHandler.softApSetup("ESP8266_1", "123456789", IPAddress(192,168,168,1), IPAddress(192,168,168,1), IPAddress(255,255,255,0));

//	readConfig(cfg);
	configHandler.readFromFile();

	WiFi.begin(cfg.wifiSsid, cfg.wifiPass);

	uint32_t startMillis = millis();
	while (millis() - startMillis < 10000){
		Log.debug("Connecting to SSID %s ... Status: %s", cfg.wifiSsid, wifiStatusToString(WiFi.status()));
		if (WiFi.status() == WL_CONNECTED){
			break;	
		} 
		delay(1000);
	}


	if (WiFi.status() == WL_CONNECTED){
		Log.info("WiFi Connected to SSID %s", cfg.wifiSsid);
	}	
	else{
		Log.info("WiFi Connection failed.");
	}

	configHandler.startApWebserver();
}

void loop() {
  //server.handleClient();
  configHandler.webServerListen();
}



#define WSN_TEMPERATURE 0
#define WSN_HUMIDITY 1
#define WSN_PRESSURE 2
#define WSN_BATTERY 3
#define WSN_MESSAGES 4

void readConfig(WsnReceiverConfig &_cfg){  
  _cfg.radioNetworkAddress = 0xA0A0A0FFLL;
  _cfg.radioChannel = 101;
//  strcpy(_cfg.wifiSsid, "");
//  strcpy(_cfg.wifiPass,"");
  
  strcpy(_cfg.ntpServerName, "time.google.com");
  _cfg.timeZone = 1;
  _cfg.localUdpPort = 8760; 

  strcpy(_cfg.thingSpeakAPIKeyArr[0],"JXWWMBZMQZNRMOJK");  
  strcpy(_cfg.thingSpeakAPIKeyArr[1],"5LXV4LVUS2D6OEJA");
  strcpy(_cfg.thingSpeakAPIKeyArr[2],"5LXV4LVUS2D6OEJA");
  _cfg.sensorSet = 23;
  _cfg.sensorReadFrequencyMs = 60000L;
  _cfg.elevation = 126;

  strcpy(_cfg.tsNodeConfigArr[0].name, "Peti");
  strcpy(_cfg.tsNodeConfigArr[0].thingSpeakReadKey, "9ZTPTLMLNFU8VZU3");
  strcpy(_cfg.tsNodeConfigArr[0].thingSpeakChannel, "340091");
  _cfg.tsNodeConfigArr[0].fieldMapping[WSN_TEMPERATURE] = 1;
  _cfg.tsNodeConfigArr[0].fieldMapping[WSN_HUMIDITY] = 2;
  _cfg.tsNodeConfigArr[0].nodeID = 6;
  _cfg.tsNodeConfigArr[0].readFrequencyMs = 61000L;

  strcpy(_cfg.tsNodeConfigArr[1].name, "Central");
  strcpy(_cfg.tsNodeConfigArr[1].thingSpeakReadKey, "JXWWMBZMQZNRMOJK");
  strcpy(_cfg.tsNodeConfigArr[1].thingSpeakChannel, "528401");
  _cfg.tsNodeConfigArr[1].fieldMapping[WSN_TEMPERATURE] = 1;
  _cfg.tsNodeConfigArr[1].fieldMapping[WSN_HUMIDITY] = 2;
  _cfg.tsNodeConfigArr[1].fieldMapping[WSN_PRESSURE] = 3;
  _cfg.tsNodeConfigArr[1].fieldMapping[WSN_MESSAGES] = 4;
  _cfg.tsNodeConfigArr[1].nodeID = 7;
  _cfg.tsNodeConfigArr[1].readFrequencyMs = 60000L;

}

const char* wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
  return "UNKNOWN_STATUS";
}