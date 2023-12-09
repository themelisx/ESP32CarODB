#include <Arduino.h>

#include "settings.h"
#include "defines.h"
#include "vars.h"

#include "debug.h"

Settings::Settings() {

}

void Settings::load() {

}

void Settings::save() {

}

void Settings::setDefaults() {
    debugLevel = DEBUG_LEVEL_INFO;
}
