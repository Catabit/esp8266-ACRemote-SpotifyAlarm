#pragma once

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

//Between each HTMLxy chunk there is some dynamic content that requires run-time info. Everything here will be used from PROGMEM
//with F(HTMLxy)

#define HTML1 \
		"<html>" \
		"<head>" \
		"<title>ACRemote</title>" \
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
		"<style>table, th, td {border: 1px solid black;}table {border-collapse: collapse;}th, td {padding: 15px;text-align: center;}td{width: 40px;} p {font-size:18px;}</style>" \
		"</head>" \
		"<body>" \
		"<div style=\"overflow-x:auto;\">"\
		"<table>" \
		"<tr>" \
		"<td><p>Settings</p></td>" \
		"<td><p><a href=\"ac?do="  STR(OFF)  "\">OFF</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(SWING)  "\">Swing</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(DIRECT)  "\">Direct</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(SLEEP)  "\">Sleep</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(LED)  "\">LED</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(SILENT)  "\">SILENT</a></p></td>" \
		"</tr>" \
		"<tr>" \
		"<td><p>Modes</p></td>" \
		"<td><p><a href=\"ac?do="  STR(MAUTO)  "\">AUTO</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(MCOOL)  "\">COOL</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(MDRY ) "\">DRY</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(MFAN ) "\">FAN</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(MHEAT)  "\">HEAT</a></p></td>" \
		"</tr>" \
		"<tr>" \
		"<td><p>Fan</p></td>" \
		"<td><p><a href=\"ac?do="  STR(FANMIN ) "\">Min</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(FANMED ) "\">Med</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(FANMAX ) "\">Max</a></p></td>" \
		"<td><p><a href=\"ac?do="  STR(FANAUTO)  "\">Auto</a></p></td>" \
		"</tr>" \
		"<tr>" \
		"<td><p>Temperature</p></td>" \
		"<td><p><a href=\"ac?do="  STR(T16)  "\">"  STR(T16)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T17)  "\">"  STR(T17)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T18)  "\">"  STR(T18)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T19)  "\">"  STR(T19)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T20)  "\">"  STR(T20)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T21)  "\">"  STR(T21)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T22)  "\">"  STR(T22)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T23)  "\">"  STR(T23)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T24)  "\">"  STR(T24)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T25)  "\">"  STR(T25)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T26)  "\">"  STR(T26)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T27)  "\">"  STR(T27)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T28)  "\">"  STR(T28)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T29)  "\">"  STR(T29)  "</a></p>" \
		"<td><p><a href=\"ac?do="  STR(T30)  "\">"  STR(T30)  "</a></p>" \
		"</tr>" \
		"</table>" \
		"</div>" \
		"<br>" \
		"<div style=\"overflow-x:auto;\">" \
		"<h3>Spotify controls</h3>"



#define HTML2 \
		"<table>" \
		"<tr>" \
		"<td><p><a href=\"player?do="  STR(PREV)  "\">PREV</a></p>" 

#define HTML2a \
		"<td><p><a href=\"player?do="  STR(NEXT)  "\">NEXT</a></p>" 

#define HTML2b \
		"<td><p><a href=\"player?do="  STR(PLAY_ON_PC)  "\">PLAY ON PC</a></p>" \
		"<td><p><a href=\"player?do="  STR(PLAY_SAVED_PLAYLIST)  "\">PLAY SAVED PLAYLIST</a></p>" \
		"</tr>" \
		"</table>" \
		"</div>" \
		"<div style=\"overflow-x:auto;\">" \
		"<form action=\"/player\" method=\"get\"> Spotify URI for the saved playlist: <input type=\"text\" name=\"playlist\"> <input type=\"submit\" value=\"Submit\"></form>"



#define HTML3 \
		"</div>" \
		"<br>" \
		"<div style=\"overflow-x:auto;\">" \
		"<p><a href=\"wol\">Wake up PC</a></p>"\
		"</div>" \
		"</div>" \
		"<br>" \
		"<div style=\"overflow-x:auto;\">"



#define HTML4 \
		"<a href=\"alarm?dst=1\"><input type=\"button\" value=\"Toggle DST\"></a> " \
		"<p>Set an alarm: <form action=\"/alarm\" method=\"get\"><input type=\"time\" name=\"time\"> <input type=\"submit\" value=\"Submit\"></form></p>" \
		"<p>Set the alarm volume: <form action=\"/alarm\" method=\"get\"><input type=\"number\" name=\"volume\" min=\"0\" max=\"100\"> <input type=\"submit\" value=\"Submit\"></form></p>" \
		"<p><a href=\"alarm?disable=1\">Disable alarm</a></p>"


#define HTML5 \
		"</div>" \
		"<br>" \
		"<div style=\"overflow-x:auto;\">"



#define HTML6 \
		"</div>" \
		"</body>" \
		"</html>"