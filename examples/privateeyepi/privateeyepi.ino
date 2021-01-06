/*
WIFI Gateway for PrivateEyePi using ESP-32 WIFI chip and Flex RF Module

Tested on a ESP-WROOM-32 Development Board

Wiring:

JemRF Flex Module:
 - The ESP-32 has two serial ports so we are using Serial2 for RF data so that you can use Serial1 for debugging through the Arduino Serial monitor

Flex Pin 15 (TX) -> ESP-32 GPIO16 (RX2)
Flex Pin 16 (TX) -> ESP-32 GPIO17 (TX2)
Flex Pin 01 3V3  -> ESP-32 3V3
Flex Pin 10 GND  -> ESP-32 GND

 - Configure the Flex module as Type 2

Door/Window Sensors:
 - GPIOXX to one side of the alarm/door switch
 - GND to the other side of the alarm/door switch
 
DS18B20:
                                                           0
3 Pins of the DS18B20 flat side facing you with legs down |||
                                                          123
 - GPIO4 - DS18B20 Pin 2 (can be changed in pep.h > #define ONE_WIRE_BUS 4)
 - 3V3   - DS18B20 Pin 3
 - GND   - DS18B20 Pin 1
 - DS18B20 Pin 2 to 4.7k resistor to DS18B20 Pin 3
 
 
*/

#include <rflib.h>
#include <WiFi.h>
#include <rflib.h>
#include <pep.h>

#ifdef RF
RFLIB rflib;
#endif

const char* pep_token = ""; //Enter your PrivateEyePi token between quotes
const char* ssid      = ""; //Enter your WIFI router SSID between quotes
const char* password  = ""; //Enter your WIFI password SSID between quotes
//Enter wired sensors here
//Add ,{xx,'s',0} settings for every wired sensor whee xx is the GPIO number
//GPIO 15 and 19 are configures as follows:
uint8_t wired_sensors[20][3] = {{15,'s',0},{19,'s',0}}; 

//Email alert configuration
const char* smtp_corp="";
const char* smtp_password="";
const char* smtp_user="";
const uint8_t smtp_port=80;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  Serial.println("");
  Serial.println("Connected to the WiFi network");
  
#ifdef RF
  rflib.rf_init();
#endif

  setup_wired_sensors();
 
  }

#ifdef RF
//Monitors the RF message queue and sends data to PrivateEyePi server
void poll_rf_queue(){
  char message[13];
  if (get_from_queue(message)){
    rflib.process_message(message);
    if (rflib.PEPFunction > 0) {
      send_data_to_server(rflib.PEPFunction, rflib.dev_id, rflib.sensordata, "");
    }
  }
}
#endif

void loop() {
 poll_wired_sensors(); 
#ifdef RF
 poll_rf_queue();
#endif
 }


