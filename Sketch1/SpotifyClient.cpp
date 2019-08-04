#include "SpotifyClient.h"

#define min(X, Y) (((X)<(Y))?(X):(Y))


void SpotifyClient::setSavedURI(String newURI)
{
	savedURI = newURI;
}

String SpotifyClient::getSavedURI()
{
	return savedURI;
}

SpotifyClient::SpotifyClient(String clientId, String clientSecret, String redirectUri) {
  this->clientId = clientId;
  this->clientSecret = clientSecret;
  this->redirectUri = redirectUri;
  shuffle = false;
  volume = 75;
  parser.setListener(this);
  parser.reset();
}

void executeCallback() {
	yield();
	//Serial.println(ESP.getFreeHeap());
}


uint16_t SpotifyClient::update(SpotifyData *data, SpotifyAuth *auth) {
  this->data = data;
  parser.reset();
  level = 0;
  isDataCall = true;
  currentParent = "";

  
  String url = F("/v1/me/player/currently-playing");
  if (!client.connect(host.c_str(), HTTPS_PORT)) {
    Serial.println("connection failed");
    return 0;
  }

  String request = makeRequest("GET", url, "");
  client.print(request);
  request.~String();

  uint16_t httpCode;
  httpCode = processClient(client, parser);


  if (httpCode == 200) {
    this->data->isPlayerActive = true;
  } else if (httpCode == 204) {
    this->data->isPlayerActive = false;
  }

  this->data = nullptr;
  lastHTTPcode = httpCode;
  return httpCode;
}


uint16_t SpotifyClient::playerCommand(SpotifyAuth *auth, String method, String command) {
	parser.reset();
  level = 0;
  isDataCall = true;
  currentParent = "";

  
  String url = "/v1/me/player/" + command;
  if (!client.connect(host.c_str(), HTTPS_PORT)) {
    Serial.println("connection failed");
    return 0;
  }

  String request = makeRequest(method, url, "");
  client.print(request);

  request.~String();

  uint16_t httpCode;
  httpCode = processClient(client, parser);

  lastHTTPcode = httpCode;

  return httpCode;
}

uint16_t SpotifyClient::getAvailableDevices(SpotifyAuth *auth) {
	parser.reset();
  lastDeviceID = "";
  level = 0;
  isDataCall = true;
  currentParent = "";
  


  
  String url = "/v1/me/player/devices";
  if (!client.connect(host.c_str(), HTTPS_PORT)) {
    Serial.println("connection failed");
    return 0;
  }
  String request = makeRequest("GET", url, "");

  client.print(request);
  request.~String();

  uint16_t httpCode;
  httpCode = processClient(client, parser);

  lastHTTPcode = httpCode;

  return httpCode;
}

uint16_t SpotifyClient::setShuffle(SpotifyAuth *auth, bool nShuffle) {
	parser.reset();
	level = 0;
	isDataCall = true;
	currentParent = "";

	
	String url = "/v1/me/player/shuffle";
	if (!client.connect(host.c_str(), HTTPS_PORT)) {
		Serial.println("connection failed");
		return 0;
	}


	url = url + String("?state=") + ((nShuffle == true) ? "true" : "false") + "&device_id=" + PC_id;
	String request = makeRequest("PUT", url, "");

client.print(request);
request.~String();

uint16_t httpCode;
httpCode = processClient(client, parser);

if (httpCode == 204 || httpCode==200) {
	shuffle = nShuffle;
	Serial.printf("Successfully set shuffle to %s\n", ((shuffle == true) ? "true" : "false"));
}
lastHTTPcode = httpCode;

return httpCode;
}

uint16_t SpotifyClient::toggleShuffle(SpotifyAuth *auth) {
	return setShuffle(auth, !shuffle);
}

uint16_t SpotifyClient::setVolume(SpotifyAuth *auth, int newVolume) {
	parser.reset();
  if (volume < 0 || volume > 100)
    return -1;
  level = 0;
  isDataCall = true;
  currentParent = "";

  
  String url = "/v1/me/player/volume";
  if (!client.connect(host.c_str(), HTTPS_PORT)) {
    Serial.println("connection failed");
    return 0;
  }


  url = url + String("?volume_percent=") + newVolume + "&device_id=" + PC_id;
  String request = makeRequest("PUT", url, "");
  client.print(request);
  request.~String();
  uint16_t httpCode;
  httpCode = processClient(client, parser);

  if (httpCode == 204 || httpCode == 200) {
    Serial.printf("Successfully set the volume to %d\n", newVolume);
    volume = newVolume;
  }
  lastHTTPcode = httpCode;

  return httpCode;
}

uint16_t SpotifyClient::setDevice(SpotifyAuth *auth, String newDevice) {
	parser.reset();
  if (newDevice == "")
    newDevice = PC_id;
  if (PC_id == "")
    return -1;

  level = 0;
  isDataCall = true;
  currentParent = "";

  
  String url = "/v1/me/player";
  if (!client.connect(host.c_str(), HTTPS_PORT)) {
    Serial.println("connection failed");
    return 0;
  }

  String content = String("{\"device_ids\":[\"") + newDevice + "\"]}" + "\r\n" + "play=true";
  String request = makeRequest("PUT", url, content);
  client.print(request);
  request.~String();

  uint16_t httpCode;
  httpCode = processClient(client, parser);

  if (httpCode == 204 || httpCode == 200) {
    Serial.printf("Successfully set the device to %s\n", newDevice.c_str());
  }
  lastHTTPcode = httpCode;

  return httpCode;
}

uint16_t SpotifyClient::playURI(SpotifyAuth *auth, String newURI) {
	parser.reset();
  level = 0;
  isDataCall = true;
  currentParent = "";

  if (newURI == "")
	  newURI = savedURI;

  
  String url = "/v1/me/player/play";
  if (!client.connect(host.c_str(), HTTPS_PORT)) {
    Serial.println("connection failed");
    return 0;
  }

  url = url + "?device_id=" + PC_id;
  String content = String("{\"context_uri\":\"") + newURI + "\"}";

  String request = makeRequest("PUT", url, content);
  client.print(request);
  
  request.~String();
  uint16_t httpCode;
  httpCode = processClient(client, parser);

  if (httpCode == 204 || httpCode == 200) {
    Serial.printf("Successfully set the playlist to %s\n", newURI.c_str());
  }
  lastHTTPcode = httpCode;

  return httpCode;
}

void SpotifyClient::getToken(SpotifyAuth *auth, String grantType, String code) {
	parser.reset();
  this->auth = auth;
  isDataCall = false;
  //https://accounts.spotify.com/api/token
  const char* host = "accounts.spotify.com";
  
  String url = "/api/token";
  if (!client.connect(host, HTTPS_PORT)) {
    Serial.println("connection failed");
    return;
  }

  String codeParam = "code";
  if (grantType == "refresh_token") {
    codeParam = "refresh_token";
	auth->refreshToken = code;
  }
  String authorizationRaw = clientId + ":" + clientSecret;
  String authorization = base64::encode(authorizationRaw, false);
  // This will send the request to the server
  String content = "grant_type=" + grantType + "&" + codeParam + "=" + code + "&redirect_uri=" + redirectUri;
  String request = String("POST ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Authorization: Basic " + authorization + "\r\n" +
                   "Content-Length: " + String(content.length()) + "\r\n" +
                   "Content-Type: application/x-www-form-urlencoded\r\n" +
                   "Connection: close\r\n\r\n" +
                   content;
  client.print(request);
  request.~String();

  int retryCounter = 0;
  while (!client.available()) {
	  yield();
    retryCounter++;
    if (retryCounter > CLIENT_RETRY_ATTEMPTS) {
      Serial.println("TIMED OUT");
      return;
    }
    delay(100);
  }

  boolean isBody = false;
  char c;

  int size = 0;
  client.setNoDelay(false);
  while (client.connected() || client.available()) {
    while ((size = client.available()) > 0) {
      c = client.read();
	  //Serial.print(c);
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
       
      }
    }
	yield();
  }

  this->data = nullptr;
}

String SpotifyClient::startConfigPortal() {
  String oneWayCode = "";
  server = new ESP8266WebServer();

  server->on ( "/", [this]() {
    Serial.println(this->clientId);
    Serial.println(this->redirectUri);
    server->sendHeader("Location", String("https://accounts.spotify.com/authorize/?client_id="
                                         + this->clientId
                                         + "&response_type=code&redirect_uri="
                                         + this->redirectUri
                                         + "&scope=user-read-private%20user-read-currently-playing%20user-read-playback-state%20user-modify-playback-state"), true);
    server->send ( 302, "text/plain", "");
  } );

  server->on ( "/callback/", [this, &oneWayCode]() {
    if (!server->hasArg("code")) {
      server->send(500, "text/plain", "BAD ARGS");
      return;
    }

    oneWayCode = server->arg("code");
    Serial.printf("Code: %s\n", oneWayCode.c_str());

    String message = "<html><head></head><body>Succesfully authentiated This device with Spotify. Restart your device now</body></html>";

    server->send ( 200, "text/html", message );
  } );

  server->begin();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
  } else {
    Serial.println("WiFi not connected!");
  }

  Serial.println ( "HTTP server started" );

  while (oneWayCode == "") {
    server->handleClient();
    yield();
  }
  server->stop();
  server->~ESP8266WebServer();
  return oneWayCode;
}

void SpotifyClient::whitespace(char c) {
}

void SpotifyClient::startDocument() {
  level = 0;
}

void SpotifyClient::key(String key) {
  currentKey = String(key);
  rootPath[level] = key;
}

String SpotifyClient::getRootPath() {
  String path = "";
  for (uint8_t i = 1; i <= level; i++) {
    String currentLevel = rootPath[i];
    if (currentLevel == "") {
      break;
    }
    if (i > 1) {
      path += ".";
    }
    path += currentLevel;
  }
  return path;
}

void SpotifyClient::value(String value) {
  if (isDataCall) {

    String rootPath = this->getRootPath();

    if (rootPath == "devices.id") {
      lastDeviceID = value;
    }

    if (rootPath == "devices.name") {
      if (value == PC_NAME)
        PC_id = lastDeviceID;
      Serial.print(F("Set the PC device id to: "));
      Serial.println(PC_id);

    }


	
    if (rootPath == "progress_ms") {
      data->progressMs = value.toInt();
    }
    if (rootPath == "item.duration_ms") {
      data->durationMs = value.toInt();
    }
    if (rootPath == "item.name") {
      data->title = value;
    }
    if (rootPath == "is_playing") {
      data->isPlaying = (value == "true" ? true : false);
    }
    if (currentKey == "height") {
      currentImageHeight = value.toInt();
    }
    if (currentKey == "url") {
   
      if (rootPath == "item.album.images.url") {
        if (currentImageHeight == 640) {
          data->image640Href = value;
        }
        if (currentImageHeight > 250 && currentImageHeight < 350) {
          data->image300Href = value;
        }
        if (currentImageHeight == 64) {
          data->image64Href = value;
        }
      }
    }
    if (rootPath == "item.album.artists.name") {
      data->artistName = value;
    }
	
	if (currentParent == "error") {
		if (currentKey == "status")
		{
			Serial.print(F("Got json status error "));
			Serial.print(value.toInt());
		}
		if (currentKey == "message") {
			Serial.print(F(" with message "));
			Serial.println(value);
			lastErrorMessage = value;
		}
	}

  } else {
    if (currentKey == "access_token") {
      auth->accessToken = value;
    }
    if (currentKey == "token_type") {
      auth->tokenType = value;
    }
    if (currentKey == "expires_in") {
      auth->expiresIn = value.toInt();
    }
    if (currentKey == "refresh_token") {
		Serial.println(F("Refresh token modified"));
		auth->refreshToken = value;
    }
    if (currentKey == "scope") {
      auth->scope = value;
    }
  }
}

void SpotifyClient::endArray() {

}


void SpotifyClient::startObject() {
  currentParent = currentKey;
  //rootPath[level] = currentKey;
  level++;

}

void SpotifyClient::endObject() {
  //rootPath[level] = "";
  level--;
  currentParent = "";
}

void SpotifyClient::endDocument() {

}

void SpotifyClient::startArray() {

}


int SpotifyClient::processClient(WiFiClientSecure client, JsonStreamingParser parser) {
	lastErrorMessage = "";
	int retryCounter = 0;
	while (!client.available()) {
		executeCallback();

		retryCounter++;
		if (retryCounter > CLIENT_RETRY_ATTEMPTS) {
			Serial.println(F("Processing timeout."));
			return -1;
		}
		delay(10);
	}
	unsigned char* buf = (unsigned char*)malloc(BUFLEN * sizeof(unsigned char));
	boolean isBody = false;
	char c = ' ';

	int size = 0;
	client.setNoDelay(false);
	uint16_t httpCode = 0;

	while (client.connected() || client.available()) {
		while ((size = client.available()) > 0) {
			if (isBody) {
				uint16_t len = min(BUFLEN, size);
				c = client.readBytes(buf, len);
				
				for (uint16_t i = 0; i < len; i++) {
					parser.parse(buf[i]);
					executeCallback();
				}
			}
			else {
				String line = client.readStringUntil('\r');
				if (line.startsWith("HTTP/1.")) {
					httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
				}
				if (line == "\r" || line == "\n" || line == "") {
					isBody = true;
				}
			}

		}
		executeCallback();
	}
	free(buf);
	
	return httpCode;
}

String SpotifyClient::makeRequest(String method, String url, String content) {
	String request = method + " " + url + F(" HTTP/1.1\r\n") +
		F("Host: ") + host + F("\r\n") +
		F("Authorization: Bearer ") + auth->accessToken + F("\r\n") +
		F("Content-Length: ") + String(content.length()) + F("\r\n") +
		//F("Content-Type: application/json\r\n") +
		//F("Accept: application/json\r\n") +
		F("Connection: close\r\n\r\n") +
		content;
	//Serial.println(request);
	return request;
}
