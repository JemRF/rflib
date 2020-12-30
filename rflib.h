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
*********************************************************************************
*
* Module     : rflib.h
* Author     : Jonathan Evans
* Description: JEMRF RF Driver for Arduino
*
********************************************************************************
*/

#ifndef rflib_h
#define rflib_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define RX 2
#define TX 3
#define RESEND_TIMEOUT 1500
#define MESSAGE_BUFFER_SIZE 20

extern uint8_t message_buf[20][13];
extern uint8_t msgcnt;

//RFLIB
class RFLIB
{
  
  using InputEvent = void (*)(void); 
  
private:
  int char_cnt;
  unsigned long resend_timeout;
  unsigned long sent_time;
  InputEvent Event;
  char prev_message[12];
  int callback_registered=false;
  int retries_ind = false;
  
public: 
  void RegisterCallback(InputEvent InEvent)
  {
      Event = InEvent;
      callback_registered=true;
  }

  unsigned long overall_time;
  unsigned long total_timeout;
  int timeout;
  int got_ack;
  int filter_duplicates;
  char message_in[13];
  char message_out[13];
  char sensordata[10];
  int PEPFunction;
  int type;
  char description[15];
  char dev_id[2];
  int got_message;
  RFLIB(void);
  void begin(void);
  void process_rf(void);
  void transmit(char * message);
  void transmit(char * message, int retries);
  void process_message(char *message);
  void rf_init();
};

void init_queue();
void add_to_queue(char *message);
uint8_t get_from_queue(char message[13]);
void rf_loop( void * parameter );

#endif