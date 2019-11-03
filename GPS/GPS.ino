// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include "credenziali.h"

//Creare un file di testo con il seguente contenuto
//SSID
//const char* ssid = "*****";
//const char* password = "*****";

#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module


#define TRACCAR_HOST "40.112.128.183"
#define TRACCAR_PORT 5055
//#define TRACCAR_DEV_ID "YOUR_DEVICE_ID"

static const int RXPin = 0, TXPin = 2;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600


static unsigned long timeToSendDataToServer = 20000; //fa frequenza di trasmissione viene rimodulata ad ogni cilo in base alla veloctà rilevata
static unsigned long FREQMIN=7200000;
static unsigned long FREQMED=20000;
static unsigned long FREQMAX=5000;

static const int minDegToSendDataToServer = 30;


char isotime[24];
char lastisotime[24];
char data[128];
unsigned long lastSend = 0;
int lastDegree=0, degree = 0;
float speed=0;


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
    Serial.print(".");
  }
  Serial.println("WIFI CONNECTED");

  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600
  Serial.println("Serial Connection to GPS ok");
  digitalWrite(BUILTIN_LED, LOW);
  lastSend = millis();
}



// the loop function runs over and over again forever
void loop() {


  smartDelay(1000);                                      // Run Procedure smartDelay

degree=gps.course.deg();
speed = gps.speed.kmph();




  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

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
  if ( strcmp(isotime, lastisotime) == 0 ) {
    //   check UTC timestamp from GPS
    return;
  }
  strcpy(lastisotime, isotime);

  printInfoGPS(gps);

//invia i dati al server ogni X secondi
  if ((millis() - lastSend > timeToSendDataToServer) || (abs(lastDegree- degree)>= minDegToSendDataToServer)) {
    // turn on  indicator LED
    digitalWrite(BUILTIN_LED, HIGH);
    Serial.println("\n\nSEND DATA\n\n");
    // send data
    client.print(OsmAndProtocol(gps));

    // output server response
    while (client.available()) {
      Serial.print((char)client.read());
    }

    //  // turn off indicator LED
    digitalWrite(BUILTIN_LED, LOW);

    //memorizza alcuni parametri per valutare il prossimo invio dati al server
    lastSend = millis();
    lastDegree=gps.course.deg();
  }



//se la velocità è bassa aumento la frequenza di invio, altrimenti la diminuisco
//se la velocità è bassissima allora diminuisco l'invio

if(speed<3){
  timeToSendDataToServer=FREQMIN;
} else if (speed>=3 && speed <40) {
  timeToSendDataToServer=FREQMED;
} else if (speed >=40) {
  timeToSendDataToServer=FREQMAX;
}
   Serial.print("Set frequency data send to ");
   Serial.println(timeToSendDataToServer);


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


void printInfoGPS(TinyGPSPlus gps) {
  // now that new GPS coordinates are available
  double lat=0, lng=0;
  
  if (gps.location.isUpdated() && gps.location.isValid()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
  }
  float speed = gps.speed.kmph();
  double alt = gps.altitude.meters();
  int sats = gps.satellites.value();
  double heading = gps.course.deg();

  Serial.println();
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
  Serial.println(heading);
  Serial.print("Speed     : ");
  Serial.println(speed);
  Serial.print("isotime: ");
  Serial.println(isotime);

}


String OsmAndProtocol(TinyGPSPlus gps) {
  // now that new GPS coordinates are available
  double lat, lng, alt, speed;
String URL="";
  if (gps.location.isUpdated() && gps.location.isValid()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
    speed = gps.speed.kmph();
    alt = gps.altitude.meters();
  }
  int sats = gps.satellites.value();
  double heading = gps.course.deg();

  if (gps.date.isUpdated() && gps.date.isValid() )
  {
    // generate ISO time string
    sprintf(isotime, "%04u-%02u-%02uT%02u:%02u:%02u.%01uZ",
            gps.date.year(), gps.date.month(), gps.date.day(),
            gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
  }

  if (lat != 0 && lng != 0) {
  // arrange and send data in OsmAnd protocol
  // refer to https://www.traccar.org/osmand
  String data = "&lat=" + String(lat, 6) + "&lon=" + String(lng, 6)   + "&altitude=" + String(alt, 1) + "&speed=" + String(speed, 1) + "&heading=" + String(heading, 1);
  URL = "GET /?id=" + String(TRACCAR_DEV_ID) + "&timestamp=" + isotime + data + " HTTP/1.1\r\n" +
               "Host: " + TRACCAR_HOST + "\r\n" +
               "Connection: keep-alive\r\n\r\n";
  }
  return URL;
}
