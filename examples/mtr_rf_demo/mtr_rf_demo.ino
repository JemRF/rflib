/* mtr_rf_demo.ino

  A multi-tasking example how to receive RF messages and handle other processing tasks
  at the same time. This example prints all in-coming RF mnessages to the serial port,
  which at the same time printing a counter to the serial port every 1 second.

  This is not true multiprocessing because the ATMEGA chips only have ont processor. This
  example uses a timer interrupt that is guaranteed to trigger every millisecond. This gives
  us the ability to ensure rflib.process_rf() is called frequently, ensuring we do not miss any
  RF messages. Both the hardware serial port and SoftwareSerial library have a 64k buffer which
  can buffer 5 RF messages (12x5=60 bytes). Therefore if we do not call rflib.process_rf() frequently
  enough we could loose RF messages when the serial port buffer overflows. With the timer interrupt
  handling RF messages you are free to put any code in the loop() section and even use delay
  statements or have time consuming loops that would otherwise cause loss of RF messages.

  See also:
   - automation1.ino that uses another multi tasking technique (round robin processing)
     to achieve the same result. 

  For use with radio modules from www.jemrf.com

*/
#include <rflib.h>

RFLIB rflib;

unsigned long last_update = 0;
int x = 0;

//A callback function that is called every time there is a message is received
//You can insert your code in this function to process the incoming messages
void myFunc() {
  Serial.println("Got message...");
  Serial.println(rflib.message_in);
}

void setup() {
  Serial.begin(9600);

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the below function
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

  //Set up your callback function
  rflib.RegisterCallback(myFunc);
  rflib.begin();
}

// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect)
{
  if (millis() - last_update > 10) {
    rflib.process_rf();
    last_update = millis();
  }

}

void loop() {
  Serial.println("Count: " + String(x++));
  delay(1000);
}
