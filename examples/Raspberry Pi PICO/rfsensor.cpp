/**
Example for Raspberry Pi PICO platform
Switch an LED on when a specific radio message is received
 */


#include "pico/stdlib.h"
#include <string.h>
#include "..\rflib\rflib.h"

RFLIB rflib;
const uint LED_PIN = 2;

void myFunc(){
    if (strncmp(rflib.message_in, "a08BUTTONOFF",12)==0){
        gpio_put(LED_PIN, 1);
    }
}

int main() {
    stdio_init_all();
    printf("started...");
    rflib.RegisterCallback(myFunc);
    rflib.begin();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    while (1){
        rflib.process_rf();
    }
}
