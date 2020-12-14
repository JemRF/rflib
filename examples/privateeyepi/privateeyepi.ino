/*
WIFI Gateway for PrivateEyePi using ESP-32 WIFI chip and Flex RF Module

Tested on a ESP-WROOM-32 Development Board

Wiring:
Flex Pin 15 (TX) -> ESP-32 GPIO16 (RX2)
Flex Pin 16 (TX) -> ESP-32 GPIO17 (TX2)
Flex Pin 01 3V3  -> ESP-32 3V3
Flex Pin 10 GND  -> ESP-32 GND

Configure the Flex module as Type 2

Notes:
 - The ESP-32 is duel core so this code utilizes both cores, one core for receiving RF messages through RX2 and the other core for calling HTTP GET requests to PrivateEyePi. The will ensure no radio messages are missed while time is spent doing the HTTP requests.

 - The ESP-32 has to serial ports so we are using Serial2 so that you can use Serial1 for debugging through the Arduino Serial monitor
 
*/

#include <rflib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define MESSAGE_BUFFER_SIZE 20

RFLIB rflib;
TaskHandle_t TaskA;

const char* ssid = "";       //Enter your WIFI router SSID between quotes
const char* password =  "";  //Enter your WIFI password SSID between quotes
const char* pep_token =  ""; //Enter your PrivateEyePi token between quotes

uint8_t message_buf[20][13];
uint8_t msgcnt=0;

void myFunc() {

  Serial.println("Got message...");
  Serial.println(rflib.message_in);
  if (msgcnt>=MESSAGE_BUFFER_SIZE-1) msgcnt=0;
  //Save the RF Message in a message buffer for processing 
  memcpy(message_buf[msgcnt++], rflib.message_in, 13);
}

void TransmitToWifi(){
  uint8_t x;
  while (1){
    for (x=0;x<MESSAGE_BUFFER_SIZE;x++){
      if (message_buf[x][0]!=0x00){ //if there is an RF message in the queue then do an HTTP call to the PrivateEyePi server
        Serial.println("Processing message");
        Serial.println((char *)message_buf[x]); //process the message
        rflib.process_message((char *)message_buf[x]);
        memset(message_buf[x],0x00,13); //take the message off the queue
        if (rflib.PEPFunction > 0) {
          http_request(rflib.PEPFunction, rflib.sensordata, rflib.dev_id);
        }
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  rflib.RegisterCallback(myFunc);
  rflib.begin();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  Serial.println("");
  Serial.println("Connected to the WiFi network");

  xTaskCreatePinnedToCore(
    rf_loop,                /* pvTaskCode */
    "Workload1",            /* pcName */
    1000,                   /* usStackDepth */
    NULL,                   /* pvParameters */
    1,                      /* uxPriority */
    &TaskA,                 /* pxCreatedTask */
    1);                     /* xCoreID */
  
  //Clear the message buffer
  for (int x=0;x<MESSAGE_BUFFER_SIZE;x++){
    memset(message_buf[x],0x00,13);
    }

  }


void http_request(int PEP_Function, String sensordata, String dev_id) {
    String searchString;
    String url;
    int rt;
    HTTPClient http;
      
    url = "https://www.privateeyepi.com/alarmhostr.php?token="+(String)pep_token+"&function="+String(PEP_Function)+"&opcode0=" + dev_id+"&opcode1=" + sensordata;
    Serial.println("Requesting URL: ");
    Serial.println(url);
    http.begin(url); //Specify the URL and certificate
    int httpCode = http.GET();             

    Serial.println(httpCode);
    if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
    return;    
  }

void rf_loop( void * parameter ){
  while (1){
    rflib.filter_duplicates=1;
    rflib.process_rf();  
  }
}

void loop() {
  //Important to call profess_rf() frequently and have no delays in this loop
 TransmitToWifi(); 
}
