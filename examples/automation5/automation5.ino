/*
   Switch and LED on when a door is open and off when the door is closed
   Use the button STATE as a means to switch the LED on/off as backup

   Uses: Have a warning light in your house to indicate the garage door 
         or some other exterior door is open

    Radio ID
   - 55 Wireless switch
   
   For use with radio modules from www.jemrf.com

*/

#include <rflib.h>

RFLIB rflib;

int requested_state;
int ledPin = 11; // the number of the LED pin

void myFunc() {

  Serial.println("Got message...");
  Serial.println(rflib.message_in);

  if (strncmp(rflib.message_in, "a55BUTTONON-", 12) == 0 || strncmp(rflib.message_in, "a55STATEON--", 12) == 0) {
    digitalWrite(ledPin, 1);
  }

  if (strncmp(rflib.message_in, "a55BUTTONOFF", 12) == 0 || strncmp(rflib.message_in, "a55STATEOFF-", 12) == 0) {
    digitalWrite(ledPin, 0);
  }

}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  rflib.RegisterCallback(myFunc);
  rflib.begin();
}

void loop() {
  rflib.process_rf();
}
