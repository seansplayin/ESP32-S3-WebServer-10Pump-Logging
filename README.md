# ESP32-S3-AsyncWebServer-10Pump-Logging

*potential uses*

Control up to 10 relays from a webpage, view current relay states and select relay modes (on, off, auto). 
- Add temperature sensors (ds18b20 or thermistors) and control up to 10 zones of infloor radiant heat using "zone valves" or forced air heat using "dampers".  


*Overview*

This Arduino IDE project uses a W5500 Ethernet adapter connected to the ESP32-S3-DevkitC-1 mcu for network connectivity. 
Controller hosts a multi client asynchronously updated web server with the first webpage displaying the state of all 10 pumps (relays/ output pins) using websocket and a user selectable mode changer (Auto, On, Off).
These output pins can be easily and individually configured on=high/off=low or on=low/off=high to suite your application. (LED typically configures as on=high/off=low and relays can be configured as on=low/off=high or 
on=High/off=Low)
System utilizes a DS3231 RTC that is synchronized with NTP -6 (MST7MDT) on startup and every day at 3AM. 
System time is broadcast every second through websocket and 'CurrentTime' is available system wide to reduce duplication of RTC calls rtc.now() to the DS3231 and to ensure all times are in sync. 
System uses littlefs to hold favicon icons and all logs.
Implemented full logging system to track pump run times on a per day, month, year basis along with a logging scheme that avoids increasing log file sizes except for a single line added every year for the yearly total. 
The functions that start and stop the pumps also creates a log file if it doesn't exist whith a name that corresponding to the pump ie. pump1 is logged in pump1_Log.txt and then logs the start and stop events with a 
timestamp.
at midnight a function is called that aggregates the runtimes from the pumpX_Logs.txt and records into the pumpX_Daily.txt logs and then clears/deletes the source pumpX_Logs.txt files 
If it is the first day of the mnonth an additional log aggregation function will called that aggregates the runtimes from pumpX_Daily.txt logs into pumpX_Monthly.txt logs and then deletes the source pumpX_Daily.txt logs. 
If it's the first day of the year an additional log aggregation function is called that aggregates the runtimes from the pumpX_Monthly.txt logs into the pumpX_Yearly.txt logs and deletes the source pumpXMonthly.txt logs. 
A second webpage is available at /second-page where elapsed pump runtimes can be viewed for the current day, month, year, total 
For organization code is split up into many separate files based on function

Type "list files" into serial input to display contents of LittleFS file system


*Version History*

06-02-24: github version incremented from v10.5 to v11

// ESPAsyncWebServer46-10Pumps7.6 : working : Webserver, pumps On/Off,Auto with simulated temperature T,1,25, NTP>RTC sync. 
// ESPAsyncWebServer46-10Pumps7.6 : RTC is no longer updated if NTP update fails. NTP function "void initNTP()" 
// now updateds RTC but not if NTP update fails and logging will continue using the time inside the DS3231 RTC. 
// ESPAsyncWebServer46-10Pumps7.6 : transfered RTClib-NTP_Sync4 code to ESPAsyncWebServer46-10Pumps7.6. 
// ESPAsyncWebServer46-10Pumps7.6 : Removed duplicate logging functions, 'logPumpStart(pumpIndex);' and 'logPumpStop(pumpIndex);' and now logPumpEvent function does both when 
// specifying "START" or "STOP".  
// ESPAsyncWebServer46-10Pumps7.6 : replaced calls for 'logPumpStart(pumpIndex);' and 'logPumpStop(pumpIndex);' in 'setPumpState' function with 'logPumpEvent(pumpIndex, "START");' and 'logPumpEvent(pumpIndex, "STOP");'
// ESPAsyncWebServer46-10Pumps7.6 : Verified log files for pumps

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
// ESPAsyncWebServer46-10Pumps9.7 : CheckAutoMode removed from ticker and now called every second via 'executeEverySecond()' which is called by 'refreshCurrentTime()' in RTCManager.cpp
// ESPAsyncWebServer46-10Pumps9.8 : Changed File System from using SPIFFS to using LittleFS. Modified InitializeFileSystem() function to support LittleFS and added option to Automatically format if LittleFS Mount Fail if '// LittleFSformat();' is uncommented.
// ESPAsyncWebServer46-10Pumps9.9 : rebuilt functions 'aggregateDailyLogsReport, aggregateMonthlyLogsReport, aggregateYearlyLogsReport, aggregateDecadeLogsReport' but decided stratagy was inefficient and abandoned this sketch.
// ESPAsyncWebServer46-10Pumps10.0 : built 'updateAllRuntimes()' function and rebuilt second page functions 'aggregateDailyLogsReport, aggregateMonthlyLogsReport, aggregateYearlyLogsReport, aggregateDecadeLogsReport' to efficiently report pump runtimes.
// ESPAsyncWebServer46-10Pumps10.1 : second webpage reporting of pump runtimes appears to work properly. Discovered the functions aggregateDailyToMonthlyLogs and aggregateMonthlyToYearlyLogs are not working
// after switching to LittleFS. rewrote both of these functions and they are now working properly.
// ESPAsyncWebServer46-10Pumps10.2 : Added ws.send('updateAllRuntimes'): into addEventListener function in secondwebpage.cpp so log data display updates when client connects.
// ESPAsyncWebServer46-10Pumps10.3 : added log file downloads to second webpage
// ESPAsyncWebServer46-10Pumps10.4 : modification to html and java script function in SecondWebpage.cpp. no change visible when using webpage
// ESPAsyncWebServer46-10Pumps10.5 : again modified the same html and java script function in SecondWebpage.cpp and moved the 'Downloads Files' button to below the 'List Files' button
// ESPAsyncWebServer46-10Pumps10.6 : oraganized file declarations
// ESPAsyncWebServer46-10Pumps10.7 : Adding RTOS Tasks which now call all functions
// ESPAsyncWebServer46-10Pumps10.8 : modified tasks to include execution time into the delay time to accurately repeat and then verified stack size is adequate for all repeating tasks
// ESPAsyncWebServer46-10Pumps11.0 : modified calculateTotalLogRuntime function in WebServerManager.cpp file so elapsed runtime for pumps currently operating is adding into calculation for current day.