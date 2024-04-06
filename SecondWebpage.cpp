#include <ESPAsyncWebServer.h>
#include "SecondWebpage.h"
#include <ArduinoJson.h> // Make sure you have this library for JSON handling


extern AsyncWebServer server; // Ensure server is declared somewhere globally

#include <ArduinoJson.h> // Make sure you have this library for JSON handling


// HTML content of the second page stored as a raw string literal
const char* secondPageHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<link rel="icon" type="image/png" sizes="48x48" href="/favicon.png">
<link rel="icon" type="image/png" sizes="16x16" href="/favicon16.png">
  <link rel="icon" type="image/png" sizes="32x32" href="/favicon32.png">
  <link rel="icon" type="image/png" sizes="48x48" href="/favicon48.png">
  <title>ESP Web Server/Logs</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        .pump-row { display: flex; align-items: center; padding: 10px; }
        .pump-name { flex: 1; }
        .pump-data { flex: 2; display: flex; align-items: center; }
        .pump-graph { flex: 3; height: 20px; background-color: lightgrey; position: relative; }
        .runtime-bar { position: absolute; height: 100%; background-color: green; }
        select { margin-right: 20px; }
    </style>
</head>
<body>
    <div id="pumpDataContainer"></div>
    <script>
    // JavaScript to dynamically create pump rows and fetch log data
    const pumps = 10; // Assuming 10 pumps
    const pumpDataContainer = document.getElementById('pumpDataContainer');

    for (let i = 1; i <= pumps; i++) {
        const pumpRow = document.createElement('div');
        pumpRow.className = 'pump-row';
        pumpRow.innerHTML = `
            <div class="pump-name">Pump ${i}</div>
            <div class="pump-data">
                <select onchange="fetchAndDisplayData(${i}, this.value)">
                    <option value="" disabled selected>Select</option>
                    <option value="day">Current Day</option>
                    <option value="month">Current Month</option>
                    <option value="year">Current Year</option>
                    <option value="decade">Current Total</option>
                </select>
                <div class="runtime-display">Runtime: --</div>
            </div>
            <div class="pump-graph"><div class="runtime-bar" style="width: 0%;"></div></div>
        `;
        pumpDataContainer.appendChild(pumpRow);
    }

    function fetchAndDisplayData(pumpIndex, timeframe) {
        console.log(`Fetching data for pump ${pumpIndex} with timeframe ${timeframe}`);
        fetch(`/get-log-data?pumpIndex=${pumpIndex}&timeframe=${timeframe}`)
            .then(response => response.json())
            .then(data => {
                // Assuming 'data' is an object containing 'runtime' and 'percentage' properties
                const runtimeDisplay = document.querySelector(`.pump-row:nth-child(${pumpIndex}) .runtime-display`);
                const runtimeBar = document.querySelector(`.pump-row:nth-child(${pumpIndex}) .runtime-bar`);
                
                runtimeDisplay.textContent = `Runtime: ${data.runtime}`;
                runtimeBar.style.width = `${data.percentage}%`;
            })
            .catch(error => console.error('Error fetching log data:', error));
    }
</script>

</body>
</html>
)rawliteral";

void setupSecondPageRoutes() {
    server.on("/second-page", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", secondPageHtml);
    });
}
