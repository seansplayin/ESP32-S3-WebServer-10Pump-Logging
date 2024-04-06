#include "TemperatureControl.h"
#include "Logging.h"
#include "WebServerManager.h"
#include "PumpManager.h"
#include "Config.h"

#include <Ticker.h>

float simulatedTemperatures[10] = {24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0}; // Initial simulated temperatures
float temperatureThresholds[10] = {24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5}; // Temperature thresholds

Ticker temperatureTicker;

void setupTemperatureBroadcasting() {
    temperatureTicker.attach(10, broadcastTemperatures); // Assume broadcastTemperatures is a suitable function within this file
}


void updateTemperature(int pumpIndex, float newTemperature) {
        if (abs(newTemperature - simulatedTemperatures[pumpIndex]) > 0.5) {
        simulatedTemperatures[pumpIndex] = newTemperature;
        Serial.println("Temperature updated for pump " + String(pumpIndex+1) + ": " + String(newTemperature));
        checkAutoModeConditions(pumpIndex); // Directly check conditions after updating temperature
        }
    }
    

void broadcastTemperatures() {
    String message = "Temperatures:";
    for (int i = 0; i < numPumps; i++) {
        message += " Pump" + String(i + 1) + "Sim:" + String(simulatedTemperatures[i]) + ",Thresh:" + String(temperatureThresholds[i]);
        if (i < numPumps - 1) message += ";";
        
        // Also log to the Serial Monitor
        Serial.print("Pump ");
        Serial.print(i + 1);
        Serial.print(". Current Temp: ");
        Serial.print(simulatedTemperatures[i]);
        Serial.print(", Threshold: ");
        Serial.println(temperatureThresholds[i]);
    }
    Serial.println(); // Print a blank line
    broadcastMessageOverWebSocket(message);
}
