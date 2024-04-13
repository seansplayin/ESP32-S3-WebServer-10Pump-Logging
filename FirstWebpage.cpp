#include <ESPAsyncWebServer.h>
#include "WebServerManager.h"
#include "FirstWebpage.h"

const char firstPageHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <link rel="icon" type="image/png" sizes="48x48" href="/favicon.png">
  <link rel="icon" type="image/png" sizes="16x16" href="/favicon16.png">
  <link rel="icon" type="image/png" sizes="32x32" href="/favicon32.png">
  <link rel="icon" type="image/png" sizes="48x48" href="/favicon48.png">
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 0; }
    .container { padding: 20px; }
    .pump { margin-bottom: 10px; }
  </style>
</head>
<body>
  <div class="container">
    <h2>ESP Web Server</h2>
    <p><span id="currentDateTime">--:--</span></p>
    <div id="pumps"></div>
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', function () {
  var ws = new WebSocket('ws://' + window.location.hostname + '/ws');
  ws.onopen = function() {
    console.log('WebSocket connection established');
  };

 ws.onmessage = function(event) {
    console.log('WebSocket message received:', event.data);

    // Check if the message is a time update
    if (event.data.includes("-")) {
        // Parse the received time string
        var dateTimeParts = event.data.split(" ");
        var dateParts = dateTimeParts[0].split("-");
        var timeParts = dateTimeParts[1].split(":");

        // Convert 24-hour time to 12-hour format
        var hours = parseInt(timeParts[0], 10);
        var suffix = hours >= 12 ? "PM" : "AM";
        hours = ((hours + 11) % 12 + 1); // Convert to 12-hour format
        var formattedTime = ("0" + hours).slice(-2) + ":" + timeParts[1] + ":" + timeParts[2] + " " + suffix;

        // Reformat the date and time (Example: MM/DD/YYYY 12-hour format with AM/PM)
        var formattedDateTime = dateParts[1] + "-" + dateParts[2] + "-" + dateParts[0] + " " + formattedTime;

        // Update the webpage
        document.getElementById("currentDateTime").textContent = formattedDateTime;
    } else {
        // Handle pump state and mode updates
        var parts = event.data.split(',');
        parts.forEach(function(part) {
          var keyValue = part.split(':');
          if (keyValue[0].startsWith('pump') && keyValue[0].endsWith('State')) {
            document.getElementById(keyValue[0]).textContent = keyValue[1];
          } else if (keyValue[0].startsWith('pump') && keyValue[0].endsWith('Mode')) {
            var selectElement = document.getElementById(keyValue[0] + 'Select');
            if (selectElement) {
              selectElement.value = keyValue[1].charAt(0).toUpperCase() + keyValue[1].slice(1);
            }
          }
        });
    }
};




  // Function to send pump mode changes
  function changePumpMode(pumpIndex, mode) {
    console.log(`Changing pump ${pumpIndex} mode to: ${mode}`);
    ws.send(`setPumpMode:${pumpIndex}:${mode}`);
  }

  window.changePumpMode = changePumpMode;

  // Generate pump controls
  function generatePumpControls() {
    var pumpsContainer = document.getElementById('pumps');
    for (var i = 1; i <= 10; i++) {
      var pumpDiv = document.createElement('div');
      pumpDiv.className = 'pump';
      pumpDiv.innerHTML = `<p>Pump ${i} State: <span id="pump${i}State">off</span></p>` +
                          `<p>Pump ${i} Mode: <select id="pump${i}ModeSelect" onchange="changePumpMode(${i}, this.value)">` +
                          `<option value="Auto">Auto</option>` +
                          `<option value="On">On</option>` +
                          `<option value="Off">Off</option>` +
                          `</select></p>`;
      pumpsContainer.appendChild(pumpDiv);
    }
  }

  generatePumpControls();
  // Additional javascript code goes here
});
  </script>
</body>
</html>
)rawliteral";

void setupFirstPageRoutes() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", firstPageHtml);
    });
}
