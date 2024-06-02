#include "TimeSync.h"
#include "Logging.h"
#include "RTCManager.h"
#include "Config.h"

#include <Wire.h>
#include <TimeLib.h>
#include <RTClib.h>
#include "time.h"
#include <Ticker.h>

Ticker ntpRetryTicker;



extern DateTime CurrentTime;

bool needToSyncTime = true; // Initially, we need to synchronize time
bool needNtpSync = true; // Flag to indicate if NTP sync is needed
unsigned long lastNtpUpdateAttempt = 0;
const unsigned long ntpRetryInterval = 600000; // 10 minutes in milliseconds
bool isNtpSyncDue = true;



// if time sync has not been done today it will sync at 3AM. this is called every second from task TaskcheckAndSyncTime
void checkAndSyncTime() {
    DateTime now = CurrentTime; // Assume CurrentTime is up to date
    // Check if it's 3 AM and if the last sync was not today
    static DateTime lastSyncDate;
    if (now.hour() == 3 && now.minute() == 00 && (lastSyncDate.day() != now.day() || lastSyncDate.month() != now.month() || lastSyncDate.year() != now.year())) {
        // Trigger NTP sync
        Serial.print("3AM, calling initNTP to initiate NTP time sync");
        initNTP();
        lastSyncDate = now; // Update last sync date
        
    }
}

 
// this is called in setup to connect to the NTP server
void initNTP() {
    Serial.print("Starting NTP time sync ");
    configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov", "MST7MDT");
    tryNtpUpdate(); // Attempt to update NTP time immediately
}


// this NTP>RTC sync is called in setup() and if it fails the ticker will retry in 10 minutes. this is the current function to accomplish time sync.
void tryNtpUpdate() {
    Serial.println ();
    Serial.print("Attempting NTP time sync... ");
    Serial.println ();
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10000)) { // Try to get the time with a 10-second timeout
        Serial.print("NTP Time synchronize Successful ");
        // Print the synchronized time
        Serial.printf("NTP Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                      timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                      timeinfo.tm_mday, timeinfo.tm_hour,
                      timeinfo.tm_min, timeinfo.tm_sec);
        // Adjust RTC with NTP time
        rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                            timeinfo.tm_mday, timeinfo.tm_hour,
                            timeinfo.tm_min, timeinfo.tm_sec));
        Serial.print("RTC adjusted to NTP time. ");
        // Update CurrentTime with the new RTC time
        CurrentTime = rtc.now();
        printCurrentRtcTime(); // Display the current RTC time
        ntpRetryTicker.detach(); // Stop retrying since we've successfully synchronized time
    } else {
        Serial.println ();
        Serial.print(" NTP sync failed, will retry in 10 minutes... ");
        Serial.println ();
        CurrentTime = rtc.now();
        printCurrentRtcTime(); // Display the current RTC time
        ntpRetryTicker.once(600, tryNtpUpdate); // Retry after 10 minutes
    }
}

// called in tryNtpUpdate() function above
// Updated to use the CurrentTime variable
void printCurrentRtcTime() {
    Serial.print(" Current time: ");
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
     

    void initializeTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println ();
        Serial.print(" Failed to obtain time ");
        Serial.println ();
    } else {
        // Optionally, log or process the obtained time
        // Serial.print("Time obtained successfully");
    }
}
