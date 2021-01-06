/*
   Switch light on/off depending on light sensor reading

   There are three main parts to this code:
   1. Receive a message from a light sensor
   2. Send a status request message to the remote relay and wait for the reply
   3. Sending a command to the relay to switch either on or off depending on the light sensor value and the status of the light switch

   Application logic:

   1. Power On
   2. When a message arrives from the light sensor determine if the remote relay should be on off depending on the light sensor value
   3. Query the status of the light switch
   4. When the reply arrives then determine if the light needs to be turned on/off

   Radio ID's
   - 55 Light sensor
   - 61 Remote relay
   
   For use with radio modules from www.jemrf.com

*/


#include <rflib.h>
#define LIGHT_OFF_TRIGGER 400
#define LIGHT_ON_TRIGGER 1500

RFLIB rflib;

int requested_state;

void myFunc() {
  int val;
  char buf[6];

  Serial.println("Got message...");
  Serial.println(rflib.message_in);
  Serial.println(requested_state);

  //2. When a message arrives from the light sensor determine if the remote relay should be on off depending on the light sensor value
  if (strncmp(rflib.message_in, "a55ANAA", 7) == 0) {
    strncpy(buf, rflib.message_in + 7, 5);
    val = atoi(buf);

    if (val < LIGHT_ON_TRIGGER) {
      requested_state = LOW;
      Serial.println("LOW");
    }

    if (val > LIGHT_OFF_TRIGGER) {
      requested_state = HIGH;
      Serial.println("HIGH");
    }
    //3. Query the status of the light switch
    rflib.transmit("a61RELAYA---");
  }

  //4. When the reply arrives then determine if the light needs to be turned on/off
  if (strncmp(rflib.message_in, "a61RELAYAON-", 12) == 0 && requested_state == LOW) {
    if (rflib.got_ack) { //We asked to have the light switched OFF, and it's ON, so report an error
      Serial.println("Error with light switch");
    }
    else { //the light is OFF and I want it ON so switch ON
      Serial.println("Switching light on");
      rflib.transmit("a61RELAYAOFF"); //turn light on
    }
  }

  if (strncmp(rflib.message_in, "a61RELAYAOFF", 12) == 0 && requested_state == HIGH) {
    if (rflib.got_ack) { //We asked to have the light switched ON, and it's OFF, so report an error
      Serial.println("Error with light switch");
    }
    else { //the light is ON and I want it OFF so switch OFF
      Serial.println("Switching light off");
      rflib.transmit("a61RELAYAON-"); //turn light off
    }
  }
}


void setup() {
  //1. Power On
  Serial.begin(9600);
  rflib.RegisterCallback(myFunc);
  rflib.begin();
}

void loop() {
  rflib.process_rf();
}
