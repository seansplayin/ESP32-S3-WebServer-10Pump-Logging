#ifndef PUMPMANAGER_H
#define PUMPMANAGER_H

#include "Config.h"


void broadcastPumpState(int pumpIndex = -1);
void setPumpState(int pumpIndex, int state);
void togglePumpState(int pumpIndex);
void applyPumpMode(int pumpIndex);
void setPumpMode(int pumpIndex, int mode);
void checkAutoModeConditions(int pumpIndex);
void setupAutoModeChecking();
void initializePumps();
void setupPumpBroadcasting();



extern bool pumpOnStateHigh[10];
extern bool pumpOffStateHigh[10];
extern const int numPumps;
extern int pumpModes[10];
extern int pumpStates[10];
extern const int pumpPins[10];




#endif // PUMPMANAGER_H
