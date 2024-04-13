#include "WebServerManager.h"
#include "Logging.h"
#include "Config.h"
#include "PumpManager.h"
#include <FS.h>
//#include <SPIFFS.h>
#include <LittleFS.h>
#include "RTCManager.h" // For getCurrentDateString()
#include <ESPAsyncWebServer.h>
#include "SecondWebpage.h"
#include <ArduinoJson.h> // Make sure to include ArduinoJson library


AsyncWebServer server(80); // Initialize AsyncWebServer on port 80

AsyncWebSocket ws("/ws"); // Initialize AsyncWebSocket on URI "/ws"

unsigned long dayRuntimeSeconds;
unsigned long monthRuntimeSeconds;
unsigned long yearRuntimeSeconds;

void startServer() {
    initWebSocket(); // Initialize WebSocket
    server.addHandler(&ws); // Add WebSocket handler to the server
    setupRoutes();  // Setup additional routes for listing and downloading files
    server.begin(); // Start the server
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

void setupRoutes() {
    server.on("/list-logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        File root = LittleFS.open("/");
        if (!root || !root.isDirectory()) {
            request->send(500, "text/plain", "Failed to open directory");
            return;
        }

        String json = "[";
        File file = root.openNextFile();
        bool first = true;
        while (file) {
            if (!first) json += ",";
            json += "\"" + String(file.name()) + "\"";
            first = false;
            file = root.openNextFile();
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // Serve a specific log file for download from the root directory
server.on("/download-log", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
        String filename = request->getParam("file")->value();
        // Security check: avoid directory traversal
        if (filename.indexOf('/') != -1 || filename.indexOf('\\') != -1) {
            request->send(400, "text/plain", "Invalid file path");
            return;
        }

        // Debug: Check if file exists
        String filePath = "/" + filename; // Assuming files are in the root directory
        if (LittleFS.exists(filePath)) {
            Serial.println("Sending file: " + filePath);
            request->send(LittleFS, filePath, String(), true);
        } else {
            Serial.println("File not found: " + filePath);
            request->send(404, "text/plain", "File not found");
        }
    } else {
        request->send(400, "text/plain", "Missing file parameter");
    }
});

}



 void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->opcode == WS_TEXT) {
        data[len] = '\0'; // Ensure null-termination
        String message = String((char*)data);
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

            // Logic to act on the pump mode message
            if (mode == "on") {
                setPumpMode(pumpIndex, PUMP_ON);
            } else if (mode == "off") {
                setPumpMode(pumpIndex, PUMP_OFF);
            } else if (mode == "auto") {
                setPumpMode(pumpIndex, PUMP_AUTO);
            }
        } else if (message.startsWith("requestLogData")) {
            // Parse the message for pump index and timeframe
            int firstColon = message.indexOf(':');
            int secondColon = message.lastIndexOf(':');
            int pumpIndex = message.substring(firstColon + 1, secondColon).toInt();
            String timeframe = message.substring(secondColon + 1);

            // Prepare and send the log data
            String logData = prepareLogData(pumpIndex, timeframe);
            ws.textAll(logData);
        } else if (message.equals("updateAllRuntimes")) {
            // Handle the "Update All" button click
            updateAllRuntimes();
        }
        // You can add more message handling as needed
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
    unsigned long runtimeSeconds = 0;

    if (timeframe == "day") {
        runtimeSeconds = aggregateDailyLogsReport(pumpIndex);
    } else if (timeframe == "month") {
        unsigned long dayRuntimeSeconds = aggregateDailyLogsReport(pumpIndex);
        runtimeSeconds = aggregateMonthlyLogsReport(pumpIndex, dayRuntimeSeconds);
    } else if (timeframe == "year") {
        unsigned long dayRuntimeSeconds = aggregateDailyLogsReport(pumpIndex);
        unsigned long monthRuntimeSeconds = aggregateMonthlyLogsReport(pumpIndex, dayRuntimeSeconds);
        runtimeSeconds = aggregateYearlyLogsReport(pumpIndex, monthRuntimeSeconds);
    } else if (timeframe == "decade") {
        unsigned long dayRuntimeSeconds = aggregateDailyLogsReport(pumpIndex);
        unsigned long monthRuntimeSeconds = aggregateMonthlyLogsReport(pumpIndex, dayRuntimeSeconds);
        unsigned long yearRuntimeSeconds = aggregateYearlyLogsReport(pumpIndex, monthRuntimeSeconds);
        runtimeSeconds = aggregateDecadeLogsReport(pumpIndex, yearRuntimeSeconds);
    }

    // Now convert runtimeSeconds back to a String if needed, or directly use it if you are sending numbers to the front-end
    // For example, if you're directly sending the seconds to a JavaScript function that formats the time, you might keep it as is
    return String(runtimeSeconds); // or return runtimeSeconds; based on your requirement
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

// Function to aggregate daily logs for reporting to webpage through ws
unsigned long aggregateDailyLogsReport(int pumpIndex) {
    String logFilename = "/pump" + String(pumpIndex + 1) + "_Log.txt";

    // Use the system-wide current time to format the date string
    char currentDate[11]; // Buffer to hold the formatted date string
    snprintf(currentDate, sizeof(currentDate), "%04d-%02d-%02d", 
             CurrentTime.year(), CurrentTime.month(), CurrentTime.day());

    // Calculate the total runtime directly from the log file
    unsigned long totalRuntime = calculateTotalLogRuntime(logFilename);
        
    return totalRuntime; // Return total runtime in seconds
}





// Function to aggregate monthly logs for reporting to webpage through ws
// Function to aggregate monthly logs for reporting to webpage through ws
unsigned long aggregateMonthlyLogsReport(int pumpIndex, unsigned long dayRuntimeSeconds) {
    String dailyLogFilename = "/pump" + String(pumpIndex + 1) + "_Daily.txt";
    File dailyLogFile = LittleFS.open(dailyLogFilename, "r");

    if (!dailyLogFile) {
        Serial.println("Failed to open " + dailyLogFilename + " for reading.");
        return 0; // Return 0 seconds if file not found
    }

    char currentMonth[8];
    snprintf(currentMonth, sizeof(currentMonth), "%04d-%02d", CurrentTime.year(), CurrentTime.month());

    unsigned long totalRuntimeSeconds = dayRuntimeSeconds; // Start with the day's runtime
    while (dailyLogFile.available()) {
        String line = dailyLogFile.readStringUntil('\n');
        if (line.startsWith(currentMonth)) {
            int indexStart = line.indexOf("Total Runtime: ") + 15;
            int indexEnd = line.indexOf(" seconds", indexStart);
            if (indexStart != -1 && indexEnd != -1) {
                totalRuntimeSeconds += line.substring(indexStart, indexEnd).toInt();
            }
        }
    }
    dailyLogFile.close();

    return totalRuntimeSeconds;
}



// Function to aggregate yearly logs for reporting to webpage through ws
// Function to aggregate yearly logs for reporting to webpage through ws
unsigned long aggregateYearlyLogsReport(int pumpIndex, unsigned long monthRuntimeSeconds) {
    String monthlyLogFilename = "/pump" + String(pumpIndex + 1) + "_Monthly.txt";
    File monthlyLogFile = LittleFS.open(monthlyLogFilename, "r");

    if (!monthlyLogFile) {
        Serial.println("Failed to open " + monthlyLogFilename + " for reading.");
        return 0; // Return 0 seconds if file not found
    }

    unsigned long totalRuntimeSeconds = monthRuntimeSeconds; // Start with the month's runtime
    char currentYear[5];
    snprintf(currentYear, sizeof(currentYear), "%04d", CurrentTime.year());

    while (monthlyLogFile.available()) {
        String line = monthlyLogFile.readStringUntil('\n');
        if (line.startsWith(currentYear)) {
            int indexStart = line.indexOf("Total Runtime: ") + 15;
            int indexEnd = line.indexOf(" seconds", indexStart);
            if (indexStart != -1 && indexEnd != -1) {
                totalRuntimeSeconds += line.substring(indexStart, indexEnd).toInt();
            }
        }
    }
    monthlyLogFile.close();

    return totalRuntimeSeconds;
}




// Function to aggregate yearly logs for reporting to webpage through ws
// Function to aggregate decade logs for reporting to webpage through ws
unsigned long aggregateDecadeLogsReport(int pumpIndex, unsigned long yearRuntimeSeconds) {
    String yearlyLogFilename = "/pump" + String(pumpIndex + 1) + "_Yearly.txt";
    File yearlyLogFile = LittleFS.open(yearlyLogFilename, "r");

    if (!yearlyLogFile) {
        Serial.println("Failed to open " + yearlyLogFilename + " for reading.");
        return 0; // Return 0 seconds if file not found
    }

    unsigned long totalRuntime = yearRuntimeSeconds; // Start with the year's runtime
    while (yearlyLogFile.available()) {
        String line = yearlyLogFile.readStringUntil('\n');
        int runtimeStart = line.indexOf("Total Runtime: ") + 15;
        int runtimeEnd = line.indexOf(" seconds", runtimeStart);
        if(runtimeStart != -1 && runtimeEnd != -1) {
            totalRuntime += line.substring(runtimeStart, runtimeEnd).toInt();
        }
    }
    yearlyLogFile.close();

    return totalRuntime;
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
                runtime = aggregateMonthlyLogsReport(pumpIndex, dayRuntimeSeconds);
            } else if (timeframe == "year") {
                runtime = aggregateYearlyLogsReport(pumpIndex, monthRuntimeSeconds);
            } else if (timeframe == "decade") {
                runtime = aggregateDecadeLogsReport(pumpIndex, yearRuntimeSeconds);
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

// Responds to 'Update All' request from Webpage. 
void updateAllRuntimes() {
    DynamicJsonDocument doc(1024); // Adjust size based on your data needs
    JsonArray data = doc.createNestedArray("data");

    for (int i = 0; i < 10; i++) {
        unsigned long dayRuntimeSeconds = aggregateDailyLogsReport(i);
        unsigned long monthRuntimeSeconds = aggregateMonthlyLogsReport(i, dayRuntimeSeconds); // Ensure this function accepts unsigned long
        unsigned long yearRuntimeSeconds = aggregateYearlyLogsReport(i, monthRuntimeSeconds); // Ensure this function accepts unsigned long
        unsigned long totalRuntimeSeconds = aggregateDecadeLogsReport(i, yearRuntimeSeconds); // Ensure this function accepts unsigned long

        JsonObject pumpData = data.createNestedObject();
        pumpData["pumpIndex"] = i + 1;
        pumpData["day"] = dayRuntimeSeconds;
        pumpData["month"] = monthRuntimeSeconds;
        pumpData["year"] = yearRuntimeSeconds;
        pumpData["total"] = totalRuntimeSeconds;
    }

    String jsonString;
    serializeJson(doc, jsonString);
    ws.textAll("JSON:" + jsonString); // Broadcast to all connected clients
}
