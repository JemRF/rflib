/*
   Remote Control
   In this example we use a tactile push button to toggle a remote relay switch on/off.

   Common uses:
    - Switch a light on/off
    - Open/close a garage door

   There are three main parts to this code:
   1. Monitoring the push button and taking action when the button is pushed
   2. Requesting the status of remote relay so we know if the relay is on or off
   3. Sending a command to the relay to switch either on or off

   Application logic:

   1. Power On
   2. Send a status request message to the remote relay and wait for the reply
   3. When the reply arrives then store the STATUS of the remote relay
   4. When the tactile button is pushed then send a command to the remote relay to switch to the opposite STATUS
   5. Send a status request message to the remote relay and wait for a reply
   6. When the reply arrives then store the STATUS of the remote relay
   7. Check that the switch status is the same as the new status we requested

   LEDs:
   Green - indicates when the remote relay switched successfully to the new state (on or off) that was requested
   Orange - is illumintaed when the button is pressed
   Red - indicates we either didnt receive an ACK back from the remote relay or the status of the relay is not what we requested

   Wiring:
   Pin 11 to 1k resistor to +ve leg of green resistor and -ve leg to ground
   Pin 12 to 1k resistor to +ve leg of redn resistor and -ve leg to ground
   Pin 10 to 1k resistor to +ve leg of orange resistor and -ve leg to ground
   +3v3 to 1k resistor to:
    - Pin 4
    - Tactile switch to ground

   Radio ID's
   - 61 Remote relay

   For use with radio modules from www.jemrf.com

*/

#include <rflib.h>
RFLIB rflib;
int greenPin = 11;     // the number of the input pin
int redPin = 12;       // the number of the output pin
int orangePin = 10;    // the number of the output pin
int statusPin = 9;     // the number of the output pin
int inPin = 4;         // the number of the output pin

int state;             // the current state of the remote relay
int reading;           // the current state of the tactile push button

long time = 0;         // the last time the output pin was toggled
long debounce = 400;   // debounce time is the amount ms ms the button should be pressed in order to trigger (prevents false positives from noisy switches)
int previous = LOW;    // the previous reading from the input pin


void myFunc() {
  Serial.println("Got message...");
  Serial.println(rflib.message_in);

  //got_ack = True when we receive an acknowledgement back from receiver that message was received
  if (rflib.got_ack) {
    //5. Send a status request message to the remote relay and wait for a reply
    getButtonState();
    return;
  }

  //3. + 6. store the STATUS of the remote relay
  if (strncmp(rflib.message_in, "a62RELAYAON-", 12) == 0) {
    Serial.println("setting low");
    state = LOW;
  }

  if (strncmp(rflib.message_in, "a62RELAYAOFF", 12) == 0) {
    Serial.println("setting high");
    state = HIGH;
  }

}

void getButtonState() {
  rflib.transmit("a62RELAYA---");
  Serial.println("Sending status request...");
}

void setup() {
  //1. Power ON
  Serial.begin(9600);
  pinMode(inPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(orangePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(statusPin, OUTPUT);

  rflib.RegisterCallback(myFunc);
  rflib.begin();
  //2. Send a message to the remote relay and wait for the reply and then
  getButtonState();
  previous = state;
}

void loop() {
  int rt;
  int requested_state;

  rflib.process_rf();

  //Read the status of the tactile switch
  reading = digitalRead(inPin);

  //4. When the tactile button is pushed then send a command to the remote relay to switch to the opposite STATUS
  if (reading == LOW && previous == HIGH && millis() - time > debounce) { //debounce is used for noisy switches. The switch needs to be pressed for more than 400ms to trigger
    digitalWrite(greenPin, 0);
    digitalWrite(redPin, 0);
    digitalWrite(orangePin, 1);
    if (state == HIGH) {
      requested_state = LOW;
      Serial.println("HIGH");
      rflib.transmit("a62RELAYAON-");
    }
    else {
      requested_state = HIGH;
      Serial.println("LOW");
      rflib.transmit("a62RELAYAOFF");
    }
    digitalWrite(orangePin, 0);
    time = millis();
  }

  //7. Check that the switch status is the same as the new status we requested
  if (state == requested_state)
    digitalWrite(greenPin, 1);
  else
    digitalWrite(redPin, 1);

  digitalWrite(statusPin, state);
  previous = reading;
}
