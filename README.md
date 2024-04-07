# ESP32-S3-AsyncWebServer-10Pump-Logging
This Arduino IDE project uses a W5500 Ethernet adapter connected to the ESP32-S3-DevkitC-1 mcu for network connectivity.
Controller hosts a multi client asynchronously updated web server with the first webpage displaying the state of all 10 pumps (relays/ output pins) using websocket and a user selectable mode changer (Auto, On, Off).
These output pins can be easily and individually configured on=high/off=low or on=low/off=high to suite your application. (LED typically configures as on=high/off=low and relays can be configured as on=low/off=high).
System utilizes a DS3231 RTC that is synchronized with NTP -6 (MST7MDT) on startup and every day at 3AM. 
System time is broadcast every second through websocket and 'CurrentTime' is available system wide to reduce duplication of RTC calls rtc.now() to the DS3231 and to ensure all times are in sync. 
System uses spiffs to hold favicon icons and all logs. This will soon be changed to littlefs
Implemented full logging system to track pump run times on a per day, month, year basis along with a logging scheme that avoids increasing log file sizes except for a single line added every year for the yearly total. 
the functions that starts and stops the pumps also creates a log file if it doesn't exist whith a name that corresponding to the pump ie. pump1 is logged in pump1_Log.txt and then logs the start and stop events.
at midnight a function is called that aggregates the runtimes from the pumpX_Logs.txt and records into the pumpX_Daily.txt logs and then clears/deletes the source pumpX_Logs.txt files 
If it is the first day of the mnonth an additional log aggregation function will called that aggregates the runtimes from pumpX_Daily.txt logs into pumpX_Monthly.txt logs and then deletes the source pumpX_Daily.txt logs. 
If it's the first day of the year an additional log aggregation function is called that aggregates the runtimes from the pumpX_Monthly.txt logs into the pumpX_Yearly.txt logs and deletes the source pumpXMonthly.txt logs. 
A second webpage is available at /second-page where elapsed pump runtimes can be viewed for the current day, month, year, total 
For organization code is split up into many separate files based on function
