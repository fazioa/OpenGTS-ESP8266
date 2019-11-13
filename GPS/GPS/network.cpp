#include "credenziali.h"
#include <ESP8266WiFi.h>

void startWiFi(unsigned long timeout) {
  // If is the first time that we connect to WiFi.SSID
  if (strcmp(WiFi.SSID().c_str(), WiFi_SSID) || strcmp(WiFi.psk().c_str(), WiFi_Password))
  {
    Serial.println(F("Connessione WiFi"));
    WiFi.mode(WIFI_STA);
	WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.begin(WiFi_SSID, WiFi_Password);
  } else {
    Serial.println(F("Riconnessione WiFi"));
    WiFi.begin(); // WiFi.SSID is a known network, no need to specify it
  }

  // Connect
  while ((WiFi.status() != WL_CONNECTED) && timeout)
  {
    timeout--;
    Serial.println(F("."));
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    digitalWrite(BUILTIN_LED, LOW);
  }
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(BUILTIN_LED, HIGH);
  } else {
    Serial.println(F("Failed connection"));
  //  Serial.println(F("Force Sleep Wake"));
   // WiFi.forceSleepWake();
  }
}
