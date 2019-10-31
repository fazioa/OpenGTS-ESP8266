// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include "credenziali.h"

//OpenGTS server
const char* server = "40.112.128.183";

//Wifi
WiFiClient client;

//Some variable
int id = ESP.getChipId();
String gprmc = "";

void setup() {
	Serial.begin(9600);
	WiFi.begin(ssid, password);

 Serial.println("START");
	pinMode(BUILTIN_LED, OUTPUT);
	while (WiFi.status() != WL_CONNECTED) {
		digitalWrite(BUILTIN_LED, HIGH);
		delay(500);
		digitalWrite(BUILTIN_LED, LOW);
		delay(500);
	}
 Serial.println("WIFI CONNECTED");
}

// the loop function runs over and over again forever
void loop() {
	//encode gps data
	do {
		String read = Serial.readStringUntil('\n');
     Serial.println(read);
		if (read[3] == 'C' && read[4] == 'G' && read[5] == 'A')
			gprmc = read;
   
	} while (gprmc == "");

	//Send Data to Server if connected
	if (client.connect(server, 5159)) {
		digitalWrite(BUILTIN_LED, HIGH);
		client.print("GET /gprmc/Data?id="); //id
		client.print(id);
		client.print("&code=0xF030&gprmc="); //gprmc
		client.print(gprmc);

		client.println(" HTTP/1.1");
		client.println("Host: 52.34.208.40");
		client.println();
	}
	client.stop();
	client.println("Connection: close");
	digitalWrite(BUILTIN_LED, LOW);
	gprmc = "";
	delay(1);
}
