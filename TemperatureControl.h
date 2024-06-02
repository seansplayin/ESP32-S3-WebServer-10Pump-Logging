#ifndef TEMPERATURECONTROL_H
#define TEMPERATURECONTROL_H

#include "Config.h"


extern float simulatedTemperatures[10]; // Declare these as external to allow access from other files
extern float temperatureThresholds[10];

void updateTemperature(int pumpIndex, float newTemperature); // Updates the temperature for a specified pump and checks auto mode conditions if necessary
void broadcastTemperatures(); // Broadcasts the current temperatures and thresholds for all pumps
void setupTemperatureBroadcasting();
void updateTemperatures();

#endif // TEMPERATURECONTROL_H
