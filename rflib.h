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

//#define DEBUG
#define RX 2
#define TX 3
#define RESEND_TIMEOUT 1500

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
  int got_message;
  int id_match;
  RFLIB(void);
  int request_reply(char *message, int timeout);
  void begin(void);
  void RFLIB::process_rf(void);
  void RFLIB::transmit(char * message);
  void RFLIB::transmit(char * message, int retries);
};

#endif