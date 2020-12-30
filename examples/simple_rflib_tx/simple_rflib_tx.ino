/*
  A simple example that use RFLIB to transmit a03HELLO---- message every 5 seconds

  For use with radio modules from www.jemrf.com
*/

#include <rflib.h>

RFLIB rflib;

void setup() {
  rflib.begin();
}

void loop() {
  rflib.transmit("a01HELLO----");
  delay(5000);
}
