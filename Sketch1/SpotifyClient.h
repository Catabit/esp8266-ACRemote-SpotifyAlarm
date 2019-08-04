#pragma once
#include <Arduino.h>
#include <WiFiClientSecureBearSSL.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <ESP8266WebServer.h>
#include <base64.h>
#include "settings.h"

#define BUFLEN 64
#define PC_NAME "DESKTOP-CG0PF75"

#define CLIENT_RETRY_ATTEMPTS 100
#define HTTPS_PORT 443


typedef struct SpotifyAuth {
  String accessToken;
  String tokenType;
  uint16_t expiresIn;
  String refreshToken;
  String scope;

} SpotifyAuth;

typedef struct SpotifyData {
  uint32_t progressMs;
  boolean isPlaying;
  boolean isPlayerActive;
  
  String artistName;
  
  String image640Href;
  String image300Href;
  String image64Href;
  
  uint32_t durationMs;
  String title;
  
} SpotifyData;

class SpotifyClient: public JsonListener {

	private:
	JsonStreamingParser parser;
	
	WiFiClientSecure client;
	String currentKey;
	String currentParent;
	SpotifyData *data;
	SpotifyAuth *auth;
	bool isDataCall;
	String rootPath[10];
	uint8_t level = 0;
	uint16_t currentImageHeight;
	String clientId;
	String clientSecret;
	String redirectUri;
	String lastDeviceID;

	String host = "api.spotify.com";
  
	ESP8266WebServer *server;

	String getRootPath();

	String savedURI="";

	String makeRequest(String method, String url, String content);

	public:
	String PC_id;
	bool shuffle;
	int volume;
	uint32_t lastHTTPcode = 0;

	String lastErrorMessage = "";
 
	void setSavedURI(String newURI);
	String getSavedURI();
  
	SpotifyClient(String clientId, String clientSecret, String redirectUri);
	uint16_t update(SpotifyData *data, SpotifyAuth *auth);

	uint16_t playerCommand(SpotifyAuth *auth, String method, String command);

	uint16_t getAvailableDevices(SpotifyAuth *auth);
	uint16_t toggleShuffle(SpotifyAuth *auth);
	uint16_t setShuffle(SpotifyAuth *auth, bool shuffle);
	uint16_t setVolume(SpotifyAuth *auth, int newVolume=75);
	uint16_t setDevice(SpotifyAuth *auth, String newDevice="");
	uint16_t playURI(SpotifyAuth *auth, String newURI=""); 

	void getToken(SpotifyAuth *auth, String grantType, String code);

	String startConfigPortal();

	int processClient(WiFiClientSecure client, JsonStreamingParser parser);

	virtual void whitespace(char c);
	virtual void startDocument();
	virtual void key(String key);
	virtual void value(String value);
	virtual void endArray();
	virtual void endObject();
	virtual void endDocument();
	virtual void startArray();
	virtual void startObject();
};
