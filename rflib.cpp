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
* Author     : J. Evans
* Description: JemRF RF Module Driver for Arduino
*/

#include "rflib.h"

extern RFLIB rflib;

#if (ARDUINO_ARCH_ESP32)
TaskHandle_t TaskA;
#endif

uint8_t message_buf[20][13];
uint8_t msgcnt=0;

#define DEBUG true // flag to turn on/off (true/false) debugging
#define Serial if(DEBUG)Serial

#if (!ARDUINO_ARCH_ESP32)
#include <SoftwareSerial.h>
SoftwareSerial Serial2(RX, TX); // RX, TX
#endif

RFLIB:: RFLIB(void)
{

}

void RFLIB::begin(void)
{
  Serial2.begin(9600);
  delay(100);
  Serial2.flush();
  while (Serial2.available()) { //# we have a message arriving
    Serial2.read();
    }
  char_cnt=0;
  got_ack=0;
  got_message=0;
  filter_duplicates=0;
  total_timeout=6100;
  overall_time=0;
  sent_time=0;
  resend_timeout=2000;
}

void rf_gateway_in() { //This function is called every time a message is received through the RF module
  Serial.println("Got RF message...");
  Serial.println(rflib.message_in);
  //Save the RF Message in a message buffer for processing 
  add_to_queue(rflib.message_in); 
}

void RFLIB::rf_init(){
  rflib.RegisterCallback(rf_gateway_in);
  rflib.begin(); 
  init_queue();

#if (ARDUINO_ARCH_ESP32)
  xTaskCreate(
    rf_loop,         // Function that should be called
    "RF_LOOP",       // Name of the task (for debugging)
    8192,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
#endif
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
  strncpy(message_out, message, 12);
  Serial.println("Sending message");
  Serial.println(message);
  Serial2.write(message_out);
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
  timeout=0;
  
  if (sent_time > 0 && !timeout){
    if (millis() - overall_time > total_timeout) { //timeout after n seconds
      Serial.println("Timeout - Total timeout exceeded");
      timeout=true;
      sent_time=0;
      }
    else {
      if (millis() - sent_time > resend_timeout) { //resend after 1.5 seconds if no reply
        Serial.println("Timeout - Re-send timeout exceeded. Re-sending...");
        Serial2.write(message_out); //re-transmit the message
        sent_time = millis();
      }     
    }    
  }
  
  if (char_cnt==12){
    memset(message_in,0x00,13);
  }
  if (Serial2.available()) { //# we have a message arriving
  //while (Serial2.available()) { //# we have a message arriving
    inChar = Serial2.read();
    if (inChar == 'a') {
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
          Serial.println("ACK received");
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
      else{
        Serial.print("Ignoring duplicate RF message:");
        Serial.println(message_in);
      }      
    }
  }
}

char *strip (char *dest, const char *src, uint8_t len)
{
  int i;
  for (i=0;i<len;i++){
    if (src[i]!='-'){
      dest[i]=src[i];
    }
    else {
      dest[i]=' ';      
    }
  }
  dest[i]=0x00;
  return dest;
}

void RFLIB::process_message(char *message){
  float sdata;
  char smessage[13];
  char temp[13], temp2[13];
  memcpy(smessage,message,13); 
  memset(sensordata,0x00,10);  
  memset(description,0x00,15);  
  type=0;
  PEPFunction=0;
  
  strncpy(dev_id, smessage+1 ,2);
  
  if (strncmp(smessage+3,"BUTTONON",8)==0){
    strncpy(sensordata, "0",1);
    PEPFunction=26;
    type=1;
    strncpy(description,"BUTTON",6);
    }

  if (strncmp(smessage+3,"STATEON", 7)==0){
    strncpy(sensordata, "0",1);
    PEPFunction=38;
    type=2;
    strncpy(description,"STATE",5);
    }

  if (strncmp(smessage+3,"STATEOFF",8)==0){
    strncpy(sensordata, "1",1);
    PEPFunction=38;
    type=2;
    strncpy(description,"STATE",5);
    }

  if (strncmp(smessage+3,"BUTTONOFF",9)==0){
    strncpy(sensordata, "1",1);
    PEPFunction=26;
    type=1;
    strncpy(description,"BUTTON",6);
    }

  if (strncmp(smessage+3,"TMPA",4)==0){
    strip(sensordata, smessage+3+4, 5);
    PEPFunction=37;
    type=3;
    strncpy(description,"TMPA",3);
    }

  if (strncmp(smessage+3,"ANAA", 4)==0){
    strip(sensordata, smessage+3+4, 5);    
    PEPFunction=37;
    type=4;
    strncpy(description,"ANAA",4);

  }

  if (strncmp(smessage+3,"ANAB", 4)==0){
    strip(sensordata, smessage+3+4, 5);    
    PEPFunction=37;
    type=10;
    strncpy(description,"ANAB",4);
  }

  if (strncmp(smessage+3,"TMPC",4)==0){
    strip(sensordata, smessage+3+4, 5);    
    PEPFunction=37;
    type=6;
    strncpy(description,"TMPC",4);
    }

  if (strncmp(smessage+3,"TMPB",4)==0){ 
    strip(sensordata, smessage+3+4, 5);    
    PEPFunction=37;
    type=5;
    strncpy(description,"TMPB",4);
    }
                       
  if (strncmp(smessage+3,"HUM",3)==0){
    strip(sensordata, smessage+3+3, 6);    
    PEPFunction=37;
    type=7;
    strncpy(description,"HUM",3);
    }

  if (strncmp(smessage+3,"PA",2)==0){
    strip(sensordata, smessage+3+2, 7);    
    PEPFunction=37;
    type=8;
    strncpy(description,"PA",2);
    }
        
  if (strncmp(smessage+3,"BATT",4)==0){
    strip(sensordata, smessage+3+4, 5);    
    PEPFunction=22;
    type=9;
    strncpy(description,"BATT",4);
    }

  if (strncmp(smessage+3,"RELAYA",6)==0){
    strip(sensordata, smessage+3+6, 3);    
    PEPFunction=0;
    type=11;
    strncpy(description,"RELAYA",5);
    }

  if (strncmp(smessage+3,"RELAYB",6)==0){
    strip(sensordata, smessage+3+4, 3);    
    PEPFunction=0;
    type=12;
    strncpy(description,"RELAYA",5);
    }

  if (strncmp(smessage+3,"HELLO",5)==0){
    strncpy(sensordata, "HELLO",5);    
    PEPFunction=0;
    type=13;
    strncpy(description,"HELLO",5);
    }
 
}

void rf_loop( void * parameter ){
  while (1){
    rflib.filter_duplicates=1;
    rflib.process_rf();  
  }
}

uint8_t get_from_queue(char message[13]){
  uint8_t x;
  for (x=0;x<MESSAGE_BUFFER_SIZE;x++){
    if (message_buf[x][0]!=0x00){ //if there is an RF message in the queue then do an HTTP call to the PrivateEyePi server
      Serial.println("Processing RF message");
      memcpy(message,message_buf[x],13);
      memset(message_buf[x],0x00,13); //take the message off the queue
      Serial.println(message);
      return(1);
    }
  }  
  return(0);
}

void init_queue(){
  //Clear the message buffer
  for (int x=0;x<MESSAGE_BUFFER_SIZE;x++){
    memset(message_buf[x],0x00,13);
    }
}

void add_to_queue(char *message){
  if (msgcnt>=MESSAGE_BUFFER_SIZE-1) msgcnt=0;
  memcpy(message_buf[msgcnt++], message, 13);
}   

 
