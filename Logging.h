#ifndef LOGGING_H
#define LOGGING_H

#include "Config.h"


// Declare functions for logging events, managing logs, etc.
void logPumpEvent(int pumpIndex, const String& event);

// These functions might not necessarily need to be exposed if they're only used within Logging.cpp,
// but if you plan on calling them from other parts of your code, declare them here.
void listAllFiles();
void readAndPrintLogFile(const String& filename);


void aggregatePumptoDailyLogs(int pumpIndex);
void aggregateDailyToMonthlyLogs(int pumpIndex);
void aggregateMonthlyToYearlyLogs(int pumpIndex);
void performLogAggregation();
//void parseDateTimeFromLog();
//void calculateTotalRuntime();
void checkTimeAndAct(); 


unsigned long extractRuntimeFromLogLine(String line);

unsigned long extractTimestamp(const String& line);

// If you have any constants that are used across different files that relate to logging,
// declare them here. For example:
// extern const int MAX_LOG_SIZE; // Just an example

// If logMessage is used outside of Logging.cpp, declare it here as well
void logMessage(const String& message);


#endif // LOGGING_H
