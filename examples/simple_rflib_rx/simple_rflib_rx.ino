/*
  A simple example to receive radio messages and print them to the serial_port

  This example uses the rflib library to receive RF messages

  For use with radio modules from www.jemrf.com
*/

#include <rflib.h>

RFLIB rflib;

int requested_state;

void myFunc() {

  Serial.println("Got message...");
  Serial.println(rflib.message_in);

}

void setup() {
  Serial.begin(9600);
  rflib.RegisterCallback(myFunc);
  rflib.begin();
}

void loop() {
  //Important to call profess_rf() frequently and have no delays in this loop
  rflib.process_rf();
}
