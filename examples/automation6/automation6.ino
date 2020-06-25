/*
   Switch and LED on when a switch was opened
   Switch off the LED with a tactile button

   Uses:  - A warning light that water was detected (flood detection or sump pump faulty)
          - You mail mail in you post box (post box must have a door for the magnetic switch)
          - Your plant needs watering :-)
          - Motion has been detected
          - any other use where a magnetic switch can be installed

    Radio ID's
   - 55 Wireless switch or water sensor or motion sensor
   
   For use with radio modules from www.jemrf.com
*/

#include <rflib.h>

RFLIB rflib;

int requested_state;
int ledPin = 11; // the number of the LED pin
int inPin = 10; // the number of the tactile switch pin
int reading;

void myFunc() {

  Serial.println("Got message...");
  Serial.println(rflib.message_in);

  if (strncmp(rflib.message_in, "a55BUTTONON-", 12) == 0 || strncmp(rflib.message_in, "a55STATEON--", 12) == 0) {
    digitalWrite(ledPin, 1);
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

 //Read the status of the tactile switch
  reading = digitalRead(inPin);
  
  if (reading == true) {
    digitalWrite(ledPin, 0);
  }
}
