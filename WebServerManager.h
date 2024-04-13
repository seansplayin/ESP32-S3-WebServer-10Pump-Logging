
#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws;
void setupRoutes();
void initWebSocket();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void broadcastMessageOverWebSocket(const String& message);
void startServer();
void setupLogDataRoute();
void updateAllRuntimes();

String prepareLogData(int pumpIndex, String timeframe);
unsigned long aggregateDailyLogsReport(int pumpIndex);
unsigned long aggregateMonthlyLogsReport(int pumpIndex, unsigned long dayRuntimeSeconds);
unsigned long aggregateYearlyLogsReport(int pumpIndex, unsigned long monthRuntimeSeconds);
unsigned long aggregateDecadeLogsReport(int pumpIndex, unsigned long yearRuntimeSeconds);


#endif // WEBSERVERMANAGER_H
