#include <Arduino.h>
#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <ESP8266WiFi.h>

//imposta l'ID del dispositivo
void initTRACCAR(void);

//Connette il dispositivo al server TRACCAR
int connectTRACCAR(int);

//invia dati al server con il protocollo OSMAND
void OsmAndProtocol(TinyGPSPlus, char[24]);

//invia dati al server con il protocollo GPS103
void GPS103Protocol(TinyGPSPlus, char[24]);
