/* simple_tx.ino
 * 
 * Transmit a HELLO message every 5 seconds
 * 
 * See also:
 *  - simple_rflib_tx example that uses the rflib library for transmitting RF messages. 
 * 
 * For use with radio modules from www.jemrf.com
 */

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

void setup() {
 mySerial.begin(9600);
}

void loop() {
  mySerial.write("a03HELLO----");
  delay(5000);
}
