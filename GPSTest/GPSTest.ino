// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>                                 // Tiny GPS Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module

static const int RXPin = 4, TXPin = 5;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

//Some variable
int TRACCAR_DEV_ID = ESP.getChipId();
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device
TinyGPSPlus  gps;                                        // Create an Instance of the TinyGPS++ object called gps

unsigned long lastmillis;
uint8_t GPS_GNSS_stop_msg1[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x4F, 0x54, 0x53, 0xAC, 0x85}; //GNSS stop
uint8_t GPS_GNSS_stop_msg2[] = {0xB5, 0x62, 0x06, 0x57, 0x00, 0x00, 0x5D, 0x1D}; //GNSS stop

//B5 62 06 57 08 00 01 00 00 00 50 4F 54 53 AC 85
uint8_t GPS_GNSS_stop[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x01, 0x00, 0x53, 0x54, 0x4F, 0x50, 0x00, 0x00}; //GNSS stop


 boolean bFlag=false;
int GPS_POWER=3;
void setup() {
  Serial.begin(9600);
   ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600
pinMode(GPS_POWER, OUTPUT);
digitalWrite(GPS_POWER, LOW);


    Serial.println("START");

//Enter system state
//0x52554E20: GNSS running
//0x53544F50: GNSS stopped
//0x42434B50: Software Backup. USB
//interface will be disabled, other wakeup
//source is needed.


    
    //uint8_t GPSoff[] = {0xB5, 0x62, 0x06, 0x57, 0x01, 0x00, 0x42, 0x43, 0x4B, 0x50, 0x00, 0x00}; //Software Backup
    //uint8_t GPS_GNSS_Run[] = {0xB5, 0x62, 0x06, 0x57, 0x01, 0x00, 0x52, 0x55, 0x4E, 0x20, 0x00, 0x00}; //GNSS running
//B5 62 06 57 08 00 01 00 00 00 50 4F 54 53 AC 85

//0xB5 0x62 0x06 0x08
// measRate   0x88, 0x13 per 5000ms
// navRate
//timeRef
// uint8_t GPS_rate[] = {0xB5, 0x62, 0x06, 0x08, 0x88, 0x13, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00}; //GNSS stopped
    
byte CK_A, CK_B;
// Calculate checksum
      for(int i=0; i<sizeof(GPS_GNSS_stop); i++){
        CK_A = CK_A + GPS_GNSS_stop[i];
        CK_B = CK_B + CK_A;
       // Uncomment next two lines to see steps in checksum calculation
    //    Serial.println("CK_A= " + String(CK_A));
       // Serial.println("CK_B= " + String(CK_B));
     }
GPS_GNSS_stop[sizeof(GPS_GNSS_stop)-2]=CK_A;
GPS_GNSS_stop[sizeof(GPS_GNSS_stop)-1]=CK_B;

// //Calculate checksum
//      for(int i=0; i<sizeof(GPS_rate)/sizeof(uint8_t); i++){
//        CK_A = CK_A + GPS_rate[i];
//        CK_B = CK_B + CK_A;
//        //Uncomment next two lines to see steps in checksum calculation
//        //Serial.println("CK_A= " + String(CK_A));
//        //Serial.println("CK_B= " + String(CK_B));
//      }
//GPS_rate[10]=CK_A;
//GPS_rate[11]=CK_B;


//sendUBX(GPS_GNSS_stop_msg1, sizeof(GPS_GNSS_stop_msg1)/sizeof(uint8_t));
 //delay(50);
//sendUBX(GPS_GNSS_stop_msg2, sizeof(GPS_GNSS_stop_msg2)/sizeof(uint8_t));

lastmillis=millis();
}


// the loop function runs over and over again forever
void loop() {
 
    String read = ss.readStringUntil('\n');
     Serial.println(read);


if(millis()-lastmillis >5000 && bFlag==false){
  Serial.println("STOPPING GPS");
  //sendUBX(GPS_GNSS_stop_msg1, sizeof(GPS_GNSS_stop_msg1));
 // sendUBX(GPS_GNSS_stop, sizeof(GPS_GNSS_stop));
  digitalWrite(GPS_POWER, HIGH);

//delay(200);
  //sendUBX(GPS_GNSS_stop_msg2, sizeof(GPS_GNSS_stop_msg2));
  bFlag=true;
  
}


}

// Send a byte array of UBX protocol to the GPS
void sendUBX(uint8_t *MSG, uint8_t len) {
  for(int i=0; i<len; i++) {
    ss.write(MSG[i]);
    Serial.print(MSG[i]);
    Serial.print(" ");
  }
  //Serial.println();
}
