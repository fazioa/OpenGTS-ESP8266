// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include "credenziali.h"
#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module


#define TRACCAR_HOST "40.112.128.183"
#define TRACCAR_PORT 5055
//#define TRACCAR_DEV_ID "YOUR_DEVICE_ID"

static const int RXPin = 0, TXPin = 2;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600


char isotime[24];
char lastisotime[24];
char data[128];


//OpenGTS server
const char* server = "40.112.128.183";

//Wifi
WiFiClient client;

//Some variable
int TRACCAR_DEV_ID = ESP.getChipId();
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device
TinyGPSPlus  gps;                                        // Create an Instance of the TinyGPS++ object called gps

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

  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600
}



// the loop function runs over and over again forever
void loop() {
  static unsigned long lastutc = 0;

  smartDelay(3000);                                      // Run Procedure smartDelay


// check WiFi connection and connect to Traccar server if disconnected
    if (!client.connected()) {
      Serial.print("Connecting to ");
      Serial.print(TRACCAR_HOST);
      Serial.print("...");
      if (client.connect(TRACCAR_HOST, TRACCAR_PORT)) {
        Serial.println("OK");
      } else {
        Serial.println("failed");
        delay(3000);
        return;
      }
    }


// generate ISO time string
  sprintf(isotime, "%04u-%02u-%02uT%02u:%02u:%02u.%01uZ",
          gps.date.year(), gps.date.month(), gps.date.day(),
          gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());

//controllo che il tempo del GPS non sia identico al ciclo precedente
  if( strcmp(isotime,lastisotime) == 0 ) {
//   check UTC timestamp from GPS
      return;
    }
    strcpy(lastisotime,isotime);

  // turn on  indicator LED
  digitalWrite(BUILTIN_LED, HIGH);
printInfoGPS(gps);

 
  // send data
    client.print(OsmAndProtocol(gps));

  // output server response
  while (client.available()) {
    Serial.print((char)client.read());
  }



  //  // turn off indicator LED
  digitalWrite(BUILTIN_LED, LOW);
}

static void smartDelay(unsigned long ms)                // This custom version of delay() ensures that the gps object is being "fed".
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


static void printGPSDateTime()
{
  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }
}

void printInfoGPS(TinyGPSPlus gps){
   // now that new GPS coordinates are available
  double lat, lng;
  lat = gps.location.lat();
  lng = gps.location.lng();
  double speed = gps.speed.kmph();
  double alt = gps.altitude.meters();
  int sats = gps.satellites.value();
  double heading = gps.course.deg();

  Serial.print("Latitude  : ");
  Serial.println(lat, 6);
  Serial.print("Longitude : ");
  Serial.println(lng, 6);
  Serial.print("Satellites: ");
  Serial.println(sats);
  Serial.print("Elevation : ");
  Serial.print(alt);
  Serial.println("m");
  printGPSDateTime();
  Serial.println("");

  Serial.print("Heading   : ");
  Serial.println(gps.course.deg());
  Serial.print("Speed     : ");
  Serial.println(gps.speed.mph());
  Serial.print("isotime: ");
  Serial.println(isotime);

}


String OsmAndProtocol(TinyGPSPlus gps){
  // now that new GPS coordinates are available
  double lat, lng;
  lat = gps.location.lat();
  lng = gps.location.lng();
  double speed = gps.speed.kmph();
  double alt = gps.altitude.meters();
  int sats = gps.satellites.value();
  double heading = gps.course.deg();

  
  // arrange and send data in OsmAnd protocol
  // refer to https://www.traccar.org/osmand
    String data = "&lat=" + String(lat, 6) + "&lon=" + String(lng, 6)   + "&altitude=" + String(alt, 1) + "&speed=" + String(speed, 1) + "&heading=" + String(heading, 1);
String URL= "GET /?id=" + String(TRACCAR_DEV_ID) + "&timestamp=" + isotime + data + " HTTP/1.1\r\n" +
               "Host: " + TRACCAR_HOST + "\r\n" +
               "Connection: keep-alive\r\n\r\n";
  
  return URL;
  }
