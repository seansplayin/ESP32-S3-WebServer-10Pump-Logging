#ifndef TIMESYNC_H
#define TIMESYNC_H

#include "Config.h"


void setupTimeSync();
void initNTP();
void tryNtpUpdate();
void printCurrentRtcTime();
void initializeTime();


extern bool needToSyncTime;
extern bool needNtpSync;
extern unsigned long lastNtpUpdateAttempt;
extern const unsigned long ntpRetryInterval;
extern bool isNtpSyncDue;


#endif
