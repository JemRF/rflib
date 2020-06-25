/*
* The MIT License (MIT)
*
* Copyright (c) JEMRF
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
********************************************************************************
*
* Module     : cpp
* Author     : Jonathan Evans
* Description: JemRF RF Module Driver for Arduino
*/

#include "rflib.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(RX, TX); // RX, TX

RFLIB:: RFLIB(void)
{

}

void d_print(char * message){
#ifdef DEBUG
  Serial.println(message);
#endif
}

void RFLIB::begin(void)
{
  mySerial.begin(9600);
  delay(100);
  char_cnt=0;
  got_ack=0;
  got_message=0;
  filter_duplicates=0;
  total_timeout=6100;
  overall_time=0;
  sent_time=0;
  resend_timeout=2000;
}

void RFLIB::transmit(char * message, int retries){
  overall_time = (total_timeout * retries) + 100;
  retries_ind=true;
  transmit(message);
  }

void RFLIB::transmit(char * message){
  delay(100);
  got_ack=0;
  got_message=0;
  id_match=0;
  strncpy(message_out, message, 12);
  d_print("Sending message");
  d_print(message);
  mySerial.write(message_out);
  timeout=false;
  if (retries_ind){
    overall_time = millis();
    sent_time = millis();
    retries_ind=false;
  }
}

void RFLIB::process_rf(void){
  char inChar;
  
  got_ack=0;
  got_message=0;
  id_match=0;
  timeout=0;
  
  if (sent_time > 0 && !timeout){
    if (millis() - overall_time > total_timeout) { //timeout after n seconds
      d_print("Timeout - Total timeout exceeded");
      timeout=true;
      sent_time=0;
      }
    else {
      if (millis() - sent_time > resend_timeout) { //resend after 1.5 seconds if no reply
        d_print("Timeout - Re-send timeout exceeded. Re-sending...");
        mySerial.write(message_out); //re-transmit the message
        sent_time = millis();
      }     
    }    
  }
  
  if (char_cnt==12){
    memset(message_in,0x00,13);
  }
  if (mySerial.available()) { //# we have a message arriving
  //while (mySerial.available()) { //# we have a message arriving
    inChar = mySerial.read();
    if (inChar == 'a') {
      d_print("Message received");
      char_cnt = 0;
    }
    if (char_cnt >= 0 && char_cnt < 12) {
      message_in[char_cnt]=inChar;
    }
    char_cnt++;
    if (char_cnt == 12) {
      //Filter duplicates
      if (strncmp(message_in, prev_message,12)!=0 || !filter_duplicates){
        strncpy(prev_message, message_in,12);
        if (strncmp(message_in, message_out,12)==0) {
          d_print("ACK received");
          got_ack=1;
        }
        if (strncmp(message_in+1, message_out+1,2)==0) {  //ID's are the same
          sent_time=0;
        } 
        got_message = 1;
        memset(message_out,0x00,13);
        if (callback_registered)
          Event();  
      }      
    }
  }
}
