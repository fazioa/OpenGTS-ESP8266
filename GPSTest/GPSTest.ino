// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module

static const int RXPin = 0, TXPin = 2;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

//Some variable
int TRACCAR_DEV_ID = ESP.getChipId();
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device
TinyGPSPlus  gps;                                        // Create an Instance of the TinyGPS++ object called gps

void setup() {
  Serial.begin(9600);
   ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600

    Serial.println("START");
}



// the loop function runs over and over again forever
void loop() {
 
    String read = ss.readStringUntil('\n');
     Serial.println(read);
   

}
