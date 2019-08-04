#include "WebServer.h"

WebServer::WebServer()
{
	esp8266webserver = new ESP8266WebServer(80);
	
	//Set this IP to the computer you wake up with WOL
	computer_ip = IPAddress(10, 20, 30, 2);
	computer_ip_broadcast = IPAddress(10, 20, 30, 255);
	dht.setup(DHTPIN, DHTesp::DHT22);

}


WebServer::~WebServer()
{
}

void WebServer::handleClient()
{
	esp8266webserver->handleClient();
}


void WebServer::wakePC() {
	WakeOnLan::sendWOL(computer_ip, UDP, mac, sizeof mac);
	WakeOnLan::sendWOL(computer_ip_broadcast, UDP, mac, sizeof mac);
}

void WebServer::sendNTPpacket(IPAddress& address)
{
	Serial.println(F("sending NTP packet..."));
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	UDP.beginPacket(address, 123); //NTP requests are to port 123
	UDP.write(packetBuffer, NTP_PACKET_SIZE);
	UDP.endPacket();
}

void WebServer::triggerAlarm() {
	Serial.println(F("Alarm triggered"));
	
	disableAlarm();

	uint32_t beginWait = millis();
	bool ledState = 0;
	while (millis() - beginWait < WAKE_TIMEOUT) {
		digitalWrite(LED_BUILTIN, ledState);
		ledState = !ledState;
		wakePC();
		_client->PC_id = "";
		_client->getAvailableDevices(_auth);
		if (_client->PC_id != "") {// the PC is available on Spotify
			Serial.println(F("PC found"));
			delay(3000); // wait for it to be fully loaded
			_client->setDevice(_auth, "");
			delay(500);
			_client->setVolume(_auth, saved_settings.alarmVolume); //set the volume to default
			delay(500);
			_client->setShuffle(_auth, true); //set shuffle on
			delay(500);
			_client->playURI(_auth); //start playing the default playlist
			break;
		}
		else {
			delay(5000);
			Serial.println(F("PC not available"));
		}
	}
	digitalWrite(LED_BUILTIN, HIGH);
}

unsigned long WebServer::updateTime() {
	sendNTPpacket(timeServerIP); // send an NTP packet to a time server
	yield();

	uint32_t beginWait = millis();
	while (UDP.parsePacket() > 0); // discard any previously received packets

	while (millis() - beginWait < NTP_TIMEOUT) {
		int cb = UDP.parsePacket();
		if (!cb) {
			Serial.println(F("no packet yet"));
		}
		else {
			Serial.print(F("packet received, length="));
			Serial.println(cb);
			// We've received a packet, read the data from it
			UDP.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

			//the timestamp starts at byte 40 of the received packet and is four bytes,
			// or two words, long. First, esxtract the two words:

			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			// combine the four bytes (two words) into a long integer
			// this is NTP time (seconds since Jan 1 1900):
			unsigned long secsSince1900 = highWord << 16 | lowWord;

			// now convert NTP time into everyday time:		
			// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
			const unsigned long seventyYears = 2208988800UL;
			// subtract seventy years:
			unsigned long epoch = secsSince1900 - seventyYears + TIMEZONE * 3600;

			if (isDST)
				epoch += 3600;


			hour = (epoch % 86400L) / 3600;
			minute = (epoch % 3600) / 60;
			second = epoch % 60;

			checkForAlarm();
			
			Serial.print(F("Current time is "));
			Serial.printf("%d:%d:%d.\n", hour, minute, second);

			timeUTC = epoch;
			return epoch;
		}
		delay(20);
	}
	return 0;
}

void WebServer::updateDHT()
{
	humidity = dht.getHumidity();
	// Read temperature as Celsius (the default)
	temperature = dht.getTemperature();
	// Compute heat index in Celsius (isFahreheit = false)
	heatIndex = dht.computeHeatIndex(temperature, humidity, false);
}

void WebServer::addTime(long seconds)
{
	 timeUTC += seconds;
	 hour = (timeUTC % 86400L) / 3600;
	 minute = (timeUTC % 3600) / 60;
	 second = timeUTC % 60;
	 checkForAlarm();
}

void WebServer::checkForAlarm() {
	if (alarmSet == true && alarmTriggered == false) {
		if (timeUTC > alarmUTC)
			triggerAlarm();
	}
}

bool WebServer::getDST()
{
	return isDST;
}

void WebServer::disableAlarm()
{
	if (alarmSet) {
		alarmSet = false;
		alarmTriggered = false;
		alarmUTC = 0;

		saved_settings.alarmUTC = 0;
		EEPROM.put(0, saved_settings);
		EEPROM.commit();

		Serial.println(F("Disabled alarm"));
	}
}

void WebServer::setDST(bool state)
{
	isDST = state;
}

void WebServer::redirectToHome() {
	esp8266webserver->sendHeader("Location", String("/"), true);
	esp8266webserver->send(302, "text/plain", "");
	yield();
}

//the HTML page is split between a few strings in mainhtml.h and stored in PROGMEM with F()
void WebServer::handleRoot() {
	esp8266webserver->send(200, "text/html",
		String("") + \
		F(HTML1) \
		+ F("<h3>Now playing  ") + _data->artistName + " - " + _data->title + "</h3>" +\
		F(HTML2) \
		+ F("<td style=\"background-color: ") + (_data->isPlaying == true ? "#CCCCCC" : "#FFFFFF") + F("\"><p><a href=\"player?do="  STR(PLAY)  "\">PLAY/PAUSE</a></p>") +\
		F(HTML2a) \
		+ F("<td style=\"background-color: ") + (_client->shuffle==true?"#CCCCCC":"#FFFFFF") +F("\"><p><a href=\"player?do="  STR(SHUFFLE)  "\">SHUFFLE</a></p>") +\
		F(HTML2b) \
		+ F("<p>Last HTTP return code from the player was ") + _client->lastHTTPcode + (_client->lastErrorMessage!=""?" with the message \""+_client->lastErrorMessage+"\"":"") +"</p>" +\
		F(HTML3) \
		+ F("<p>Current time is:") + hour + ":" + minute + ":" + second + F(". DST is currently ") + (isDST == true ? "active" : "inactive") + "</p>"  +\
		F(HTML4) \
		+ (alarmTriggered == true ? String("") + F("<p>The alarm was triggered</p>") : "")  \
		+ (alarmSet == true ? String("") + F("<p>There is an alarm set at ") + ((alarmUTC % 86400L) / 3600) + ":" + ((alarmUTC % 3600) / 60) + " with volume " + saved_settings.alarmVolume + "</p>" : "") + \
		F(HTML5)
		+ F("<h3> Temperature: ") + temperature + F("*C Humidity: ") + humidity + F("% Heat Index: ") + heatIndex + F("*C</h3>")  +\
		F(HTML6)

		);
}

void WebServer::handleIr() {
	redirectToHome();
	for (uint8_t i = 0; i < esp8266webserver->args(); i++) {
		if (esp8266webserver->argName(i) == "do") {
			uint32_t action = strtoul(esp8266webserver->arg(i).c_str(), NULL, 10);
			ir_remote.sendCommand(action);
		}
	}
}

void WebServer::handlePlayer() {
	redirectToHome();
	for (uint8_t i = 0; i < esp8266webserver->args(); i++) {
		if (esp8266webserver->argName(i) == "do") {
			uint32_t action = strtoul(esp8266webserver->arg(i).c_str(), NULL, 10);
			String command = "";
			String method = "";
			switch (action) {
			case PLAY:
				method = "PUT";
				command = "play";
				if (_data->isPlaying) {
					command = "pause";
				}
				_data->isPlaying = !_data->isPlaying;
				break;
			case NEXT:
				method = "POST";
				command = "next";
				break;
			case PREV:
				method = "POST";
				command = "previous";
				break;
			case SHUFFLE:
				_client->toggleShuffle(_auth);
				break;
			case PLAY_ON_PC:
				_client->setDevice(_auth, ""); // "" defaults to the main PC
				break;
			case PLAY_SAVED_PLAYLIST:
				_client->playURI(_auth);
				break;
			case TEST_ALARM:
				triggerAlarm();
				break;
			}
			if (command != "" && method != "") {
				uint16_t responseCode = _client->playerCommand(_auth, method, command);
				Serial.print(F("playerCommand response ="));
				Serial.println(responseCode);
			}
		}
		if (esp8266webserver->argName(i) == "volume") {
			uint32_t action = strtoul(esp8266webserver->arg(i).c_str(), NULL, 10);
			_client->setVolume(_auth, action);
		}
		if (esp8266webserver->argName(i) == "playlist") {
			if (esp8266webserver->arg(i).length() < 100) {
				_client->setSavedURI(esp8266webserver->arg(i));

				memset(saved_settings.savedURI, 0, 100);
				memcpy(saved_settings.savedURI, _client->getSavedURI().c_str(), _client->getSavedURI().length());
				EEPROM.put(0, saved_settings);
				EEPROM.commit();
			}
			else {
				Serial.println(F("The URI is too long"));
			}
		}
	}
	_client->update(_data, _auth);
}

void WebServer::handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += esp8266webserver->uri();
	message += "\nMethod: ";
	message += (esp8266webserver->method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += esp8266webserver->args();
	message += "\n";
	for (uint8_t i = 0; i < esp8266webserver->args(); i++)
		message += " " + esp8266webserver->argName(i) + ": " + esp8266webserver->arg(i) + "\n";
	esp8266webserver->send(404, "text/plain", message);
}

void WebServer::handleWOL() {
	redirectToHome();
	wakePC();
}

void WebServer::handleAlarm() {
	redirectToHome();
	if (esp8266webserver->args() == 1 && esp8266webserver->argName(0) == "time") {
		char h[3], m[3];
		memcpy(h, esp8266webserver->arg(0).c_str(), 2);
		h[2] = 0;
		memcpy(m, esp8266webserver->arg(0).c_str()+3, 2);
		m[2] = 0;
		
		uint8_t alarmHour = strtoul(h, NULL, 10);
		uint8_t alarmMinute = strtoul(m, NULL, 10);
		uint8_t hNow, mNow;
		Serial.printf("Got data %d:%d\n", alarmHour, alarmMinute);
		long unsigned UTCNow = updateTime();
		hNow = (UTCNow % 86400L) / 3600L;
		mNow = (UTCNow % 3600L) / 60L;
		if (UTCNow == 0) {
			Serial.println(F("Could not get time!"));
			redirectToHome();
			return;
		}
		if (alarmMinute < mNow) {
			alarmHour--;
		}
		long unsigned timediff = (mod(alarmHour - hNow, 24) * 3600L) + (mod(alarmMinute - mNow, 60) * 60L);
		alarmUTC = UTCNow + timediff;
		Serial.printf("Set an alarm in %d hours and %d minutes from now! (At %d:%d)\n", (timediff % 86400L) / 3600L, (timediff % 3600L) / 60L, (alarmUTC % 86400L) / 3600L, (alarmUTC % 3600L) / 60L);
		alarmSet = true;

		saved_settings.alarmUTC = alarmUTC;
		EEPROM.put(0, saved_settings);
		EEPROM.commit();
	}
	else if (esp8266webserver->args() == 1 && esp8266webserver->argName(0) == "disable") {
		disableAlarm();
	}
	else if (esp8266webserver->args() == 1 && esp8266webserver->argName(0) == "dst") {
		isDST = !isDST;
		Serial.print("DST is now: ");
		Serial.println(isDST);

		saved_settings.DSTactive = isDST;
		EEPROM.put(0, saved_settings);
		EEPROM.commit();
		
	}
	else if (esp8266webserver->args() == 1 && esp8266webserver->argName(0) == "volume") {
		saved_settings.alarmVolume = strtoul(esp8266webserver->arg(0).c_str(), NULL, 10);
		EEPROM.put(0, saved_settings);
		EEPROM.commit();
	}
}

void WebServer::WebServer_init(SpotifyData *data, SpotifyAuth *auth, SpotifyClient *client) {
	_data = data;
	_auth = auth;
	_client = client;

	EEPROM.begin(EEPROM_SETTINGS_SIZE);
	EEPROM.get(0, saved_settings);
	if (saved_settings.version != EEPROM_SETTINGS_VERSION) {
		Serial.println(F("EEPROM data is from a different version. Clearing EEPROM"));
		for (int i = 0; i < EEPROM_SETTINGS_SIZE; i++) {
			EEPROM.write(i, 0);
		}
		EEPROM.commit();
		memset(&saved_settings, 0, sizeof(EEPROM_Settings)); //make sure it's not full of random data

		saved_settings.DSTactive = getDST();
		memset(saved_settings.savedURI, 0, 100);
		memcpy(saved_settings.savedURI, F(DEFAULT_SPOTIFY_URI), sizeof DEFAULT_SPOTIFY_URI);
		saved_settings.version = EEPROM_SETTINGS_VERSION;
		saved_settings.alarmUTC = 0;
		saved_settings.alarmVolume = DEFAULT_ALARM_VOLUME;

		EEPROM.put(0, saved_settings);
		EEPROM.commit();
	}
	else {
		Serial.println(F("Loading EEPROM settings:"));
		setDST(saved_settings.DSTactive);
		_client->setSavedURI(saved_settings.savedURI);
		if (saved_settings.alarmUTC > 0) {
			alarmUTC = saved_settings.alarmUTC;
			alarmSet = true;
		}


		Serial.print(F("DST: "));
		Serial.println(saved_settings.DSTactive);
		Serial.print(F("Saved URI: "));
		Serial.println(saved_settings.savedURI);
		Serial.print(F("alarmUTC: "));
		Serial.println(saved_settings.alarmUTC);
		Serial.print(F("alarm volume: "));
		Serial.println(saved_settings.alarmVolume);
		
	}

	UDP.begin(localPort);

	esp8266webserver->on("/", std::bind(&WebServer::handleRoot, this));
	esp8266webserver->on("/ac", std::bind(&WebServer::handleIr, this));
	esp8266webserver->on("/player", std::bind(&WebServer::handlePlayer, this));
	esp8266webserver->on("/wol", std::bind(&WebServer::handleWOL, this));
	esp8266webserver->on("/alarm", std::bind(&WebServer::handleAlarm, this));

	esp8266webserver->onNotFound(std::bind(&WebServer::handleNotFound, this));

	esp8266webserver->begin();

	WiFi.hostByName(ntpServerName, timeServerIP);
	Serial.print(F("Found NTP server "));
	Serial.println(timeServerIP);

	Serial.println(F("HTTP server started"));
}

float WebServer::getTemperature()
{
	return temperature;
}

float WebServer::getHumidity()
{
	return humidity;
}

float WebServer::getHeatIndex()
{
	return heatIndex;
}
