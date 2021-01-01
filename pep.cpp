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
* Description: PrivateEyePi Interface for ESP32
*/
#if (ARDUINO_ARCH_ESP32)
#include "pep.h"

#define DEBUG true // flag to turn on/off (true/false) debugging
#define Serial if(DEBUG)Serial

#ifdef DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress; 
long DS18B20_timer;
#endif

extern const char* pep_token;
extern const char* ssid;
extern const char* password;
extern uint8_t wired_sensors[20][3];
extern const char* smtp_corp;
extern const char* smtp_password;
extern const char* smtp_user;

void tone(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel)
{
  if (ledcRead(channel)) {
      log_e("Tone channel %d is already in use", ledcRead(channel));
      return;
  }
  ledcAttachPin(pin, channel);
  ledcWriteTone(channel, frequency);
  if (duration) {
      delay(duration);
      noTone(pin, channel);
  }    
}

void noTone(uint8_t pin, uint8_t channel)
{
    ledcDetachPin(pin);
    ledcWrite(channel, 0);
}

uint8_t check_for_siren_deactivation(uint8_t zone[50]){
// Routine to fetch the location and zone descriptions from the server 
  int rt;
  char buf[255];
  
  rt=http_request(16, String((char *)zone), "","", buf);  
  if (strncmp(buf, "FALSE",5)==0){
      return(1);
      }
  return(0);
}

void chime(){
#ifndef CHIME_PIN
return;
#endif
 
   tone(CHIME_PIN, 1300, 0, 0); 
   delay(500);
   noTone(CHIME_PIN, 0);
   
   tone(CHIME_PIN, 1200, 0, 0); 
   delay(1000);
   noTone(CHIME_PIN, 0);
   
   tone(CHIME_PIN, 1300, 0, 0); 
   delay(500);
   noTone(CHIME_PIN, 0);
   
   tone(CHIME_PIN, 1200, 0, 0); 
   delay(1000);
   noTone(CHIME_PIN, 0);
}

void sound_siren(uint8_t zone[50]){
#ifndef SIREN_PIN
return;
#endif

#ifdef SIREN_DELAY

  for (int x=0;x<SIREN_DELAY;x++){
     tone(SIREN_PIN, 1000, 0, 0); //E
     delay(800);
     noTone(SIREN_PIN, 0);
     delay(200);
  }
#endif
       
#ifdef SIREN_DURATION
  for (int x=0;x<SIREN_DURATION;x++){
    tone(SIREN_PIN, 1000, 0, 0); //E
    delay(1000);
    if (check_for_siren_deactivation(zone)){
      noTone(SIREN_PIN, 0);
      return;
    }
  }
  noTone(SIREN_PIN, 0);
#endif
  return;
}

uint8_t process_actions(char buf[255], uint8_t length){
  uint8_t rt=99;
  uint8_t x=0, y=0;
  uint8_t pep_commands[20][10];
  
  for (int i=0;i<length;i++){
    if (buf[i]!=','){
      pep_commands[x][y++]=buf[i];
    }
    else {
      x++;
      y=0;
    }
  }
  
  for (int i=0;i<x;i++){  
    if (strncmp((char *)pep_commands[i], "/SIREN", 6)==0){
      sound_siren(pep_commands[i+2]);
      //FYI
      //pep_command[i+1] = SensorID
      //pep_command[i+2] = ZoneID
      rt=2;
    }
    if (strncmp((char *)pep_commands[i], "/CHIME", 6)==0){
      chime();
      //FYI
      //pep_command[i+1] = SensorID
      //pep_command[i+2] = ZoneID
      rt=4;
    }
    if (strncmp((char *)pep_commands[i], "/EMAIL", 5)==0){
      Serial.println("EMAIL!!");
      rt=3;
    } 
    if (strncmp((char *)pep_commands[i], "/TRUE", 5)==0){
      rt=1;
    } 
    if (strncmp((char *)pep_commands[i], "/FALSE", 5)==0){
      rt=0;
    }
  }    
  return(rt);
}

void send_data_to_server(int PEP_Function, String opcode0, String opcode1, String opcode2){
  uint8_t payload_length;
  char buf[255];
  
  memset(buf,0x00,255);
  payload_length = http_request(PEP_Function, opcode0, opcode1, opcode2, buf);
  if (payload_length){
    process_actions(buf, payload_length);
  }
}

uint8_t http_request(int PEP_Function, String opcode0, String opcode1, String opcode2, char buf[255]) {
    String searchString;
    String url;
    int rt=0;
    HTTPClient http;
      
    url = "https://www.privateeyepi.com/alarmhostr.php?token="+(String)pep_token+"&function="+String(PEP_Function)+"&opcode0=" + opcode0+"&opcode1=" + opcode1+"&opcode2=" + opcode2;
    Serial.println("Requesting URL: ");
    Serial.println(url);
    http.begin(url); //Specify the URL and certificate
    int httpCode = http.GET();             

    Serial.println(httpCode);
    if (httpCode > 0) { //Check for the returning code
        if (httpCode==200){
          String payload = http.getString();
          Serial.println(payload);
          if (payload.length()>0){
            if (payload.length()>255) rt=255;
            else rt=payload.length();
            payload.toCharArray(buf, rt+1);
          }          
        }
      }
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
    return(rt);    
  }

byte eRcv(WiFiClient * client)
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client->available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      Serial.println("10 sec \r\nTimeout");
      return 0;
    }
  }

  respCode = client->peek();

  while (client->available())
  {
    thisByte = client->read();
    //Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    client->println(F("QUIT"));

    while (!client->available()) {
      delay(1);
      loopCount++;

      // if nothing received for 10 seconds, timeout
      if (loopCount > 10000) {
        Serial.println("efail \r\nTimeout");
        return 1;
      }
    }

    while (client->available())
    {
      thisByte = client->read();
      //Serial.write(thisByte);
    }


    Serial.println("disconnected");

    return 0;
  }

  return 1;
}

void compileEmailAlert(String sensor_no, String rule_id) {
  unsigned int payload_length, x, y;
  String searchString;
  String emailstr;
  String messagestr;
  String temp;
  char buf[255];

  searchString = "/EMAIL";

  payload_length = http_request(35, rule_id, sensor_no,"", buf);
  if (String(buf).indexOf(searchString) > 0) {
    return;
  }
  
  Serial.println("Received Alert details");
  Serial.println(buf);
  messagestr = String(buf).substring(7,String(buf).length());
  Serial.println(messagestr);

  searchString = "/ADDRESS";
  payload_length = http_request(36, "","","",buf);
  if (String(buf).indexOf(searchString) > 0) {
    return;
  }
  temp=String(buf);
  temp.trim();
  x = temp.indexOf("/ADDRESS") + 9;
  y = temp.length()-2;
  emailstr = temp.substring(x, y);
  Serial.println("Received Address details");
  Serial.println(emailstr);

  sendEmail(emailstr, messagestr);
}

unsigned int sendEmail(String EmailTo, String EmailMessage) {
  
  String searchString, tempstr;
  int prev, x;
  char myIpString[24];
  IPAddress ip;
  WiFiClient client;
  
  ip = WiFi.localIP();
  sprintf(myIpString, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  if (client.connect( smtp_corp, 80) == 1) {
    Serial.println("connected email server");
  } else {
    Serial.println("connection failed email server");
    return 0;
  }
  if (!eRcv(&client)) {
    return 0 ;
  }

  Serial.println("Sending ehlo");
  client.println("EHLO " + (String)myIpString);
  if (!eRcv(&client)) {
    Serial.println("ehlo");
    return 0 ;
  }

  Serial.println("Sending auth login");
  client.println("auth login");
  if (!eRcv(&client)) {
    Serial.println("auth");
    return 0 ;
  }

  Serial.println("Sending User");
  // Change to your base64 encoded user
  client.println(smtp_user);//
  if (!eRcv(&client)) {
    Serial.println("user");
    return 0 ;
  }

  Serial.println("Sending Password");
  // change to your base64 encoded password
  client.println(smtp_password);//

  if (!eRcv(&client)) {
    Serial.println("ehlo");
    return 0;
  }

  // change to your email address (sender)
  Serial.println("Sending From");
  client.println("MAIL From: <alerts.privateeyepi@gmail.com>");
  if (!eRcv(&client)) {
    Serial.println("email");
    return 0 ;
  }

  // change to recipient address
  Serial.println(EmailTo);
  prev = 0;
  for (x = 0; x <= EmailTo.length(); x++) {
    if ((EmailTo.substring(x + 1, x + 2) == "," || x == EmailTo.length()) && x > prev) {
      Serial.println("Sending To");
      Serial.println(EmailTo.substring(prev, x + 1));
      client.println("RCPT To: " + EmailTo.substring(prev, x + 1));
      if (!eRcv(&client)) {
        Serial.println("email");
        return 0 ;
      }
      prev = x + 3;
    }
  }

  Serial.println("Sending DATA");
  client.println("DATA");
  if (!eRcv(&client)) {
    Serial.println("email");
    return 0 ;
  }

  Serial.println("Sending email");

  // change to recipient address
  client.println("To: <" + EmailTo + ">");

  // change to your address
  client.println("From: Your Alarm <alerts.privateeyepi@gmail.com>");

  client.println("Subject: IOT Sensor Alert\r\n");

  client.println(EmailMessage);

  client.println(".");

  if (!eRcv(&client)) {
    Serial.println("after sending");
    return 0 ;
  }

  Serial.println("Sending QUIT");
  client.println("QUIT");

  if (!eRcv(&client)) {
    Serial.println("after Quit");
    return 0 ;
  }


  Serial.println("disconnected");

  return 1;
}

uint8_t debounce (uint8_t pin, uint8_t value){
  long m;
  uint8_t d;
  m = millis();
  d=false;
  while (digitalRead(pin)==value && !d){
    if (millis()-m>200){ //switch must be stable to 100ms before confirming the change
      d=true;
    }
  }
  return(d);
}

void poll_switch_sensors(uint8_t x){
  long m;
  uint8_t d;
  
  d=digitalRead(wired_sensors[x][0]);
  if (wired_sensors[x][2]!=d){
    if (debounce(wired_sensors[x][0], d)){
      wired_sensors[x][2]=d;
      Serial.print(" Sensor ");
      Serial.print(wired_sensors[x][0]);
      Serial.print(" Value ");
      Serial.println(wired_sensors[x][2]);
      send_data_to_server(26, String(wired_sensors[x][0]), String(wired_sensors[x][2]), "");        
    }
  }
}

void poll_ds18b20_sensors(){
#ifdef DS18B20  
  float tempC;
  uint8_t cnt;
  Serial.println("Looking for DS18B20 sensor");

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  
  Serial.print("Found : ");
  Serial.print(numberOfDevices);
  Serial.println(" DS18B20 sensors");
  
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      // Output the device ID
      Serial.print("Temperature for device address: ");
      Serial.println(String(tempDeviceAddress[7]));
      // Print the data
      
      tempC = sensors.getTempC(tempDeviceAddress);
      cnt=5;
      while ((tempC==-127 || tempC==85) && cnt>0){
        Serial.println("Error reading sensor - retrying");
        delay(500);
        sensors.requestTemperatures(); // Send the command to get temperatures
        tempC = sensors.getTempC(tempDeviceAddress);
      }
      
      if ((tempC==-127 || tempC==85)){
        Serial.println("Error reading sensor - abort and try again later");
      }
      else{
        Serial.print("Temp C: ");
        Serial.print(tempC);
        Serial.print(" Temp F: ");
        Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
        send_data_to_server(14, String(tempC), "0", String(tempDeviceAddress[7]));              
        }
      }
    }
#endif  
}


void poll_wired_sensors(){
  uint8_t num_sensors;
  uint8_t x;
  num_sensors=sizeof(wired_sensors)/sizeof(wired_sensors[0]);
  
  for (x=0;x<num_sensors;x++){
    if (wired_sensors[x][1]=='s'){
      poll_switch_sensors(x);
    }
  }
                               
#ifdef DS18B20 
  if (millis() - DS18B20_timer > DS18B20_INTERVAL){
    poll_ds18b20_sensors();
    DS18B20_timer = millis();
  }  
#endif
}

void setup_wired_sensors(){
  uint8_t num_sensors;
  uint8_t x,d;
  
  num_sensors=sizeof(wired_sensors)/sizeof(wired_sensors[0]);
  for (x=0;x<num_sensors;x++){
      if (wired_sensors[x][1]=='s'){ //switch sensor
        pinMode(wired_sensors[x][0], INPUT_PULLUP);
        d=digitalRead(wired_sensors[x][0]);
        Serial.print("Wired sensor ");
        Serial.print(String(wired_sensors[x][0]));
        Serial.print("=");
        if (debounce(wired_sensors[x][0], d)){
          wired_sensors[x][2] = d;        
          Serial.println(d);
        }
        else {
          wired_sensors[x][2] = !d;
          Serial.println(!d);
        }
      }     
    }
#ifdef DS18B20 
  DS18B20_timer = millis();
  Serial.println("Starting DS18B20");
  sensors.begin();
#endif
  }
#endif


