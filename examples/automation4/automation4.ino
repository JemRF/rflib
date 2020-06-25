/*
   Switch light on when motion detected and off after a time period
   Use the button STATE as a means to switch the light off if light did not switch
   off after the timer period

   Application logic:
   1. Motion is detected
   2. Turn light on 
   3. Start the timer
   4. Turn the light off after time expires

    Radio ID's
   - 55 Motion Sensor
   - 61 Remote relay
   
   For use with radio modules from www.jemrf.com
*/

#include <rflib.h>

RFLIB rflib;

int requested_state;
unsigned long start_time=0;

void myFunc() {

  Serial.println("Got message...");
  Serial.println(rflib.message_in);

  //1. Motion is detected
  if (strncmp(rflib.message_in, "a55BUTTONON", 11) == 0) {
    Serial.println("Switching light on");
    //2. Turn light on
    rflib.transmit("a61RELAYAOFF"); //turn light on
    //3. Start the timer
    start_time = millis();
  }

 //A motion sensor will send a status message every NOMSG minutes
 //This message can be used as a way to switch the light off in 
 //case the light off message was not received
  if (strncmp(rflib.message_in, "a55STATEOFF", 11) == 0 && start_time==0) {
    Serial.println("Switching light off");
    rflib.transmit("a61RELAYAON-"); //turn light off
  }
}

void do_time_check() {
  if (start_time == 0)
    return;  //timer not set so exit

  //4. Turn the light off after time expires
  if (millis() - start_time > 300000) { //5 minutes
    Serial.println("Switching light off");
    rflib.transmit("a61RELAYAON-"); //turn light off
    start_time=0;
  }
}

void setup() {
  Serial.begin(9600);
  rflib.RegisterCallback(myFunc);
  rflib.begin();
}

void loop() {
  rflib.process_rf();
  do_time_check();
}
