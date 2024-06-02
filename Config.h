#ifndef CONFIG_H
#define CONFIG_H

#include <RTClib.h>


// W5500 Ethernet adapter declarations
extern const int W5500_MOSI;
extern const int W5500_MISO;
extern const int W5500_SCK;
extern const int W5500_SS;
extern const int W5500_INT;


// RTC DS3231
extern RTC_DS3231 rtc;
extern const int pinSDA;
extern const int pinSCL;
extern const int sqwPin;


// Pump declarations
extern const int pumpPins[10];
extern int pumpModes[10]; // Mode of each pump
extern int pumpStates[10]; // State of each pump


// Pump state variables
#define PUMP_OFF 0
#define PUMP_ON 1
#define PUMP_AUTO 2



#endif // CONFIG_H
