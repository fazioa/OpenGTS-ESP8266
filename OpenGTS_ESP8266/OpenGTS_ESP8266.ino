// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include <TinyGPS.h>


//SSID
const char* ssid = "Fadh";
const char* password = "12345678";

//OpenGTS server
const char* server = "52.34.208.40";

//TInyGPS
TinyGPS gps;
float flat, flon, fc, fkmph, falt;
int year;
byte month, day, hour, minute, second, hundredths;
unsigned long fix_age;

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
	//encode gps data
	while (Serial.available()) {
		int c = Serial.read();
		if (gps.encode(c)) {
			gps.f_get_position(&flat, &flon, &fix_age); //get position
			falt = gps.f_altitude(); // +/- altitude in meters
			fc = gps.f_course(); // course in degrees
			fkmph = gps.f_speed_kmph(); // speed in km/hr
			gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);
		}
	}

	//Send Data to Server if connected
	if (client.connect(server, 8080)) {
		digitalWrite(BUILTIN_LED, HIGH);
		client.print("GET /gprmc/Data?id="); //id
		client.print(id);
		client.print("&code=0xF030&date="); //date
		client.print(year);
		client.print(month);
		client.print(day);
		client.print("&time=");
		client.print(hour);
		client.print(minute);
		client.print(second);
		client.print("&lat=");
		client.print(flat);
		client.print("&lon=");
		client.print(flon); 
		client.print("&alt=");
		client.print(falt);
		client.print("&head=");
		client.print(fc);
		client.print("&speed=");
		client.print(fkmph);

		client.println(" HTTP/1.1");
		client.println("Host: 52.34.208.40");
		client.println();
	}
	client.stop();
	client.println("Connection: close");
	digitalWrite(BUILTIN_LED, LOW);

	delay(1);
}
