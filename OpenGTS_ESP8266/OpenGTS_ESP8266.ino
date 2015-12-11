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
long lat, lon;
unsigned long fix_age, time_, date, speed, course;
float falt;

//Wifi
WiFiClient client;

//Some variable
int id = ESP.getChipId();

void setup() {
	Serial.begin(9600);
	WiFi.begin(ssid, password);
	pinMode(BUILTIN_LED, OUTPUT);
	while (WiFi.status() != WL_CONNECTED) {
		digitalWrite(BUILTIN_LED, HIGH);
		delay(500);
		digitalWrite(BUILTIN_LED, LOW);
		delay(500);
	}
}

// the loop function runs over and over again forever
void loop() {
	//encode gps data
	while (Serial.available()) {
		int c = Serial.read();
		if (gps.encode(c)) {
			// retrieves + / -lat / long in 100000ths of a degree
			gps.get_position(&lat, &lon, &fix_age);
			// time in hhmmsscc, date in ddmmyy
			gps.get_datetime(&date, &time_, &fix_age);
			// returns speed in 100ths of a knot
			speed = gps.speed() * 1.852;
			// course in 100ths of a degree
			course = gps.course();
			//altitude
			float falt = gps.f_altitude();
		}
	}

	//Send Data to Server if connected
	if (client.connect(server, 8080)) {
		digitalWrite(BUILTIN_LED, HIGH);
		client.print("GET /gprmc/Data?id="); //id
		client.print(id);
		client.print("&code=0xF030&date="); //date
		client.print(date);
		client.print("&time=");
		client.print(time_);
		client.print("&lat=");
		client.print(lat);
		client.print("&lon=");
		client.print(lon); 
		client.print("&alt=");
		client.print(falt);
		client.print("&head=");
		client.print(course);
		client.print("&speed=");
		client.print(speed);

		client.println(" HTTP/1.1");
		client.println("Host: 52.34.208.40");
		client.println();
	}
	client.stop();
	client.println("Connection: close");
	digitalWrite(BUILTIN_LED, LOW);

	delay(1);
}
