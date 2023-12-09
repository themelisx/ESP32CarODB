#ifndef TASKS_h
#define TASKS_h

#include <Arduino.h>

void main_menu_task(void *pvParameters);
void keypad_task(void *pvParameters);
void tft1_task(void *pvParameters);
void obd_task(void *pvParameters);

#endif