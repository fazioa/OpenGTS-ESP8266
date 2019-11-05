extern "C" {
#include "user_interface.h"
}

// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include "credenziali.h"
#include "parameter.h"

//Creare un file di testo con il seguente contenuto
//SSID
//const char* ssid = "*****";
//const char* password = "*****";

#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module


static const int RXPin = 0, TXPin = 2;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600


static unsigned long timeToSendDataToServer; //fa frequenza di trasmissione viene rimodulata ad ogni cilo in base alla veloctà rilevata


static const int minDegToSendDataToServer = 30;


char isotime[24];
char lastisotime[24];
char data[128];
unsigned long lastSend = 0;
int lastDegree = 0, degree = 0;
float speed = 0;

//Wifi
WiFiClient client;

//Some variable
int TRACCAR_DEV_ID = ESP.getChipId();
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device
TinyGPSPlus  gps;                                        // Create an Instance of the TinyGPS++ object called gps

void setup() {
  //pausa 5 secondi
  delay(5000);

  Serial.begin(9600);
  Serial.println(F("START"));
  pinMode(BUILTIN_LED, OUTPUT);

  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600
  Serial.println("Serial Connection to GPS ok");

  digitalWrite(BUILTIN_LED, LOW);
  lastSend = millis();
  Serial.print("Free Ram: ");
  Serial.println(system_get_free_heap_size());
}



// the loop function runs over and over again forever
void loop() {
  testWIFI();

  smartDelay(1000);                                      // Run Procedure smartDelay

  degree = gps.course.deg();
  speed = gps.speed.kmph();




  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

  // check WiFi connection and connect to Traccar server if disconnected
  if (!client.connected()) {
    Serial.print(F("Connecting to "));
    Serial.print(TRACCAR_HOST);
    Serial.print(F("..."));
    if (client.connect(TRACCAR_HOST, TRACCAR_PORT)) {
      Serial.println(F("OK"));
    } else {
      Serial.println(F("failed"));
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
  if ((millis() - lastSend > timeToSendDataToServer) || (abs(lastDegree - degree) >= minDegToSendDataToServer)) {
    // turn on  indicator LED
    digitalWrite(BUILTIN_LED, HIGH);
    Serial.println(F("\n\nSEND DATA\n\n"));
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

  Serial.print(F("Free Ram: "));
  Serial.println(system_get_free_heap_size());
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
  Serial.println("");

  Serial.print(F("Heading   : "));
  Serial.println(gps.course.deg());
  Serial.print(F("Speed     : "));
  Serial.println(gps.speed.kmph());
  Serial.print(F("isotime: "));
  Serial.println(isotime);

}


String OsmAndProtocol(TinyGPSPlus gps) {
  // now that new GPS coordinates are available
  double lat, lng, alt, speed;
  String URL = "";

  if (gps.date.isUpdated() && gps.date.isValid() )
  {
    // generate ISO time string
    sprintf(isotime, "%04u-%02u-%02uT%02u:%02u:%02u.%01uZ",
            gps.date.year(), gps.date.month(), gps.date.day(),
            gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
  }

  if (gps.location.isUpdated() && gps.location.isValid() && gps.location.lat() != 0 && gps.location.lng() != 0) {
    // arrange and send data in OsmAnd protocol
    // refer to https://www.traccar.org/osmand
    String data = "&lat=" + String(gps.location.lat(), 6) + "&lon=" + String(gps.location.lng(), 6)   + "&altitude=" + String(gps.altitude.meters(), 1) + "&speed=" + String(gps.speed.kmph(), 1) + "&heading=" + String(gps.course.deg(), 1);
    URL = "GET /?id=" + String(TRACCAR_DEV_ID) + "&timestamp=" + isotime + data + " HTTP/1.1\r\n" +
          "Host: " + TRACCAR_HOST + "\r\n" +
          "Connection: keep-alive\r\n\r\n";
  }
  return URL;
}


static unsigned long wifiTime = 90000;
unsigned long lastMillisWifiTime = millis() + wifiTime;
void testWIFI() {
  if (millis() - lastMillisWifiTime > wifiTime ) {
    //verifica ogni 90 sec che la ESP sia collegata alla rete Wifi (5 tentativi, poi reset ESP)
    int tent = 0;

    Serial.println(F("Verifico connessione"));

    while ((WiFi.status() != WL_CONNECTED) && tent < 4)
    {
      yield();
      delay(1000);
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      int ritardo = 0;
      while ((WiFi.status() != WL_CONNECTED) && ritardo < 10)
      {
        //mentre attende la connessione fa lampeggiare il led
        digitalWrite(BUILTIN_LED, HIGH);
        delay(500);
        Serial.print(F("."));
        digitalWrite(BUILTIN_LED, LOW);
        delay(500);
        ritardo += 1;
        Serial.println(ritardo);
      }

      if (WiFi.status() != WL_CONNECTED )
      {
        delay(2000);
      } else {
        Serial.println("WIFI CONNECTED");
      }
      tent += 1;
    }

    if (tent > 4) {
      Serial.println(F("tentativo non riuscito"));
      ESP.reset();
    }
    lastMillisWifiTime = millis();
  }
}
