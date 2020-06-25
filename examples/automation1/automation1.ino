/*
   Door Chime
   Sound a chime when a door is opened

   The chime trigger is a message from sensor ID 25 that is configured as a door switch
   Change the tigger_message to whatever incoming trigger message you need

   This application is an example of non-blocking code where there are no delay statements or loops.
   Because the ATMEGA chips are single processor we need to ensure all activities get processing time.
   We do this by writing non blocking code where every task get equal processing. We have two tasks
   in this application 1) Reading from the serial port for RF messages, 2)Sounding the Piezzo buzzer
   Non blocking code is especially important when monitoring the serial port because we could miss some
   RF messages if we are not constantly monitoring and reading from the serial port

   For use with radio modules from www.jemrf.com

*/

#include <rflib.h>
#define SPEAKER_PIN 11
RFLIB rflib;

static char * trigger_message = "a25BUTTONON";

int chime = 0;  //indicator for chime
unsigned long updateInterval = 100; //beep time interval (ms)
int number_beeps = 10; //the total number of beeps for the chime
int beep_cnt = 0; //a variable used to count the beeps
unsigned long lastUpdate; //the time of the last chime update
int toggle; //a variable used to toggle the chime on/off

void myFunc(){
  Serial.print("Got Message : ");
  Serial.println(rflib.message_in);
  //We got a message let's check if it's the message we are looking for
  if (strncmp(rflib.message_in, trigger_message, 11) == 0) {
    Serial.println("Sounding chime");
    chime = true;
    beep_cnt = 0;
  }
}

void setup() {
  Serial.begin(9600);
  rflib.RegisterCallback(myFunc);
  rflib.begin();
  
}

void piezzo()
{
  //we are using tone and noTone functions to control the Piezzo Buzzer
  //both these functions are non-blocking functions
  
  if (chime) {
    if ((millis() - lastUpdate) > updateInterval) // time to toggle
    {
      lastUpdate = millis();
      toggle = !toggle;
      beep_cnt++;
    }
    if (toggle) {
      tone(SPEAKER_PIN, 1000);
    }
    else {
      noTone(SPEAKER_PIN);
    }
  }
  else {
    noTone(SPEAKER_PIN);
  }

  if (beep_cnt > number_beeps) {
    chime = false;
    noTone(SPEAKER_PIN);
  }
}

void loop() {
  rflib.process_rf();

  if (chime) {
    piezzo();
  }

}
