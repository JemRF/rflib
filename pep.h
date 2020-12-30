#ifndef pep_h
#define pep_h

#include "Arduino.h"
#include <HTTPClient.h>

#define TONE_CHANNEL 15
#define SIREN_PIN 2
#define SIREN_DURATION 120
#define SIREN_DELAY  30
#define CHIME_PIN 2
#define RF
#define DS18B20
#define DS18B20_INTERVAL 300000 //5 minutes

extern const char* ssid;
extern const char* password;
extern const char* pep_token;
extern uint8_t wired_sensors[20][3]; 

void tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0, uint8_t channel = TONE_CHANNEL);
void noTone(uint8_t pin, uint8_t channel = TONE_CHANNEL);
uint8_t get_from_queue(char message[13]);
void add_to_queue(char *message);
void init_queue();
uint8_t http_request(int PEP_Function, String opcode0, String opcode1, String opcode2, char buf[255]);
void send_data_to_server(int PEP_Function, String opcode0, String opcode1, String opcode2);
void rf_init();
unsigned int sendEmail(String EmailTo, String EmailMessage, char smtpcorp[50], int port, String username, String password);
void compileEmailAlert(String SensorNo, String RuleId);
void setup_wired_sensors();
void poll_wired_sensors();
void rf_gateway_in();
#endif