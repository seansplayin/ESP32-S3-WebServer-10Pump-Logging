
#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws;

void initWebSocket();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void broadcastMessageOverWebSocket(const String& message);
void startServer();
void setupLogDataRoute();
String prepareLogData(int pumpIndex, String timeframe);
String aggregateDailyLogsReport(int pumpIndex);
String aggregateMonthlyLogsReport(int pumpIndex);
String aggregateYearlyLogsReport(int pumpIndex);
String aggregateDecadeLogsReport(int pumpIndex);


#endif // WEBSERVERMANAGER_H
