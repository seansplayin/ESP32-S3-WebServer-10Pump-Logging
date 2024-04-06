#include "RTCManager.h"
#include "Config.h" // Include this to use rtc, pinSDA, and pinSCL
#include "Logging.h"
#include "WebServerManager.h"
#include "TimeSync.h"

#include <RTClib.h>
#include <Wire.h>
#include <Ticker.h>

extern RTC_DS3231 rtc;
extern const int pinSDA;
extern const int pinSCL;
extern const int sqwPin;

DateTime CurrentTime; // This holds the current time updated periodically

Ticker dateTimeTickerObject; // This is the Ticker object

void setupRTC() {
    Wire.begin(pinSDA, pinSCL); // Initialize I2C
    if (!rtc.begin()) {
        Serial.print("Couldn't find RTC");
        while (1);
    }
    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
}

DateTime getCurrentTime() {
    return CurrentTime;
}

void printCurrentTime() {
    Serial.print("Current time: ");
    Serial.print(CurrentTime.year(), DEC);
    Serial.print('/');
    Serial.print(CurrentTime.month(), DEC);
    Serial.print('/');
    Serial.print(CurrentTime.day(), DEC);
    Serial.print(" ");
    Serial.print(CurrentTime.hour(), DEC);
    Serial.print(':');
    Serial.print(CurrentTime.minute(), DEC);
    Serial.print(':');
    Serial.println(CurrentTime.second(), DEC);
}

void adjustTime(const DateTime& dt) {
    rtc.adjust(dt);
}

String getCurrentDateString() {
    char dateStr[11]; // Buffer for YYYY-MM-DD format
    sprintf(dateStr, "%04d-%02d-%02d", CurrentTime.year(), CurrentTime.month(), CurrentTime.day());
    return String(dateStr);
}

// 03-17-24 confirmed this function properly reports date/time and is used in the pump on and off logs     
   String getRtcTimeString() {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", 
             CurrentTime.year(), CurrentTime.month(), CurrentTime.day(), 
             CurrentTime.hour(), CurrentTime.minute(), CurrentTime.second());
    return String(buffer);
}

// Function to get the current year as an integer
int getCurrentYear() {
    return CurrentTime.year();
}

String getCurrentDateStringMDY() {
    char dateStr[11]; // Enough to hold MM-DD-YYYY\0
    sprintf(dateStr, "%02d-%02d-%04d", CurrentTime.month(), CurrentTime.day(), CurrentTime.year());
    return String(dateStr);
}

void broadcastDateTime() {
    char buffer[32];
    // Format the current date and time into the buffer
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
            CurrentTime.year(), CurrentTime.month(), CurrentTime.day(), 
            CurrentTime.hour(), CurrentTime.minute(), CurrentTime.second());
    // Use ws to broadcast the buffer's content to all connected WebSocket clients
    ws.textAll(buffer);
}

void refreshCurrentTime() {
    CurrentTime = rtc.now(); // Refresh the global CurrentTime variable
    broadcastDateTime(); // formats time and then broadcasts through ws (websocket)
    checkTimeAndAct(); // located in Logging.cpp
    }

void dateTimeTicker() {
    dateTimeTickerObject.attach(1, refreshCurrentTime); // Use the Ticker object here
}
