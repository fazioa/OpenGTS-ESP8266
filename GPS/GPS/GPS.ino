// the setup function runs once when you press reset or power the board
#include "parameter.h"
#include "network.h"
#include "traccar.h"


//Creare un file di testo con il seguente contenuto
//SSID
//const char* WiFi_SSID = "*****";
//const char* WiFi_Password = "*****";

#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module


static const int RXPin = 4, TXPin = 5;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600


static unsigned long timeToSendDataToServer; //fa frequenza di trasmissione viene rimodulata ad ogni cilo in base alla veloctà rilevata

unsigned long lastSend = 0;
int lastDegree = 0, degree = 0;
float speed = 0;
char lastisotime[24];
char isotime[24];
TinyGPSPlus gps;                                        // Create an Instance of the TinyGPS++ object called gps



SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device

void setup() {
  Serial.begin(9600);
  Serial.println(F("START"));

  //pausa 5 secondi
  delay(5000);
  pinMode(BUILTIN_LED, OUTPUT);

  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600
  Serial.println("Serial Connection to GPS ok");
  
  digitalWrite(BUILTIN_LED, LOW);
  lastSend = millis();
 
  initTRACCAR();
  startWiFi(20);
}

// the loop function runs over and over again forever
int statoConnessione = 0;
void loop() {
  smartDelay(1000);                                      //Acquisisce dati dal GPS

  degree = gps.course.deg();
  speed = gps.speed.kmph();

statoConnessione=connectTRACCAR(TRACCAR_OSMAND_PORT);
if (statoConnessione < 1) {
  Serial.println("Start WiFi");
	startWiFi(20);
 }


//statoConnessione = connectTRACCAR(TRACCAR_GPS103_PORT);
//if (statoConnessione < 1) {
//  Serial.println("Start WiFi");
//	startWiFi(20);
//}

  // generate ISO time string for OSMAND Protocol
if (gps.date.isUpdated() && gps.date.isValid()) {
	sprintf(isotime, "%04u-%02u-%02uT%02u:%02u:%02u.%01uZ",
		gps.date.year(), gps.date.month(), gps.date.day(),
		gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
}

// generate ISO time string for GPS103 Protocol
//if (gps.date.isUpdated() && gps.date.isValid()) {
//	sprintf(isotime, "%02u%02u%02u%02u%02u%02u",
//		gps.date.year()-2000, gps.date.month(), gps.date.day(),
//		gps.time.hour(), gps.time.minute(), gps.time.second());
//}



  //controllo che il tempo del GPS non sia identico al ciclo precedente
  if ( strcmp(isotime, lastisotime) == 0 ) {
    //   check UTC timestamp from GPS
    return;
  }
  strcpy(lastisotime, isotime);

  printInfoGPS(gps);

  //invia i dati al server ogni X secondi o in base alla differenza di direzione rispetto all'invio dati precedente
  if ((millis() - lastSend > timeToSendDataToServer) || (abs(lastDegree - degree) >= minDegToSendDataToServer)) {
    // turn on  indicator LED
    digitalWrite(BUILTIN_LED, HIGH);
    Serial.println(F("\n\nSEND DATA\n"));
 
  // send data
    OsmAndProtocol(gps, isotime);
    
	//gps103 NON FUNZIONA ANCORA
	//GPS103Protocol(gps, isotime);

    //  // turn off indicator LED
    digitalWrite(BUILTIN_LED, LOW);

    //memorizza alcuni parametri per valutare il prossimo invio dati al server
    lastSend = millis();
    lastDegree = gps.course.deg();
  }



  //se la velocità è bassa aumento la frequenza di invio, altrimenti la diminuisco
  //se la velocità è bassissima allora diminuisco l'invio

  if (speed < velocita_soglia_min) {
    timeToSendDataToServer = FREQMIN;
  } else if (speed >= velocita_soglia_min && speed < velocita_soglia_max) {
    timeToSendDataToServer = FREQMAX ;
  } else if (speed >= velocita_soglia_max) {
    timeToSendDataToServer = FREQMED;
  }
  Serial.print(F("Set frequency data send to "));
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
  Serial.println();
  Serial.print(F("Location is updated: "));
  Serial.println(gps.location.isUpdated());
  Serial.print(F("Location is valid: "));
  Serial.println(gps.location.isValid());

  Serial.print(F("Latitude  : "));
  Serial.println(gps.location.lat(), 6);
  Serial.print(F("Longitude : "));
  Serial.println(gps.location.lng(), 6);
  Serial.print(F("Satellites: "));
  Serial.println(gps.satellites.value());
  Serial.print(F("Elevation : "));
  Serial.print(gps.altitude.meters());
  Serial.println(F("m"));
  //printGPSDateTime();
  Serial.print(F("Heading   : "));
  Serial.println(gps.course.deg());
  Serial.print(F("Speed     : "));
  Serial.println(gps.speed.kmph());
  Serial.print(F("isotime: "));
  Serial.println(isotime);

}
