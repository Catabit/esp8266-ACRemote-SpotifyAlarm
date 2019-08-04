#pragma once
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>
#include <WakeOnLan.h>
#include <EEPROM.h>
#include "IR_remote.h"
#include "AC_commands.h"
#include "SpotifyClient.h"
#include "Spotify_commands.h"
#include "DHTesp.h"

#include "homePageHTML.h"

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

#define NTP_PACKET_SIZE 48
#define NTP_TIMEOUT 1500
#define TIMEZONE +2 // will be subtracted from the UTC time

#define WAKE_TIMEOUT (5*60000) // 5 minutes

#define EEPROM_SETTINGS_VERSION 3
#define EEPROM_SETTINGS_SIZE 128 // make sure the struct EEPROM_Settings fits
#define DEFAULT_SPOTIFY_URI "spotify:playlist:3GfvDSQNj4VjyB2Rcw36uE"

#define DEFAULT_ALARM_VOLUME 60

class WebServer
{

	struct EEPROM_Settings {
		unsigned char version;
		bool DSTactive;
		char savedURI[100];
		unsigned long alarmUTC;
		uint8_t alarmVolume;
	};

public:
	WebServer();
	~WebServer();

	void handleClient();
	void wakePC();
	unsigned long updateTime();
	void WebServer_init(SpotifyData *data, SpotifyAuth *auth, SpotifyClient *client);
	float getTemperature();
	float getHumidity();
	float getHeatIndex();
	void updateDHT();

	//manually adds seconds to the local UTC time that is synced every now and then with updateTime
	void addTime(long seconds);
	

	void setDST(bool state);
	bool getDST();

	void disableAlarm();

private:
	WiFiUDP UDP;
	
	DHTesp dht;
	float humidity=0, temperature=0, heatIndex=0;

	unsigned int localPort = 2390;      // local port to listen for UDP packets
	const char* ntpServerName = "ro.pool.ntp.org";
	IPAddress timeServerIP;
	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

	SpotifyClient* _client;
	SpotifyData* _data;
	SpotifyAuth* _auth;

	bool alarmSet = false, alarmTriggered = false;
	unsigned long alarmUTC = 0;
	unsigned long timeUTC = 0;
	uint8_t hour, minute, second;
	bool isDST = true;

	void checkForAlarm();

	EEPROM_Settings saved_settings;

	ESP8266WebServer *esp8266webserver;

	IR_remote ir_remote;

	IPAddress computer_ip_broadcast;
	IPAddress computer_ip;
	byte mac[6] = { 0x40, 0x8D, 0x5C, 0xBB, 0x51, 0xCB }; //40-8D-5C-BB-51-CB


	void redirectToHome();
	void handleRoot();
	void handleIr();
	void handlePlayer();
	void handleNotFound();
	void handleWOL();
	void handleAlarm();

	void sendNTPpacket(IPAddress& address);
	void triggerAlarm();

	inline unsigned long mod(long n, long m) {
		return ((n % m) + m) % m;
	}
};

