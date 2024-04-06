#include "FileSystemManager.h"
#include "Logging.h" // Assuming logMessage() is declared here
#include "Config.h"


#include <SPIFFS.h>

void initializeFileSystem() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        // Optionally, handle the failure more gracefully instead of returning.
    } else {
        // Log success or perform additional setup if necessary
        Serial.println("SPIFFS mounted successfully");
    }
}
