#ifndef TASKS_h
#define TASKS_h

#include <Arduino.h>

#ifdef USE_MULTI_THREAD
void main_menu_task(void *pvParameters);
void keypad_task(void *pvParameters);
void tft1_task(void *pvParameters);
void obd_task(void *pvParameters);
#endif

#endif