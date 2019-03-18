#ifndef CONFIG_MANAGER_H
	#define CONFIG_MANAGER_H

	#include <ESP8266WiFi.h>
	#include <ESP8266WebServer.h>
	#include <FS.h>
	#include "Logger.h"

	class ConfigManager{
		private:
			bool isApWebServerActive = false;

			IPAddress softApIp 		= IPAddress(192,168,168,168);	
			IPAddress softApGateway = IPAddress(192,168,168,168);
			IPAddress softApSubnet = IPAddress(255,255,255,0);
			const char* softApSsid = "ESP8266_AP";
			const char* softApPassword = "Aa123456";
			
			ESP8266WebServer* webServer;
			const uint16_t _webServerPort = 80;

			char* configRawContent = nullptr;
			uint16_t configRawLength = 0;
			char* configFileName = nullptr;

			void initWebserver();

		public:
			ConfigManager(char* configRawContent, uint16_t configRawLength, char* configFileName);
			void softApSetup(const char* softApSsid, const char* softApPassword, IPAddress softApIp, IPAddress softApGateway, IPAddress softApSubnet);
			void setConfigData(char* configRawContent, uint16_t configRawLength, char* configFileName);
			void webServerListen();
			void startApWebserver();
			void stopApWebserver();
			uint8_t readFromFile();
			uint8_t writeToFile();
			bool deleteFile();
	};

extern Logger Log;
#endif 