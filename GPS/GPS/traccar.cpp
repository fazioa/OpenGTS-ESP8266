//#include <Arduino.h>
#include "parameter.h"
#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <ESP8266WiFi.h>

char data[128];
String URL = "";
String gpsdata = "";
int clientResponse = 0;
String traccar_dev_ID = "";
//Wifi
WiFiClient client;

//imposta l'ID del dispositivo
void initTRACCAR() {
  Serial.print(F("Board ID: "));
#ifdef TRACCAR_DEV_ID
  traccar_dev_ID = TRACCAR_DEV_ID;
#else
  traccar_dev_ID = ESP.getChipId();
#endif
  Serial.println(traccar_dev_ID);
}

//invia dati al server con il protocollo OSMAND
void OsmAndProtocol(TinyGPSPlus gps, char gpsisotime[24]) {
  // now that new GPS coordinates are available

  //if (gps.date.isUpdated() && gps.date.isValid() )
  //{
  //  // generate ISO time string
  //  sprintf(gpsisotime, "%04u-%02u-%02uT%02u:%02u:%02u.%01uZ",
  //          gps.date.year(), gps.date.month(), gps.date.day(),
  //          gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
 
  //}

  if (gps.location.isUpdated() && gps.location.isValid() && gps.location.lat() != 0 && gps.location.lng() != 0) {
    // arrange and send data in OsmAnd protocol
    // refer to https://www.traccar.org/osmand
    gpsdata = "&lat=" + String(gps.location.lat(), 6) + "&lon=" + String(gps.location.lng(), 6)   + "&altitude=" + String(gps.altitude.meters(), 1) + "&speed=" + String(gps.speed.kmph(), 1) + "&heading=" + String(gps.course.deg(), 1);
    //Serial.println("gpsdata");
   // Serial.println(gpsdata);
    URL = "GET /?id=" + String(traccar_dev_ID) + "&timestamp=" + gpsisotime + gpsdata + " HTTP/1.1\r\n" +
          "Host: " + TRACCAR_HOST + "\r\n" +
          "Connection: keep-alive\r\n\r\n";
         Serial.println("URL");
         Serial.println(URL);
  }

  //invia dati al server
       client.print(URL); 
  
    // output server response
    while (client.available()) {
      Serial.print((char)client.read());
    }

}

//Connette il dispositivo al server TRACCAR
int connectTRACCAR(int PORT){
// check connection and reconnect to Traccar server if disconnected
  if (!client.connected()) {
    Serial.print(F("Connecting to "));
    Serial.print(TRACCAR_HOST);
	Serial.print(":");
	Serial.print(PORT);
    Serial.print(F("..."));
    clientResponse = client.connect(TRACCAR_HOST, PORT);

    if (clientResponse == 1) {
      Serial.println(F("OK"));
    } else {
      Serial.print(F("Connection error - Code: "));
      Serial.println(clientResponse);
      switch (clientResponse) {
        case -1:
          Serial.println(F("TIMED_OUT "));
          break;
        case -2:
          Serial.println(F("INVALID_SERVER"));
          break;
        case -3:
          Serial.println(F("TRUNCATED"));
          break;
        case -4:
          Serial.println(F("INVALID_RESPONSE "));
          break;
      }
      Serial.println(F("Stop Client"));
      client.stop();
      delay(5000);
     
    }
	return clientResponse;
  }
}

//NON FUNZIONA ANCORA
//imei:359587010124900,tracker,0809231929,13554900601,F,112909.397,A,2234.4669,N,11354.3287,E,0.11,;
void GPS103Protocol(TinyGPSPlus gps, char gpsisotime[24]) {

	//if (gps.date.isUpdated() && gps.date.isValid())
	//{
	//	// generate time string
	//	sprintf(gpsisotime, "%04u-%02u-%02uT%02u:%02u:%02u.%01uZ",
	//	//	gps.date.year(), gps.date.month(), gps.date.day(),
	//	//	gps.time.hour(), gps.time.minute(), gps.time.second(), gps.time.centisecond());
	//	//Serial.println("gpsisotime");
	//	//Serial.println(gpsisotime);
	//}

	if (gps.location.isUpdated() && gps.location.isValid() && gps.location.lat() != 0 && gps.location.lng() != 0) {
		//// arrange and send data in GPS103 protocol
		gpsdata="imei:" + String(traccar_dev_ID) + ",tracker," + gpsisotime + "," + String(TRACCAR_GPS103_CELLPHONENUMBER) +  ",F,,A,"+ String(gps.location.lat()*1000, 3) + ",N," + String(gps.location.lng()*1000, 3) + ",E,"+ String(gps.speed.kmph(), 1) + ","+ String(gps.course.deg(), 1)+";";

		//gpsdata = "&lat=" + String(gps.location.lat(), 6) + "&lon=" + String(gps.location.lng(), 6) + "&altitude=" + String(gps.altitude.meters(), 1) + "&speed=" + String(gps.speed.kmph(), 1) + "&heading=" + String(gps.course.deg(), 1);
		Serial.println("gpsdata");
		Serial.println(gpsdata);
		//URL = "GET /?id=" + String(traccar_dev_ID) + "&timestamp=" + gpsisotime + gpsdata + " HTTP/1.1\r\n" +
		//	"Host: " + TRACCAR_HOST + "\r\n" +
		//	"Connection: keep-alive\r\n\r\n";
		//Serial.println("URL");
	//	Serial.println(URL);
	}

	//invia dati al server
	client.print(URL);

	// output server response
	while (client.available()) {
		Serial.print((char)client.read());
	}

}
