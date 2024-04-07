// list logs, read log "filename"
// partition.csv edited to 1.2 mb for each app partition and 
// working : Webserver, pumps On/Off,Auto with simulated temperature T,1,25, NTP>RTC sync. 
// transfered RTClib-NTP_Sync4 code to ESPAsyncWebServer46-10Pumps7.6 
// Removed duplicate logging functions, 'logPumpStart(pumpIndex);' and 'logPumpStop(pumpIndex);' and now logPumpEvent function does both when 
// specifying "START" or "STOP".  replaced calls for 'logPumpStart(pumpIndex);' and 'logPumpStop(pumpIndex);' in 'setPumpState' function with
// 'logPumpEvent(pumpIndex, "START");' and 'logPumpEvent(pumpIndex, "STOP");'
// Verified log files for pumps
// update ESPAsyncWebServer46-10Pumps7.6 : RTC is no longer updated if NTP update fails. NTP function "void initNTP()" 
// now updateds RTC but not if NTP update fails and logging will continue using the time inside the DS3231 RTC.
// ESPAsyncWebServer46-10Pumps7.7 : updated initNTP and tryNtpUpdate functions so no longer uses blocking code and 
// the rest of the setup code can continue while the time attempts to sync. 
// update ESPAsyncWebServer46-10Pumps7.8 : Code split up and put into siloh's. Logging is not adding timestamp
// update ESPAsyncWebServer46-10Pumps7.9 : Logs now properly display the date and time
// update ESPAsyncWebServer46-10Pumps8.0 : Added three logging functions,
// aggregatePumptoDailyLogs to read PumpX_Log.txt and calculate run times then record runtime total into PumpX_Daily.txt
// and then delete the original start and stop times in the source PumpX_Log.txt file. 
// If a value already exists in the PumpX_Daily.txt file the new value needs to be aggregated with the existing runtime value. but is not happening.
// update ESPAsyncWebServer46-10Pumps8.1 : resolved, existing time values from the same date in PumpX_Daily.txt are now added to the new values before being overwritten.
// update ESPAsyncWebServer46-10Pumps8.2 : resolved, existing time values from the same month in PumpX_Monthly are now added to the new values before being overwritten
// String currentYear = String(getCurrentYear());
// update ESPAsyncWebServer46-10Pumps8.4 : All three logging aggregation functions now work. 
// ESPAsyncWebServer46-10Pumps8.5 : using 'aggAll' from serial input calls performLogAggregation()function which successfully aggregates data from 
// pumpX_Log.txt to pumpX_Daily.txt for the entire array. This is the first function of the three functions inside,
// "aggregatePumptoDailyLogs(i);" "aggregateDailyToMonthlyLogs(i);" "aggregateMonthlyToYearlyLogs(i);" verified each works when called independently. 
// ESPAsyncWebServer46-10Pumps8.6 : added functions checkAndSetFlag(); to set Elapsed_Day flag to 'true' & checkAndPerformAggregation(); so if daily flag is true it will call the 
// performLogAggregation(); and then set the Elapsed_Day flag back to 'false'
// ESPAsyncWebServer46-10Pumps8.7 : time is now sent out every second through websocket. 
// Modified "on event" in the java script portion of WebServerManager.cpp to display time and reformat it to MM-DD-YYYY 00:00:00AM/PM
// ESPAsyncWebServer46-10Pumps8.8 : moved webpage code from WebServerManager.cpp to FirstWebpage.cpp and SecondWebpage.cpp files. second webpage is at /second
// ESPAsyncWebServer46-10Pumps8.9 : modified handleWebSocketMessage function to accept "requestLogData" messages 
// ESPAsyncWebServer46-10Pumps8.9 : added functions for String prepareLogData(int pumpIndex, String timeframe), aggregateDailyLogsReport, aggregateMonthlyLogsReport, aggregateYearlyLogsReport, and aggregateDecadeLogsReport
// ESPAsyncWebServer46-10Pumps9.0 : Modified secondWebpage.cpp so it now shows the pumps and has a drop down selector box as well as a graph. still need to finish linking the functions we built in 8.9
// ESPAsyncWebServer46-10Pumps9.1 : Modified all time functions in the RTCManager.cpp file so they now reference the system variable "CurrentTime" to eliminate duplicate rtc.now(); calls to the RTC.
// ESPAsyncWebServer46-10Pumps9.2 : Fixed automatic log aggregation with just three functions (checkTimeAndAct();, void setElapsed_Day(), void setperformLogAggregation()in the Logging.cpp file using the
// refreshCurrentTime()function being called every second form the RTCManager.cpp file to call checkTimeAndAct();
// ESPAsyncWebServer46-10Pumps9.3 : Modified log function "aggregatePumptoDailyLogs" function in logging.cpp file so runtime from ongoing pump operations before and after midnight will be handled. 
// ESPAsyncWebServer46-10Pumps9.3 : Modified Report functions in WebServerManager for: aggregateDailyLogsReport, aggregateMonthlyLogsReport, aggregateYearlyLogsReport, aggregateDecadeLogsReport
// ESPAsyncWebServer46-10Pumps9.4 : modified the ws.onmessage function in FirstWebpage.cpp file in the script and webpage pump state works again.
// ESPAsyncWebServer46-10Pumps9.5 : Modified initWebSocket()function in WebServerManager.cpp so when a new client connects it trigers the WS_EVT_CONNECT and refreshes the pump state/Mode through ws so new clients display the current pump status
// ESPAsyncWebServer46-10Pumps9.6 : renamed "Decade" to "Total" in log dropdown menu of /second-page, removed TimeSync ticker and added 'checkAndSyncTime()' in TimeSync.cpp, function called every second by 'refreshCurrentTime()' in RTCManager.cpp
// ESPAsyncWebServer46-10Pumps9.7 : 
