#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include "SpotifyClient.h"
#include "WebServer.h"

#include "settings.h"



#define TIMER_SECOND_COUNT 1000 //every second
#define TIMER_FAST_COUNT 10000 //every 10 seconds
#define TIMER_MEDIUM_COUNT (5* 60000) // every 5 minutes
#define TIMER_SLOW_COUNT (30*60000) // every 30 minutes

 // Spotify settings
String clientId = SPOTIFY_CLIENTID;
String clientSecret = SPOTIFY_SECRET;

IPAddress ip(10, 20, 30, 40);
IPAddress gateway(10, 20, 30, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);

//	http: //esp.8266.ip.address/callback/
String redirectUri = String("http%3A%2F%2F"+ip.toString()+"%2Fcallback%2F");


WebServer webserver;

SpotifyClient client(clientId, clientSecret, redirectUri);
SpotifyData data;
SpotifyAuth auth;

unsigned long UTCtime;

uint16_t counter = 0;
long lastSecondUpdate = TIMER_SECOND_COUNT;
long lastFastUpdate = TIMER_FAST_COUNT;
long lastMediumUpdate = TIMER_MEDIUM_COUNT;
long lastSlowUpdate = 0; //don't execute on first loop, actually wait the duration even for the 1st run

void saveRefreshToken(String refreshToken);
String loadRefreshToken();

void setup() {
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	Serial.println();
	Serial.print("connecting to ");
	Serial.println(WIFI_SSID);

	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gateway, subnet, dns1, dns2);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println(F("WiFi connected"));
	Serial.println(F("IP address: "));
	Serial.println(WiFi.localIP());

	ArduinoOTA.setPort(OTA_PORT);
	ArduinoOTA.setHostname(OTA_NAME);
	ArduinoOTA.setPassword(OTA_PASSWORD);

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";
		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.print(F("Start updating "));
		Serial.println(type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println(F("\nEnd"));
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
		else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
		else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
		else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
		else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
	});
	ArduinoOTA.begin();

	boolean mounted = SPIFFS.begin();

	if (!mounted) {
		Serial.println(F("FS not formatted. Doing that now"));
		SPIFFS.format();
		Serial.println(F("FS formatted..."));
		SPIFFS.begin();
	}

	String code = "";
	String grantType = "";
	String refreshToken = loadRefreshToken();
	if (refreshToken == "") {
		Serial.println(F("No refresh token found. Requesting through browser"));
		Serial.print(F("Open browser at http://"));
		Serial.println(ip.toString());

		code = client.startConfigPortal();
		grantType = "authorization_code";
	}
	else {
		Serial.println(F("Using refresh token found on the FS"));
		code = refreshToken;
		grantType = "refresh_token";
	}
	client.getToken(&auth, grantType, code);
	Serial.printf("Refresh token: %s\nAccess Token: %s\n", auth.refreshToken.c_str(), auth.accessToken.c_str());
	if (auth.refreshToken != "") {
		saveRefreshToken(auth.refreshToken);
	}

	webserver.WebServer_init(&data, &auth, &client);

	Serial.print(F("Updating PC Spotify id:"));
	client.getAvailableDevices(&auth);

	if (client.PC_id == "") {
		Serial.println(F("PC offline"));
	}

	UTCtime = webserver.updateTime();
	lastMediumUpdate = millis();
	delay(1);

	digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {

	if (millis() - lastMediumUpdate > TIMER_MEDIUM_COUNT || UTCtime == 0) {
		lastMediumUpdate = millis();

		Serial.print(F("Updating time: "));
		UTCtime = webserver.updateTime();
		if (UTCtime == 0) {
			Serial.println(F("Could not update time!"));
		}
	}

	if (millis() - lastSlowUpdate > TIMER_SLOW_COUNT) {
		lastSlowUpdate = millis();

		Serial.print(F("Updating Spotify Token: "));
		client.getToken(&auth, "refresh_token", auth.refreshToken);
		if (auth.refreshToken != "") {
			saveRefreshToken(auth.refreshToken);
			Serial.println(F("Updated"));
		}
	}

	if (millis() - lastFastUpdate > TIMER_FAST_COUNT) {
		lastFastUpdate = millis();

		Serial.print(F("Updating Spotify client: "));
		uint16_t responseCode = client.update(&data, &auth);
		Serial.printf("Updated %d. Playing now: %s\n", responseCode, data.title.c_str());
		if (responseCode == 401) {
			Serial.println(F("Refreshing token."));
			client.getToken(&auth, "refresh_token", auth.refreshToken);
			if (auth.refreshToken != "") {
				saveRefreshToken(auth.refreshToken);
			}
		}
		if (responseCode == 400) {
			Serial.println(F("Please define clientId and clientSecret"));
		}


		webserver.updateDHT();
		Serial.print(F("Humidity: "));
		Serial.print(webserver.getHumidity());
		Serial.print(" % ");
		Serial.print(F("Temperature: "));
		Serial.print(webserver.getTemperature());
		Serial.print(" *C ");
		Serial.print(F("Heat index: "));
		Serial.print(webserver.getHeatIndex());
		Serial.println(F(" *C "));

	}

	if (millis() - lastSecondUpdate > TIMER_SECOND_COUNT) {
		lastSecondUpdate = millis();
		webserver.addTime(1);
	}

	webserver.handleClient();
	ArduinoOTA.handle();

}


void saveRefreshToken(String refreshToken) {

	File f = SPIFFS.open("/refreshToken.txt", "w+");
	if (!f) {
		Serial.println(F("Failed to open config file"));
		return;
	}
	f.println(refreshToken);
	f.close();
}

String loadRefreshToken() {
	Serial.println(F("Loading config"));
	File f = SPIFFS.open("/refreshToken.txt", "r");
	if (!f) {
		Serial.println(F("Failed to open config file"));
		return "";
	}
	while (f.available()) {
		//Lets read line by line from the file
		String token = f.readStringUntil('\r');
		Serial.printf("Refresh Token: %s\n", token.c_str());
		f.close();
		return token;
	}
	return "";
}