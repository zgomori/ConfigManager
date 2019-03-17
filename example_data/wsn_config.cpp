struct WsnTSnodeConfig{ 
	char      name[16];
	char      thingSpeakReadKey[20];  		// write API key
	char      thingSpeakChannel[8];
	int8_t    fieldMapping[5] = {-1, -1, -1, -1, -1}; // Mapping thingSpeak field numbers to sensor types. Fist element is temp, second is humidity etc. 
	uint8_t   nodeID;         					// first free nodeID is 6. (0 is the local sensor, 1-5 are the radio nodes)
	uint64_t  readFrequencyMs;
};

struct WsnReceiverConfig{
	uint64_t  radioNetworkAddress;        // 8 digit hex address. (0xA0A0A0FFLL)  Full tx address is radioNetworkAddress + nodeID (0xA0A0A0FFLL01)
	int		radioChannel;
	char		wifiSsid[20];
	char		wifiPass[20];
	char		ntpServerName[20];
	int8_t  	timeZone;
	uint	   localUdpPort; 
	char		thingSpeakAddress[20] = "api.thingspeak.com";
	char		thingSpeakAPIKeyArr[6][20];
	byte		sensorSet;                  	// bits: messageCnt, voltage, pressure, humidity, temperature.  The least significant bit is temperature
	uint64_t	sensorReadFrequencyMs;      	// sensor reading ms
	float		elevation;							// elevation for calculation of relative air pressure (sea level pressure);
	WsnTSnodeConfig tsNodeConfigArr[2];		// max array size: 4 
};
