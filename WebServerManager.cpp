#include "WebServerManager.h"
#include "Logging.h"
#include "Config.h"
#include "PumpManager.h"
#include <FS.h>
#include <SPIFFS.h>
#include "RTCManager.h" // For getCurrentDateString()
#include <ESPAsyncWebServer.h>
#include "SecondWebpage.h"
#include <ArduinoJson.h> // Make sure to include ArduinoJson library


AsyncWebServer server(80); // Initialize AsyncWebServer on port 80

AsyncWebSocket ws("/ws"); // Initialize AsyncWebSocket on URI "/ws"


void startServer() {
    // Initialize WebSocket
    initWebSocket();
        
    // Add WebSocket handler to the server
    server.addHandler(&ws);

    // Start the server
    server.begin();
}

void initWebSocket() {
    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("WebSocket client connected");
            // Send the current state and mode of all pumps to the newly connected client
            for (int i = 0; i < 10; i++) { // Assuming you have 10 pumps
                String stateMessage = "pump" + String(i + 1) + "State:" + (pumpStates[i] == PUMP_ON ? "on" : "off");
                String modeMessage = "pump" + String(i + 1) + "Mode:" + (pumpModes[i] == PUMP_AUTO ? "auto" : (pumpModes[i] == PUMP_ON ? "on" : "off"));
                // Combine state and mode messages for the pump and send
                client->text(stateMessage + "," + modeMessage);
            }
        }
        // Handle other event types (e.g., WS_EVT_DISCONNECT, WS_EVT_DATA) as before
        handleWebSocketMessage(arg, data, len);
    });
}


/*
void initWebSocket() {
    // Set up WebSocket event handling
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        handleWebSocketMessage(arg, data, len);
    });
    server.addHandler(&ws); // Add WebSocket handler to the server
}
*/

 void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->opcode == WS_TEXT) {
        data[len] = 0; // Ensure null-termination
        String message = (char*)data;
        Serial.print("Received WS message: ");
        Serial.println(message);

        if (message.startsWith("setPumpMode:")) {
            int firstColon = message.indexOf(':');
            int secondColon = message.indexOf(':', firstColon + 1);
            int pumpIndex = message.substring(firstColon + 1, secondColon).toInt() - 1;
            String mode = message.substring(secondColon + 1);
            mode.toLowerCase(); // This modifies 'mode' in place

            Serial.print("Parsed Pump Index: ");
            Serial.print(pumpIndex);
            Serial.print(", Mode: ");
            Serial.println(mode);

            // Assuming your logic to act on the message goes here
            // For example:
            if (mode == "on") {
                setPumpMode(pumpIndex, PUMP_ON);
            } else if (mode == "off") {
                setPumpMode(pumpIndex, PUMP_OFF);
            } else if (mode == "auto") {
                setPumpMode(pumpIndex, PUMP_AUTO);
            }
        }
        
    if (message.startsWith("requestLogData")) {
            // Parse the message to extract the pump index and timeframe
            int firstColon = message.indexOf(':');
            int secondColon = message.lastIndexOf(':');
            int pumpIndex = message.substring(firstColon + 1, secondColon).toInt();
            String timeframe = message.substring(secondColon + 1);

            // Prepare the log data based on the requested timeframe
            String logData = prepareLogData(pumpIndex, timeframe);
            
            // Send the log data back to the client
            ws.textAll(logData);
        }
    }
}

// Assume ws is a global WebSocket server instance
void broadcastMessageOverWebSocket(const String& message) {
    ws.textAll(message);
}




// Assuming parseDateTimeFromLog is the correct and working parsing function
DateTime parseDateTimeFromLogFile(const String& datetimeStr) {
    // Parses datetime string in "YYYY-MM-DD HH:MM:SS" format and returns a DateTime object
    int year = datetimeStr.substring(0, 4).toInt();
    int month = datetimeStr.substring(5, 7).toInt();
    int day = datetimeStr.substring(8, 10).toInt();
    int hour = datetimeStr.substring(11, 13).toInt();
    int minute = datetimeStr.substring(14, 16).toInt();
    int second = datetimeStr.substring(17).toInt(); // Assuming the rest of the string is seconds
    return DateTime(year, month, day, hour, minute, second);
}





unsigned long calculateTotalLogRuntime(const String& logFilename) {
    File logFile = SPIFFS.open(logFilename, "r");
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
            lastStartTime = parseDateTimeFromLogFile(timestampStr);
            isPumpRunning = true;
        } else if (line.startsWith("STOP") && isPumpRunning) {
            String timestampStr = line.substring(5); // Adjust based on your log format
            DateTime stopTime = parseDateTimeFromLogFile(timestampStr);
            totalRuntime += (stopTime.unixtime() - lastStartTime.unixtime());
            isPumpRunning = false;
        }
    }
    logFile.close();
    return totalRuntime;
}



 
String prepareLogData(int pumpIndex, String timeframe) {
    // String to hold the aggregated log data
    String logData;

    // Determine the timeframe and aggregate data accordingly
    if (timeframe == "day") {
        logData = aggregateDailyLogsReport(pumpIndex);
    } else if (timeframe == "month") {
        logData = aggregateMonthlyLogsReport(pumpIndex);
    } else if (timeframe == "year") {
        logData = aggregateYearlyLogsReport(pumpIndex);
    } else if (timeframe == "decade") {
        logData = aggregateDecadeLogsReport(pumpIndex);
    }

    // Format the log data for WebSocket transmission
    // This could involve JSON formatting or other structured formats

    return logData;
}





// Helper function to format runtime from seconds into "2h 15m 30s" format
String formatRuntime(long totalSeconds) {
    long hours = totalSeconds / 3600;
    long minutes = (totalSeconds % 3600) / 60;
    long seconds = totalSeconds % 60;

    // Format the string as "2h 15m 30s"
    String formattedRuntime = "";
    if (hours > 0) formattedRuntime += String(hours) + "h ";
    if (minutes > 0 || hours > 0) formattedRuntime += String(minutes) + "m ";
    formattedRuntime += String(seconds) + "s";
        
    return formattedRuntime;
}

String aggregateDailyLogsReport(int pumpIndex) {
    String logFilename = "/pump" + String(pumpIndex + 1) + "_Log.txt";

    // Use the system-wide current time to format the date string
    char currentDate[11]; // Buffer to hold the formatted date string
    snprintf(currentDate, sizeof(currentDate), "%04d-%02d-%02d", 
             CurrentTime.year(), CurrentTime.month(), CurrentTime.day());

    // Calculate the total runtime directly from the log file
    unsigned long totalRuntime = calculateTotalLogRuntime(logFilename);

    // Optionally, format the runtime into a readable string (e.g., "2h 15m 30s")
    String formattedRuntime = formatRuntime(totalRuntime);
        
    return formattedRuntime;
}




// Function to aggregate monthly logs for reporting to webpage through ws
String aggregateMonthlyLogsReport(int pumpIndex) {
    // Ensure we're using the correct file for monthly aggregation.
    // It appears we should aggregate from daily logs rather than a monthly log file.
    String dailyLogFilename = "/pump" + String(pumpIndex + 1) + "_Daily.txt";
    File dailyLogFile = SPIFFS.open(dailyLogFilename, "r");

    if (!dailyLogFile) {
        Serial.println("Failed to open " + dailyLogFilename + " for reading.");
        return "Error: File not found";
    }

    // Buffer to hold the formatted date string for the current month "YYYY-MM"
    char currentMonth[8];
    snprintf(currentMonth, sizeof(currentMonth), "%04d-%02d", CurrentTime.year(), CurrentTime.month());

    unsigned long totalRuntimeSeconds = 0;
    while (dailyLogFile.available()) {
        String line = dailyLogFile.readStringUntil('\n');
        // Check if the line corresponds to the current month
        if (line.startsWith(currentMonth)) {
            // Assuming the line format is "YYYY-MM-DD Total Runtime: XXXX seconds"
            int indexStart = line.indexOf("Total Runtime: ") + 15;
            int indexEnd = line.indexOf(" seconds", indexStart);
            if (indexStart != -1 && indexEnd != -1) {
                unsigned long runtimeSeconds = line.substring(indexStart, indexEnd).toInt();
                totalRuntimeSeconds += runtimeSeconds;
            }
        }
    }
    dailyLogFile.close(); // Close the file when done

    // Use the formatRuntime function to convert total runtime from seconds to the desired format
    String formattedRuntime = formatRuntime(totalRuntimeSeconds);

    // Construct the response string
    String response = "Total Runtime for " + String(currentMonth) + ": " + formattedRuntime;

    return response; // Return the formatted total runtime for the current month
}



// Function to aggregate yearly logs for reporting to webpage through ws
String aggregateYearlyLogsReport(int pumpIndex) {
    // We should be aggregating from monthly logs instead of a yearly log file.
    String monthlyLogFilename = "/pump" + String(pumpIndex + 1) + "_Monthly.txt";
    File monthlyLogFile = SPIFFS.open(monthlyLogFilename, "r");

    if (!monthlyLogFile) {
        Serial.println("Failed to open " + monthlyLogFilename + " for reading.");
        return "Error: File not found";
    }

    unsigned long totalRuntimeSeconds = 0;
    char currentYear[5]; // Buffer to hold the formatted year string "YYYY"
    snprintf(currentYear, sizeof(currentYear), "%04d", CurrentTime.year());

    while (monthlyLogFile.available()) {
        String line = monthlyLogFile.readStringUntil('\n');
        // Check if the line corresponds to the current year
        if (line.startsWith(currentYear)) {
            // Assuming the line format is "YYYY-MM Total Runtime: XXXX seconds"
            int indexStart = line.indexOf("Total Runtime: ") + 15;
            int indexEnd = line.indexOf(" seconds", indexStart);
            if (indexStart != -1 && indexEnd != -1) {
                unsigned long runtimeSeconds = line.substring(indexStart, indexEnd).toInt();
                totalRuntimeSeconds += runtimeSeconds;
            }
        }
    }
    monthlyLogFile.close(); // Close the file when done

    // Use the formatRuntime function to convert total runtime from seconds to the desired format
    String formattedRuntime = formatRuntime(totalRuntimeSeconds);

    // Construct the response string
    String response = "Total Runtime for " + String(currentYear) + ": " + formattedRuntime;

    return response; // Return the formatted total runtime for the current year
}




// Function to aggregate yearly logs for reporting to webpage through ws
String aggregateDecadeLogsReport(int pumpIndex) {
    String filename = "/pump" + String(pumpIndex + 1) + "_Yearly.txt"; // Construct the filename
    File file = SPIFFS.open(filename, "r"); // Attempt to open the file

    if (!file) {
        Serial.println("Failed to open " + filename);
        return "Error: File not found";
    }

    unsigned long totalRuntime = 0; // Initialize total runtime

    // Read and process each line in the file
    while (file.available()) {
        String line = file.readStringUntil('\n');
        // Assuming each line is in the format: YYYY Total Runtime: XXXX seconds
        int runtimeStart = line.indexOf("Total Runtime: ") + 15;
        int runtimeEnd = line.indexOf(" seconds", runtimeStart);
        if(runtimeStart != -1 && runtimeEnd != -1) {
            String runtimeStr = line.substring(runtimeStart, runtimeEnd);
            totalRuntime += runtimeStr.toInt(); // Accumulate total runtime
        }
    }

    file.close(); // Close the file when done

    // Use the formatRuntime function to convert total runtime from seconds to "2h 15m 30s" format
    String formattedRuntime = formatRuntime(totalRuntime);

    // Construct the response string
    String response = "Total Runtime Over Years: " + formattedRuntime;

    return response; // Return the formatted summary
}


// New function for setting up the AJAX route
void setupLogDataRoute() {
    server.on("/get-log-data", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("pumpIndex") && request->hasParam("timeframe")) {
            String pumpIndexParam = request->getParam("pumpIndex")->value();
            String timeframe = request->getParam("timeframe")->value();
            int pumpIndex = pumpIndexParam.toInt() - 1; // Assuming pumpIndex is 1-based in your request
            
            String runtime;
            
            // Determine the timeframe and call the appropriate function
            if (timeframe == "day") {
                runtime = aggregateDailyLogsReport(pumpIndex);
            } else if (timeframe == "month") {
                runtime = aggregateMonthlyLogsReport(pumpIndex);
            } else if (timeframe == "year") {
                runtime = aggregateYearlyLogsReport(pumpIndex);
            } else if (timeframe == "decade") {
                runtime = aggregateDecadeLogsReport(pumpIndex);
            } else {
                request->send(400, "application/json", "{\"error\":\"Invalid timeframe\"}");
                return;
            }

            // Prepare the JSON response
            DynamicJsonDocument doc(1024);
            doc["runtime"] = runtime;
            // Example: Adding a fixed percentage for demonstration purposes
            // You might want to calculate or fetch an actual value for this
            doc["percentage"] = 100; // Placeholder percentage value
            
            String response;
            serializeJson(doc, response);
            
            request->send(200, "application/json", response);
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
        }
    });
}
