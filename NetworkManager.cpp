#include "NetworkManager.h"
#include "Config.h"
#include "Logging.h"

#include <SPI.h>
#include <WebServer_ESP32_SC_W5500.h>



extern const int W5500_MOSI;
extern const int W5500_MISO;
extern const int W5500_SCK;
extern const int W5500_SS;
extern const int W5500_INT;

// Ethernet settings for W5500
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 20, 90, 33); 


// Initialize Ethernet connection
 void setupNetwork() {
    SPI.begin();
    ETH.begin(W5500_MISO, W5500_MOSI, W5500_SCK, W5500_SS, W5500_INT, 25, SPI3_HOST, mac);
    delay(2000);
    Serial.print(F("IP Address : "));
    Serial.println(ETH.localIP());
}
