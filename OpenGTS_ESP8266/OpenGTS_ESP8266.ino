// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>


//SSID
const char* ssid = "Fadh";
const char* password = "12345678";

//OpenGTS server
const char* server = "52.34.208.40";

//TInyGPS
TinyGPSPlus gps;
//Wifi
WiFiClient client;

//Some variable
int id = ESP.getChipId();

void setup() {
	Serial.begin(9600);
	WiFi.begin(ssid, password);
	WiFi.begin(ssid, password);
	pinMode(BUILTIN_LED, OUTPUT);
	while (WiFi.status() != WL_CONNECTED) {
		digitalWrite(BUILTIN_LED, HIGH);
		delay(50);
		digitalWrite(BUILTIN_LED, LOW);
		delay(50);
	}
}

// the loop function runs over and over again forever
void loop() {
	while (Serial.available() > 0)
		gps.encode(Serial.read());

	//Send Data to Server if connected
	if (client.connect(server, 8080)) {
		digitalWrite(BUILTIN_LED, HIGH);
		client.print("GET /gprmc/Data?id="); //id
		client.print(id);
		client.print("&code=0xF030&date="); //date
		client.print(gps.date.year());
		client.print(gps.date.month());
		client.print(gps.date.day());
		client.print("&time=");
		client.print(gps.time.value());
		client.print("&lat=");
		client.print(gps.location.lat());
		client.print("&lon=");
		client.print(gps.location.lng()); 
		client.print("&alt=");
		client.print(gps.altitude.meters());
		client.print("&head=");
		client.print(gps.course.deg());
		client.print("&speed=");
		client.print(gps.speed.kmph());

		client.println(" HTTP/1.1");
		client.println("Host: 52.34.208.40");
		client.println();
	}
	client.stop();
	client.println("Connection: close");
	digitalWrite(BUILTIN_LED, LOW);

	delay(1);
}
