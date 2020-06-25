/*
   simple_rx.ino

   This example monitors the SoftwareSerial port where pins 2 and 3 are connected to the
   RF module TX and RX respectively. Incoming RF messages are detected using the "a"
   start charachter and sliced into 12 charachter messages per the message protocol

   See also:
    - simple_rflib_rx example that uses the rflib library for receving RF messages.
    - mtr_rf_demo that uses a multiprocessing technique to allow you to monitor the serial
      port while also allowing you to easily incorporate your own application code.

   For use with radio modules from www.jemrf.com
*/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  int x = 0;
  char inChar;
  while (true) {
    while (mySerial.available()) { //# we have a message arriving
      inChar = mySerial.read();
      if (inChar == 'a') { //start charachter detected
        x = 0;
      }
      x++;
      Serial.print(inChar);
      if (x == 12) //message complete so print it
        Serial.println();
    }
  }
}
