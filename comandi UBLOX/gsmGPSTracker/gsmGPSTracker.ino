// Comment this out if you don't have a screen. If you have a different type of screen,
// you'll need to replace all the screen calls as appropriate for your screen.
// Note that even if you use the same Nokia 5110 screen as me, you may need to adjust
// the contrast to get it to show clearly. And don't use 5V to power it, use 3.3V!
#define USE_SCREEN

// This must be a ten-character string with no spaces, slashes,ampersands or question marks.
// Choosing it randomly should be fine, the chances of colliding with someone else are
// pretty small. If you want to check that an id is free you can use this URL:
//    http://www.iforce2d.net/gt/checkJourney.php?jn=vr3bkwye7R
// Of course, change the last part of the URL to the id you want to check.
#define JOURNEY "vr3bkwye7R"

// To view the journey results, use this URL:
//    http://www.iforce2d.net/gt/index.php?jn=vr3bkwye7R&tz=13&os=10&lt=50
// The parameters are:
//    jn - the id of the journey to view
//    tz - the difference in hours from UTC/GMT of your time zone
//    os - an offset in the list of recorded points to start from
//    lt - a limit on the number of points to show
// The last two parameters are mainly helpful when a journey has a large number of points, because
// the map can be a bit slow to generate. The map will also be positioned and zoomed into the first
// point every time, which can be annoying if you wanted to focus on a specific part of the journey.

#include <Arduino.h> // prevents error: 'byte' does not name a type

#ifdef USE_SCREEN
  #include <SPI.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
  
  // Nokia 5110 screen
  // Hardware SPI (faster, but must use certain hardware pins):
  // SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
  // MOSI is LCD DIN - this is pin 11 on an Arduino Uno
  // pin 5 - Data/Command select (D/C)
  // pin 4 - LCD chip select (CS)
  // pin 3 - LCD reset (RST)
  Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
  // Note with hardware SPI MISO and SS pins aren't used but will still be read
  // and written to during SPI transfer.  Be careful sharing these pins!
#endif // USE_SCREEN




#define GSM_PORT softSerial // 'softSerial' is a software serial connection declared below (pins 6,7)
//#define GSM_PORT Serial3 // on the mega2560 you should be able to do this instead to use hardware serial 3 (pins 14,15) - not tested



// mega2560 - allows us to talk to GSM, GPS and serial monitor together
#define GPS_PORT Serial2 // use pins 16,17 for GPS and free up the default UART
#define DEBUG_SERIAL // this will enable logging to the default UART, ie. 'Serial' on pins 0,1


// mega328 - can only talk to GSM and GPS
#define GPS_PORT Serial // use hardware serial (pins 0,1) for GPS. Remember to disconnect it when uploading the sketch!
#undef DEBUG_SERIAL // since hardware serial is used for GPS we can't do any logging to it




#include <SoftwareSerial.h>

SoftwareSerial softSerial(6, 7); // RX, TX
#include "GSM.h"
#include "GPS.h"

const char UBLOX_INIT[] PROGMEM = {
  // Disable NMEAs
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x24, //(GxGGA)
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x2B, //(GxGLL)
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x02,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x32, //(GxGSA)
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x39, //(GxGSV)
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x04,0x00,0x00,0x00,0x00,0x00,0x01,0x04,0x40, //(GxRMC)
  0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x05,0x00,0x00,0x00,0x00,0x00,0x01,0x05,0x47, //(GxVTG)

  // Disable UBX
  0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0xDC, //NAV-PVT off
  0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0xB9, //NAV-POSLLH off
  0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0xC0, //NAV-STATUS off

  // Enable UBX
  0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x07,0x00,0x01,0x00,0x00,0x00,0x00,0x18,0xE1, //NAV-PVT on
  //0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x13,0xBE, //NAV-POSLLH on
  //0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x03,0x00,0x01,0x00,0x00,0x00,0x00,0x14,0xC5, //NAV-STATUS on

  // RATE
  //0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12, //(10Hz)
  0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A   //(5Hz)
};

void setup()
{
  GPS_PORT.begin(9600);
  GSM_PORT.begin(9600);
#ifdef DEBUG_SERIAL
  Serial.begin(9600);
#endif

#ifdef USE_SCREEN
  display.begin();
  display.setContrast(55); // you may need to adjust this for your screen

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);

  display.clearDisplay();
  display.println("Init GPS...");  
  display.display();
#endif // USE_SCREEN

  delay(1000);

  // send configuration data in UBX protocol
  for(uint8_t i=0; i<sizeof(UBLOX_INIT); i++) {                        
    GPS_PORT.write(pgm_read_byte(UBLOX_INIT+i));
    delay(5); //simulating a 38400baud pace (or less), otherwise commands are not accepted by the device.
  }

#ifdef USE_SCREEN
  display.println("Wait GSM...");  
  display.display();
#endif // USE_SCREEN

  // wait ten seconds for GSM module to connect to mobile network
  delay(10000);

  sendGSM("AT+SAPBR=3,1,\"APN\",\"vodafone\"");  // change this for your cell provider
  sendGSM("AT+SAPBR=1,1",3000);
  sendGSM("AT+HTTPINIT");
  sendGSM("AT+HTTPPARA=\"CID\",1");

#ifdef USE_SCREEN  
  display.println("Check GPS...");
  display.display();
#endif // USE_SCREEN

  // process anything received from GPS so far, to avoid first
  // reportgiving zero values when the GPS already has a lock
  for (byte i = 0; i < 10; i++) {
    if ( processGPS() )
      break;
    delay(100);
  }  

#ifdef USE_SCREEN
  display.println("Done.");
  display.display();
#endif // USE_SCREEN
}

// process all incoming bytes available from the GSM module. This is 
// to ensure we don't miss anything - call it regularly.
void flushGSM(unsigned long now) {
  while(GSM_PORT.available()) {
    lastActionTime = now;
    parseATText(GSM_PORT.read());
  }
}

char* spinner = "/-\\|";
byte spinnerPos = 0;

void loop()
{ 
  unsigned long now = millis();
  static unsigned long lastScreenUpdate = 0;
  static unsigned long lastGPSRead = 0;

  boolean gotGPS = false;

  gotGPS |= processGPS();

  if ( actionState == AS_IDLE ) {
    if ( loc.fixType > 0 && now > lastActionTime + 10000 ) {

      GSM_PORT.print("AT+HTTPPARA=\"URL\",\"http://www.iforce2d.net/gt/gt.php?");
      GSM_PORT.print("&jn=");
      GSM_PORT.print(JOURNEY);
      GSM_PORT.print("&tm=");
      GSM_PORT.print(loc.year);
      if ( loc.month < 10 ) GSM_PORT.print("0");
      GSM_PORT.print(loc.month);
      if ( loc.day < 10 ) GSM_PORT.print("0");      
      GSM_PORT.print(loc.day);
      if ( loc.hour < 10 ) GSM_PORT.print("0");
      GSM_PORT.print(loc.hour);
      if ( loc.minute < 10 ) GSM_PORT.print("0");
      GSM_PORT.print(loc.minute);
      if ( loc.second < 10 ) GSM_PORT.print("0");
      GSM_PORT.print(loc.second);
      GSM_PORT.print("&fx=");
      GSM_PORT.print(loc.fixType);
      GSM_PORT.print("&lt=");
      GSM_PORT.print(loc.lat);
      GSM_PORT.print("&ln=");
      GSM_PORT.print(loc.lon);
      GSM_PORT.print("&sv=");
      GSM_PORT.print(loc.numSV);
      GSM_PORT.print("&ha=");
      GSM_PORT.print(loc.hAcc);
      GSM_PORT.print("&gs=");
      GSM_PORT.print(loc.gSpeed);
      GSM_PORT.print("&hd=");
      GSM_PORT.print(loc.heading);
      GSM_PORT.println("\"");
      
      flushGSM(now);
      
      delay(500);

      gotGPS |= processGPS();

      sendGSM("AT+HTTPACTION=0");

      gotGPS |= processGPS();

      lastActionTime = now;
      httpResult = 0;
      actionState = AS_WAITING_FOR_RESPONSE;
    }
  }
  else {
    // waiting on response - abort if taking too long
    if ( now > lastActionTime + 15000 ) {
      actionState = AS_IDLE;
      parseState = PS_DETECT_MSG_TYPE;
      resetBuffer();
    }
  }

  flushGSM(now);

  // if the 'gotGPS' variable is true, then one of the processGPS calls 
  // done in this loop iteration succeeded.
  if ( gotGPS )
    lastGPSRead = now;
    
        
#ifdef USE_SCREEN

  if ( (now - lastScreenUpdate) > 500 ) {

    display.clearDisplay();

    flushGSM(now);

    display.print(spinner[spinnerPos]);
    spinnerPos = (spinnerPos + 1) % 4;

    if ( (now - lastGPSRead) < 3000 ) {
      
      display.print(" Sv:");
      display.print(loc.numSV);
      
      flushGSM(now);
      
      display.print(" (");      
      display.print(httpResult);
      display.println(")");

      flushGSM(now);

      display.print("Lt:");
      display.println(loc.lat);

      flushGSM(now);

      display.print("Ln:");
      display.println(loc.lon);

      flushGSM(now);

      display.print("hAcc: ");
      display.println(loc.hAcc);

      flushGSM(now);

      display.print("gSpeed: ");
      display.println(loc.gSpeed);

      flushGSM(now);

      display.print("Head: ");
      display.println(loc.heading);
      
      flushGSM(now);
    }
    else {
      
      loc.numSV = 0;
      loc.fixType = 0;
      
      display.println(" No GPS ");

      flushGSM(now);

      display.print("Lt:");
      display.println(loc.lat);

      flushGSM(now);

      display.print("Ln:");
      display.println(loc.lon);

      flushGSM(now);
    }

    display.display();
    lastScreenUpdate = now;

    flushGSM(now);
  }
#endif // USE_SCREEN
}















