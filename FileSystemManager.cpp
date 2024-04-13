#include "FileSystemManager.h"
#include "Logging.h" // Assuming logMessage() is declared here
#include "Config.h"
//#include <SPIFFS.h>
#include <LittleFS.h>


void LittleFSformat()  {
     if (LittleFS.format()) {
             Serial.println("Formatting LittleFS succeeded. Attempting to mount again...");
             if (LittleFS.begin()) {
                 Serial.println("LittleFS mounted successfully after formatting.");
             } else {
                 Serial.println("Mounting LittleFS failed even after formatting.");
             }
         } else {
             Serial.println("Formatting LittleFS failed.");
         }
}
  

void initializeFileSystem() {
    // Attempt to mount LittleFS. If fail, provide instructions for manual formatting.
        Serial.println("Attempting to mount LittleFS file system.");
    if (!LittleFS.begin()) {
        Serial.println("Mounting LittleFS failed. If you wish to format the filesystem to LittleFS,");
        Serial.println("uncomment the 'LittleFS.format()' line in the 'initializeFileSystem()' function");
        Serial.println("in the FileSystemManager.cpp file and re-upload your sketch.");
        // Uncomment the next line to enable formatting LittleFS automatically. Use with caution.

        // LittleFSformat();
         
        return; // Exit the function since mounting failed
    }
    Serial.println("LittleFS mounted successfully.");
}
