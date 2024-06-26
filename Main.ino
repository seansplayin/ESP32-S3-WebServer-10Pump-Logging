#include <Ticker.h>
#include "Config.h"
#include "Logging.h"
#include "NetworkManager.h"
#include "PumpManager.h"
#include "RTCManager.h"
#include "TemperatureControl.h"
#include "TimeSync.h"
#include "WebServerManager.h"
#include "FileSystemManager.h"
#include "FirstWebpage.h"
#include "SecondWebpage.h"
#include "TaskManager.h"








void setup() {
   Serial.begin(115200);
   
   startAllTasks();  // This function starts all the tasks you've defined

   /* 
   setupRTC(); // Initialize the DS3231 RTC, function in RTCManager.cpp file
  
   setupNetwork(); // Initialize the W5500 Ethernet Adapter, function in NetworkManager.cpp file
     
   initNTP(); // Connect to NTP server to retreve time and call 'tryNtpUpdate()' to Sync updated time to DS3231 RTC, function in TimeSync.cpp file
    
   initializeTime(); // Initialize and obtain current time, function in TimeSync.cpp file
   
   initializeFileSystem(); // Initialize the file system, function in FileSystemManager.cpp file

   initializePumps(); // Initialize pump relay array and set pins as outputs, function in PumpManager.cpp file
    
   startServer(); // Initialize WebSocket and set up server routes, function in WebServerManager.cpp file

   setupFirstPageRoutes(); // Setup routes and WebSocket for the first page, function in FirstWebpage.cpp file

   setupSecondPageRoutes(); // Setup routes and WebSocket for the second page, function in SecondWebpage.cpp file
   
   setupLogDataRoute(); // Set up the route for AJAX requests for log data. used for graphs on second webpage, function in WebServerManager.cpp file
   
   dateTimeTicker(); // calls 'refreshCurrentTime()' which calls 'broadcastDateTime()' to broadcast time on ws, Also calls 'executeEverySecond' which calls 'executecheckTimeAndAct()', 'checkAndSyncTime()', executecheckAutoModeConditions(); function in RTCManager.cpp file
  
   //setupPumpBroadcasting(); // Broadcasts the State and Mode of all 10 pumps to the Serial Monitor

   //setupTemperatureBroadcasting(); // Broadcasts current and threshold temperatures all 10 pumps to the Serial Monitor 
  */
  
}

void loop() {

//********Monitors Serial Input for Simulated Temperature change commands********
  // Handle Serial Commands for Temperature Adjustment
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("T,")) {
        int commaIndex = command.indexOf(',', 2);
        if (commaIndex > 0 && commaIndex < command.length() - 1) {
            int pumpIndex = command.substring(2, commaIndex).toInt() - 1;
            float newTemp = command.substring(commaIndex + 1).toFloat();
            if (pumpIndex >= 0 && pumpIndex < 10) {
                updateTemperature(pumpIndex, newTemp); // Call the new function
            }
        }
    }
}
/*  
//********Monitors Serial Input for Manual Pump commands********    
    // Handle Serial Commands for Temperature and Pump Mode Adjustment
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Trim any whitespace

        // Check if the command is for setting pump mode
        if (command.startsWith("P,")) {
            int firstCommaIndex = command.indexOf(',');
            int secondCommaIndex = command.indexOf(',', firstCommaIndex + 1);

            if (firstCommaIndex != -1 && secondCommaIndex != -1) {
                // Parse the pump number and desired mode from the command
                int pumpNumber = command.substring(firstCommaIndex + 1, secondCommaIndex).toInt();
                int mode = command.substring(secondCommaIndex + 1).toInt();

                // Ensure the pump number is within the valid range
                if (pumpNumber >= 1 && pumpNumber <= 10) {
                    // Adjust for array index (0-based)
                    int pumpIndex = pumpNumber - 1;

                    // Set the pump mode based on the command
                    setPumpMode(pumpIndex, mode);
                    Serial.println("Set Pump " + String(pumpNumber) + " to Mode: " + mode);
                } else {
                    Serial.println("Invalid Pump Number: " + String(pumpNumber));
                }
            }
        }
        // Add your existing temperature adjustment code here...
    }
   */ 
//********Monitors Serial Input for LittleFS commands********
//enter 'list files to list log files in serial monitor
//enter 'read file filename' to read contents of a log file in serial monitor
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any whitespace

    if (command == "list files") {
      listAllFiles();
    } else if (command.startsWith("read file ")) {
      // Extract the filename from the command
      String filename = command.substring(10); // Remove "read file " from the command
      readAndPrintLogFile(filename);
    }
  }


if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Trim any leading/trailing whitespace

        if (command.startsWith("aggDaily")) {
            // Example command: "aggDaily,1" to aggregate logs for pump 1
            int pumpIndex = command.substring(command.indexOf(',') + 1).toInt() - 1;
            aggregatePumptoDailyLogs(pumpIndex);
            Serial.println("Aggregated daily logs for Pump " + String(pumpIndex + 1));
        } else if (command.startsWith("aggMonthly")) {
            // Call aggregateDailyToMonthlyLogs for a specific pump or all pumps
            int pumpIndex = command.substring(command.indexOf(',') + 1).toInt() - 1;
            aggregateDailyToMonthlyLogs(pumpIndex);
            Serial.println("Aggregated monthly logs for Pump " + String(pumpIndex + 1));
        } else if (command.startsWith("aggYearly")) {
            // Call aggregateMonthlyToYearlyLogs for a specific pump or all pumps
            int pumpIndex = command.substring(command.indexOf(',') + 1).toInt() - 1;
            aggregateMonthlyToYearlyLogs(pumpIndex);
            Serial.println("Aggregated yearly logs for Pump " + String(pumpIndex + 1));
        } else if (command == "aggAll") {
            // Call performLogAggregation to perform all aggregations
            performLogAggregation();
            Serial.println("Performed full log aggregation cycle.");
        }
        // Continue with other command handling...
    }

}
