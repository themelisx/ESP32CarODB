#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_RTC_CLOCK

#include "myRTC.h"
#include "vars.h"

ThreeWire myWire(12, 13, 14);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

RTC_DS1302::RTC_DS1302() {

}

void RTC_DS1302::start() {

  /*
  debug.print(DEBUG_LEVEL_DEBUG, F("compiled: "));
  debug.print(DEBUG_LEVEL_DEBUG, __DATE__);
  debug.println(DEBUG_LEVEL_DEBUG, __TIME__);
  */

  debug.println(DEBUG_LEVEL_INFO, F("Starting RTC..."));
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //printDateTime(compiled);
  //debug.println();

  if (!Rtc.IsDateTimeValid()) {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    debug.println(DEBUG_LEVEL_DEBUG, F("RTC lost confidence in the DateTime!"));
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    debug.println(DEBUG_LEVEL_DEBUG, F("RTC was write protected, enabling writing now"));
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    debug.println(DEBUG_LEVEL_DEBUG, F("RTC was not actively running, starting now"));
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    debug.println(DEBUG_LEVEL_DEBUG, F("RTC is older than compile time!  (Updating DateTime)"));
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    debug.println(DEBUG_LEVEL_DEBUG, F("RTC is newer than compile time. (this is expected)"));
  } else if (now == compiled) {
    debug.println(DEBUG_LEVEL_DEBUG, F("RTC is the same as compile time! (not expected but all is fine)"));
  }
  debug.println(DEBUG_LEVEL_INFO, F("Done"));
}

char *RTC_DS1302::getFormattedDate() {
  sprintf(dateString, "%02d/%02d/%02d", dt.Day(), dt.Month(), dt.Year() % 100);
  return dateString;
}

char *RTC_DS1302::getFormattedTime() {
  sprintf(timeString, "%02d:%02d", dt.Hour(), dt.Minute());
  return timeString;
}


#endif