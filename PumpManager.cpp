#include "PumpManager.h"
#include "Config.h"
#include "Logging.h"
#include "WebServerManager.h"
#include "TemperatureControl.h"

#include <Ticker.h>

// Pump Pin Declarations
const int pumpPins[10] = {2, 42, 41, 40, 39, 38, 37, 36, 35, 0};

int pumpModes[10] = {PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO, PUMP_AUTO};
int pumpStates[10] = {PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF, PUMP_OFF};

bool pumpOnStateHigh[10] = {false, false, false, false, false, false, false, false, false, false};
bool pumpOffStateHigh[10] = {true, true, true, true, true, true, true, true, true, true};
const int numPumps = 10;

extern const int pumpPins[10]; // Assuming these are declared elsewhere

Ticker broadcastPumpTicker; 

void setupPumpBroadcasting() {
    // Use a lambda that calls broadcastPumpState with -1 to broadcast all pumps
    broadcastPumpTicker.attach(10, []() { broadcastPumpState(-1); });
}

// Adjust broadcastPumpState to optionally take a pump index. If -1, broadcast all.
void broadcastPumpState(int pumpIndex) {
     String message = "";
     String serialMessage = ""; // Initialize an empty string for Serial output

        if (pumpIndex == -1) {
        // Broadcast all pump states, each on a new line for the Serial Monitor
        for (int i = 0; i < 10; i++) {
            String pumpState = (pumpStates[i] == PUMP_ON ? "on" : "off");
            String pumpMode = (pumpModes[i] == PUMP_AUTO ? "auto" : (pumpModes[i] == PUMP_ON ? "on" : "off"));
            
            // Prepare WebSocket message
            String pumpMessage = "pump" + String(i + 1) + "State:" + pumpState + ",pump" + String(i + 1) + "Mode:" + pumpMode;
            message += pumpMessage;
            if (i < 9) message += ","; // Add comma except for the last pump
            
            // Prepare Serial message
            serialMessage += "Pump " + String(i + 1) + ". State: " + pumpState + ", Mode: " + pumpMode + "\n";
        }
        } else {
        // Broadcast specific pump state
        String pumpState = (pumpStates[pumpIndex] == PUMP_ON ? "on" : "off");
        String pumpMode = (pumpModes[pumpIndex] == PUMP_AUTO ? "auto" : (pumpModes[pumpIndex] == PUMP_ON ? "on" : "off"));
        
        String pumpMessage = "pump" + String(pumpIndex + 1) + "State:" + pumpState + ",pump" + String(pumpIndex + 1) + "Mode:" + pumpMode;
        message = pumpMessage; // For a specific pump, the WebSocket message is just about that pump
        
        // Prepare Serial message for a specific pump
        serialMessage = "Pump " + String(pumpIndex + 1) + ". State: " + pumpState + ", Mode: " + pumpMode + "\n";
        }

        broadcastMessageOverWebSocket(message); // Send the compiled message to all WebSocket clients
        Serial.print(serialMessage); // Print the formatted message to the Serial Monitor
        Serial.println(); // Print a blank line
    }

void setPumpState(int pumpIndex, int state) {
    // Assuming you have a function named logPumpEvent that logs pump events
    String event = "Pump " + String(pumpIndex + 1) + " " + (state == PUMP_ON ? "ON" : "OFF");
    
    // Update the physical state of the pump
    digitalWrite(pumpPins[pumpIndex], state == PUMP_ON ? (pumpOnStateHigh[pumpIndex] ? HIGH : LOW) : (pumpOnStateHigh[pumpIndex] ? LOW : HIGH));
    pumpStates[pumpIndex] = state;
    
    // Log the event to the serial monitor and broadcast the new pump state
    Serial.println(event);
    broadcastPumpState(pumpIndex);

    // Log the event conditionally, ensuring it's done only once per action
    if (state == PUMP_ON) {
        logPumpEvent(pumpIndex, "START");
    } else if (state == PUMP_OFF) {
        logPumpEvent(pumpIndex, "STOP");
    }
}



void togglePumpState(int pumpIndex) {
    if (pumpIndex < 0 || pumpIndex >= 10) return; // Validate index
    int newState = (pumpStates[pumpIndex] == PUMP_ON) ? PUMP_OFF : PUMP_ON;
    setPumpState(pumpIndex, newState);
    }


void applyPumpMode(int pumpIndex) {
    if (pumpModes[pumpIndex] == PUMP_AUTO) {
        // In AUTO mode, the actual state is determined by external conditions like temperature
        Serial.println("Pump " + String(pumpIndex + 1) + " is set to AUTO mode.");
        checkAutoModeConditions(pumpIndex);
    } else if (pumpModes[pumpIndex] == PUMP_ON) {
        setPumpState(pumpIndex, PUMP_ON);
        Serial.println("Pump " + String(pumpIndex + 1) + " is set to ON.");
    } else if (pumpModes[pumpIndex] == PUMP_OFF) {
        setPumpState(pumpIndex, PUMP_OFF);
        Serial.println("Pump " + String(pumpIndex + 1) + " is set to OFF.");
    }

    // Broadcasting the new mode state for the specific pump
    broadcastPumpState(pumpIndex);
}


void setPumpMode(int pumpIndex, int mode) {
    // Set the mode
    pumpModes[pumpIndex] = mode;

    // Apply the mode immediately
    if (mode == PUMP_AUTO) {
        checkAutoModeConditions(pumpIndex);
    } else {
        setPumpState(pumpIndex, mode == PUMP_ON ? PUMP_ON : PUMP_OFF);
    }

    // Broadcast the state change
    broadcastPumpState(pumpIndex);
}


void executecheckAutoModeConditions() {
        for (int i = 0; i < numPumps; i++) {
            checkAutoModeConditions(i);
        }
    }

void checkAutoModeConditions(int pumpIndex) {
    if (pumpModes[pumpIndex] == PUMP_AUTO) {
        bool shouldTurnOn = simulatedTemperatures[pumpIndex] > temperatureThresholds[pumpIndex];
        int newState = shouldTurnOn ? PUMP_ON : PUMP_OFF;

        // Check if the state actually needs to change to minimize redundant operations
        if (pumpStates[pumpIndex] != newState) {
            Serial.print("Signal arrived at checkAutoModeConditions for Pump ");
            Serial.print(pumpIndex + 1);
            Serial.println(".");

            setPumpState(pumpIndex, newState);
            Serial.println("Pump " + String(pumpIndex + 1) + " AUTO mode: Turned " + (newState == PUMP_ON ? "ON" : "OFF") + " due to " + (shouldTurnOn ? "high" : "low") + " temperature.");

            // Broadcast the new state for this specific pump
            broadcastPumpState(pumpIndex);
        }
    }
}

void initializePumps() {
    for (int i = 0; i < 10; i++) {
        pinMode(pumpPins[i], OUTPUT);
        digitalWrite(pumpPins[i], pumpOffStateHigh[i] ? HIGH : LOW);
    }
}
