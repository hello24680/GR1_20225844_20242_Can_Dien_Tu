#ifndef SLEEP_PROCESS_H
#define SLEEP_PROCESS_H

#include <Arduino.h>
#include "esp32-hal-gpio.h"

void wakeup_Init(); // Cấu hình đánh thức
void enterSleep();   // Vào deep sleep

#endif