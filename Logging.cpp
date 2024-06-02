#include "Logging.h"
#include "TimeSync.h"
#include "RTCManager.h"
#include "PumpManager.h"
#include "Config.h"
#include <map>


//#include <SPIFFS.h>
#include <LittleFS.h>
#include <FS.h> // Include the SPIFFS library
#include <RTClib.h>

extern DateTime CurrentTime;

extern RTC_DS3231 rtc;  // Use the external 'rtc' declaration


volatile bool Elapsed_Day = false;


// Helper function to parse datetime strings
DateTime parseDateTime(String datetimeStr);
// Assuming parseDateTimeFromLog() and rtc.now() are properly defined elsewhere
DateTime parseDateTimeFromLog(const String& datetimeStr);

        
        
        void logPumpEvent(int pumpIndex, const String& event) {
        // Construct the filename using the pump index
        String filename = "/pump" + String(pumpIndex + 1) + "_Log.txt"; // Assuming pumpIndex is 0-based

        // Open the file in append mode
        File logFile = LittleFS.open(filename, "a");
        if (!logFile) {
        Serial.println("Failed to open " + filename + " for writing");
        return;
    }

        // Get the current timestamp from the RTC
        String timestamp = getRtcTimeString();

        // Construct the log entry
        String logEntry = event + " " + timestamp;

        // Write the log entry to the file
        logFile.println(logEntry);

        // Close the file
        logFile.close();

        Serial.println("Logged " + event + " for Pump " + String(pumpIndex + 1));
    }    

// I don't believe this is used anylonger since setPumpState() was updated to use logPumpEvent();
    void startPump(int pumpIndex) {
        // Log the start event for the pump
        String logMessage = "Pump " + String(pumpIndex + 1) + " START";
        logPumpEvent(pumpIndex, "START"); // This function should create a log entry with the event

        // Write to Serial Monitor
        Serial.println(logMessage);
    }
// I don't believe this is used anylonger since setPumpState() was updated to use logPumpEvent();
    void stopPump(int pumpIndex) {
        // Log the stop event for the pump
        String logMessage = "Pump " + String(pumpIndex + 1) + " STOP";
        logPumpEvent(pumpIndex, "STOP"); // This function should create a log entry with the event

        // Write to Serial Monitor
        Serial.println(logMessage);
    }


//********List files in the LittleFS********

void listAllFiles() {
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  
  Serial.println("Files stored in LittleFS:");
  while(file){
      Serial.println(file.name());
      file = root.openNextFile();
  }
}

//********Read Files in the LittleFS********

void readAndPrintLogFile(const String& filename) {
    String fullPath = "/" + filename; // Ensure it starts with a '/'
    File logFile = LittleFS.open(fullPath, "r");
    if (!logFile) {
        Serial.println("Failed to open " + fullPath + " for reading");
        return;
    }

    Serial.println("Contents of " + fullPath + ":");
    while (logFile.available()) {
        Serial.println(logFile.readStringUntil('\n'));
    }
    logFile.close();
}

//********This section is for managing the logs********


//Chatgpt 03-24-24
               
                    unsigned long extractTimestamp(const String& line) {
                    // Example implementation, extract and convert the timestamp from the line
                    // Assume the timestamp is at the beginning of the line followed by a space
                    int index = line.indexOf(' ');
                    if (index != -1) {
                    String timestampStr = line.substring(0, index);
                    // Convert the extracted part of the line to an unsigned long
                    // This is just an example; the actual conversion depends on your timestamp format
                    return timestampStr.toInt();
                    }
                    return 0; // Return 0 or an appropriate error value if extraction fails
                }

                    // Helper function to get current month as a string (e.g., "January")
                    String getCurrentMonthString() {
                    DateTime now = rtc.now(); // Assuming you have an RTC object named rtc
                    char monthName[12];
                    snprintf(monthName, sizeof(monthName), "%04d-%02d", now.year(), now.month());
                    return String(monthName);
                    }

                    


                     // Logging.cpp
                     unsigned long extractRuntimeFromLogLine(String line) {
                     // Find the position of "Total Runtime: " in the line
                     int start = line.indexOf("Total Runtime: ") + 15;
                     if (start != -1) {
                     // Extract the substring from this position to the end, excluding " seconds"
                     int end = line.lastIndexOf(" seconds");
                     if (end > start) {
                     String runtimeStr = line.substring(start, end);
                     return runtimeStr.toInt(); // Convert this substring to an unsigned long and return
                    }
                    }
                    return 0; // If parsing fails, return 0
                }

                   



//Chatgpt 03-24-24

 // Helper function to parse datetime string and return a DateTime object
DateTime parseDateTimeFromLog(const String& datetimeStr) {
  // Parses datetime string in "YYYY-MM-DD HH:MM:SS" format and returns a DateTime object
    int year = datetimeStr.substring(0, 4).toInt();
    int month = datetimeStr.substring(5, 7).toInt();
    int day = datetimeStr.substring(8, 10).toInt();
    int hour = datetimeStr.substring(11, 13).toInt();
    int minute = datetimeStr.substring(14, 16).toInt();
    int second = datetimeStr.substring(17, 19).toInt();

    return DateTime(year, month, day, hour, minute, second);
}

unsigned long calculateTotalRuntime(const String& logFilename) {
    File logFile = LittleFS.open(logFilename, "r");
    if (!logFile) {
        Serial.println("Failed to open log file for reading: " + logFilename);
        return 0;
    }

    unsigned long totalRuntime = 0;
    DateTime lastStartTime;
    bool isPumpRunning = false;

    while (logFile.available()) {
        String line = logFile.readStringUntil('\n');
        // Check for START or STOP events and parse the datetime
        if (line.startsWith("START")) {
            String timestampStr = line.substring(6); // Adjust based on your log format
            lastStartTime = parseDateTimeFromLog(timestampStr);
            isPumpRunning = true;
        } else if (line.startsWith("STOP") && isPumpRunning) {
            String timestampStr = line.substring(5); // Adjust based on your log format
            DateTime stopTime = parseDateTimeFromLog(timestampStr);
            totalRuntime += (stopTime.unixtime() - lastStartTime.unixtime());
            isPumpRunning = false;
        }
    }
    logFile.close();
    return totalRuntime;
}







void aggregatePumptoDailyLogs(int pumpIndex) {
    String logFilename = "/pump" + String(pumpIndex + 1) + "_Log.txt";
    String dailyLogFilename = "/pump" + String(pumpIndex + 1) + "_Daily.txt";

    std::map<String, unsigned long> dailyRuntimeMap;
    File dailyLogFile = LittleFS.open(dailyLogFilename, "r");
    if (dailyLogFile) {
        while (dailyLogFile.available()) {
            String line = dailyLogFile.readStringUntil('\n');
            int dateEnd = line.indexOf(" Total Runtime: ");
            if (dateEnd != -1) {
                String date = line.substring(0, dateEnd);
                unsigned long runtime = std::stoul(line.substring(dateEnd + strlen(" Total Runtime: ")).c_str());
                dailyRuntimeMap[date] += runtime;
            }
        }
        dailyLogFile.close();
    }

    DateTime lastStartTime;
    bool isPumpRunning = false;
    File logFile = LittleFS.open(logFilename, "r");
    if (logFile) {
        while (logFile.available()) {
            String line = logFile.readStringUntil('\n');
            if (line.startsWith("START")) {
                lastStartTime = parseDateTimeFromLog(line.substring(6)); // Adjusted to skip "START "
                isPumpRunning = true;
            } else if (line.startsWith("STOP") && isPumpRunning) {
                DateTime stopTime = parseDateTimeFromLog(line.substring(5)); // Adjusted to skip "STOP "
                String date = line.substring(5, 15); // Extract date from STOP line
                dailyRuntimeMap[date] += (stopTime.unixtime() - lastStartTime.unixtime());
                isPumpRunning = false;
            }
        }
        logFile.close();
    }

    // Rewrite the daily log file
    dailyLogFile = LittleFS.open(dailyLogFilename, "w");
    for (const auto& entry : dailyRuntimeMap) {
        dailyLogFile.printf("%s Total Runtime: %lu seconds\n", entry.first.c_str(), entry.second);
    }
    dailyLogFile.close();

    if (isPumpRunning) {
        DateTime now = rtc.now();
        // Instead of removing, we now overwrite with just the ongoing operation's START entry
        File newLog = LittleFS.open(logFilename, "w");
        if (newLog) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "START %04d-%02d-%02d %02d:%02d:%02d\n",
                     now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
            newLog.println(buffer);
            newLog.close();
        }
    } else {
        LittleFS.remove(logFilename); // Clear the log file only if there's no ongoing operation
    }
}






//Chatgpt 03-24-24

unsigned long calculateTotalMonthlyRuntime(const String& dailyLogFilename) {
    File dailyLogFile = LittleFS.open(dailyLogFilename, "r");
    if (!dailyLogFile) {
        Serial.println("Failed to open daily log file for reading: " + dailyLogFilename);
        return 0;
    }

    unsigned long totalMonthlyRuntime = 0;
    while (dailyLogFile.available()) {
        String line = dailyLogFile.readStringUntil('\n');
        // Assuming the line format is "YYYY-MM-DD Total Runtime: XXX seconds"
        int start = line.indexOf("Total Runtime: ") + 15;
        int end = line.lastIndexOf(" seconds");
        if (start != -1 && end != -1 && end > start) {
            String runtimeStr = line.substring(start, end);
            totalMonthlyRuntime += runtimeStr.toInt();
        }
    }
    dailyLogFile.close();
    return totalMonthlyRuntime;
}

                void aggregateDailyToMonthlyLogs(int pumpIndex) {
    String dailyLogFilename = "/pump" + String(pumpIndex + 1) + "_Daily.txt";
    String monthlyLogFilename = "/pump" + String(pumpIndex + 1) + "_Monthly.txt";
    unsigned long totalMonthlyRuntime = calculateTotalMonthlyRuntime(dailyLogFilename);
    String currentMonth = getCurrentMonthString(); // Ensure this returns "YYYY-MM"
    bool monthExists = false;
    unsigned long existingRuntime = 0;
    String updatedContents = "";

    // Check if the monthly log file exists
    if (LittleFS.exists(monthlyLogFilename)) {
        Serial.println("Monthly log file exists. Reading...");
        File monthlyLogFile = LittleFS.open(monthlyLogFilename, "r");
        while (monthlyLogFile.available()) {
            String line = monthlyLogFile.readStringUntil('\n');
            if (line.startsWith(currentMonth)) {
                int spaceIndex = line.indexOf("Total Runtime: ") + 15;
                existingRuntime = line.substring(spaceIndex, line.indexOf(" seconds")).toInt();
                monthExists = true;
            } else {
                updatedContents += line + "\n"; // Keep other months' data
            }
        }
        monthlyLogFile.close();
    } else {
        Serial.println("Monthly log file does not exist. Will create a new one.");
    }

    // Add new runtime to existing or create entry for current month
    if (monthExists) {
        totalMonthlyRuntime += existingRuntime;
    }
    updatedContents += currentMonth + " Total Runtime: " + totalMonthlyRuntime + " seconds\n";

    // Write updated contents back to the monthly log file
    File monthlyLogFile = LittleFS.open(monthlyLogFilename, "w");
    if (monthlyLogFile) {
        monthlyLogFile.print(updatedContents);
        monthlyLogFile.close();
        Serial.println("Monthly log file updated.");
    } else {
        Serial.println("Failed to open monthly log file for writing.");
    }

    // Clear the daily log file after updating the monthly log
    if (LittleFS.remove(dailyLogFilename)) {
        Serial.println("Daily log file cleared.");
    } else {
        Serial.println("Failed to clear daily log file.");
    }
}



//Chatgpt 03-24-24

        unsigned long calculateTotalYearlyRuntime(const String& yearlyLogFilename) {
            File yearlyLogFile = LittleFS.open(yearlyLogFilename, "r");
            if (!yearlyLogFile) {
                Serial.println("Failed to open yearly log file for reading: " + yearlyLogFilename);
                return 0;
            }

            unsigned long totalYearlyRuntime = 0;
            while (yearlyLogFile.available()) {
                String line = yearlyLogFile.readStringUntil('\n');
                // Use the existing `extractRuntimeFromLogLine` function
                totalYearlyRuntime += extractRuntimeFromLogLine(line);
            }
            yearlyLogFile.close();
            return totalYearlyRuntime;
        }


              void aggregateMonthlyToYearlyLogs(int pumpIndex) {
    String monthlyLogFilename = "/pump" + String(pumpIndex + 1) + "_Monthly.txt";
    String yearlyLogFilename = "/pump" + String(pumpIndex + 1) + "_Yearly.txt";
    
    unsigned long totalYearlyRuntime = calculateTotalYearlyRuntime(monthlyLogFilename);
    String currentYear = String(getCurrentYear());
    bool yearExists = false;
    unsigned long existingRuntime = 0;
    String updatedContents = "";
      
    // Open the yearly log file for reading. If it doesn't exist, it's okay.
    File yearlyLogFile = LittleFS.open(yearlyLogFilename, "r");
    if (yearlyLogFile) {
        while (yearlyLogFile.available()) {
            String line = yearlyLogFile.readStringUntil('\n');
            if (line.startsWith(currentYear)) {
                int spaceIndex = line.indexOf("Total Runtime: ") + 15;
                existingRuntime = line.substring(spaceIndex, line.indexOf(" seconds")).toInt();
                yearExists = true;
            } else {
                updatedContents += line + "\n";
            }
        }
        yearlyLogFile.close();
    }

    if (yearExists) {
        totalYearlyRuntime += existingRuntime;
    }
    updatedContents += currentYear + " Total Runtime: " + totalYearlyRuntime + " seconds\n";

    // Overwrite the yearly log file with new contents
    yearlyLogFile = LittleFS.open(yearlyLogFilename, "w");
    if (!yearlyLogFile) {
        Serial.println("Failed to open yearly log file for writing.");
        return;
    }
    yearlyLogFile.print(updatedContents);
    yearlyLogFile.close();

    // Optionally clear the monthly log file
    if (LittleFS.remove(monthlyLogFilename)) {
        Serial.println("Cleared monthly log file.");
    } else {
        Serial.println("Failed to clear monthly log file.");
    }
    Serial.println("Aggregated monthly logs to yearly log for pump " + String(pumpIndex + 1) + ".");
}


    
     
    
    

                   


// ChatGPT 03-24-24
            void performLogAggregation() {
                // Aggregate daily logs for each pump
                for (int i = 0; i < 10; i++) {
                    aggregatePumptoDailyLogs(i);
                }

                String currentDate = getCurrentDateStringMDY();
                // Extract month and day from the current date string
                int month = currentDate.substring(0, 2).toInt();
                int day = currentDate.substring(3, 5).toInt();

                // Check if it's the first day of any month
                if (day == 1) {
                    // It's the first day of a month, aggregate daily to monthly logs for each pump
                    for (int i = 0; i < 10; i++) {
                        aggregateDailyToMonthlyLogs(i);
                    }

                    // Additionally, check if it's the first day of the year (January 1st)
                    if (month == 1) {
                        // It's the first day of the year, aggregate monthly to yearly logs for each pump
                        for (int i = 0; i < 10; i++) {
                            aggregateMonthlyToYearlyLogs(i);
                        }
                    }
                }
            
            }

void setElapsed_Day() {
    if (!Elapsed_Day) { // Check if Elapsed_Day is false
        Elapsed_Day = true;
        Serial.println("Elapsed_Day flag set to true");
    } 
}

void setperformLogAggregation() {
    if (Elapsed_Day) { // Check if Elapsed_Day is true
        performLogAggregation();
        Elapsed_Day = false;
        Serial.println("Log aggregation performed and Elapsed_Day flag reset");
    }
}
  

void checkTimeAndAct() {
  
    if (CurrentTime.hour() == 23 && CurrentTime.minute() == 59) {
      setElapsed_Day();  
    } 
    if (CurrentTime.hour() == 00 && CurrentTime.minute() == 01) {
     setperformLogAggregation();   
    }
}
