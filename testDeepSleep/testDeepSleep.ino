#include <Ticker.h>
Ticker flipper;

void setup() {
 pinMode(5, OUTPUT);
 flipper.attach(0.02, flip);

}

void loop() {
  delay(2000);
  //Serial.println("Going into deep sleep for 20 seconds");
  //ESP.deepSleep(20e6); // 20e6 is 20 seconds
}

void flip()
{
  int state = digitalRead(5);  // get the current state of GPIO1 pin
  digitalWrite(5, !state);     // set pin to the opposite state
  
}
