#include "NetworkManager.h"
#include "Config.h"
#include "Logging.h"

#include <SPI.h>
#include <WebServer_ESP32_SC_W5500.h>

// Ethernet adapter (W5500) pin configurations
const int W5500_MOSI = 11;
const int W5500_MISO = 13;
const int W5500_SCK = 12;
const int W5500_SS = 10;
const int W5500_INT = 4;



// Ethernet settings for W5500
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 20, 90, 33); 


// Initialize Ethernet connection
 void setupNetwork() {
    SPI.begin();
    ETH.begin(W5500_MISO, W5500_MOSI, W5500_SCK, W5500_SS, W5500_INT, 25, SPI3_HOST, mac);
    vTaskDelay(pdMS_TO_TICKS(2000));  // Convert milliseconds to ticks
    Serial.print(F("IP Address : "));
    Serial.println(ETH.localIP());
}
