#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include "Config.h"

#include <RTClib.h>

void setupRTC();
DateTime getCurrentTime();
void printCurrentTime();
void adjustTime(const DateTime& dt);
String getCurrentDateString();
String getRtcTimeString();

int getCurrentYear();
String getCurrentDateStringMDY();
void refreshCurrentTime();
extern RTC_DS3231 rtc;

void dateTimeTicker();


extern DateTime CurrentTime; // Declaration for global access


#endif // RTCMANAGER_H
