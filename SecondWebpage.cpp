#include <ESPAsyncWebServer.h>
#include "SecondWebpage.h"
#include <ArduinoJson.h> // Make sure you have this library for JSON handling

extern AsyncWebServer server; // Ensure server is declared somewhere globally

// Updated HTML and CSS for the second page with improved layout
const char* secondPageHtml = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" sizes="48x48" href="/favicon.png">
    <title>ESP Web Server/Logs</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        #updateAllButton { margin-bottom: 20px; }
        #buttonContainer { text-align: center; }
        #filesContainer, #downloadFilesButton { display: none; margin-top: 10px; }
        #pumpGrid { display: grid; grid-template-columns: repeat(5, 1fr); text-align: left; margin: auto; max-width: 800px; }
        .grid-header { font-weight: bold; padding: 10px 0; }
        .grid-cell { padding: 5px 0; }
    </style>
</head>
<body>
    <h2>****Pump Runtimes****</h2>
    <button id="updateAllButton">Update All</button>
    <div id="pumpGrid">
        <div class="grid-header">Pump</div>
        <div class="grid-header">Today</div>
        <div class="grid-header">Current Month</div>
        <div class="grid-header">Current Year</div>
        <div class="grid-header">Total</div>
    </div>

    <div id="buttonContainer">
        <button id="listFilesButton">List Files</button><br>
        <button id="downloadFilesButton">Download Files</button>
    </div>
    <div id="filesContainer"></div>  <!-- Container for the file list -->

    <script>
        function formatRuntime(seconds) {
            const hours = Math.floor(seconds / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            const secs = seconds % 60;
            return `${hours}h ${minutes}m ${secs}s`;
        }

        document.addEventListener('DOMContentLoaded', function () {
            const ws = new WebSocket('ws://' + window.location.hostname + '/ws');
            const pumps = 10; // Number of pumps
            const pumpGrid = document.getElementById('pumpGrid');

            ws.onopen = function() {
                console.log('WebSocket connection established');
                ws.send('updateAllRuntimes');  // Automatically update all runtimes upon connection
            };

            ws.onmessage = function(event) {
                console.log('WebSocket message received:', event.data);
                if (event.data.startsWith("JSON:")) {
                    const data = JSON.parse(event.data.substring(5));
                    data.data.forEach(pump => {
                        document.getElementById(`pump${pump.pumpIndex}-day`).textContent = formatRuntime(pump.day);
                        document.getElementById(`pump${pump.pumpIndex}-month`).textContent = formatRuntime(pump.month);
                        document.getElementById(`pump${pump.pumpIndex}-year`).textContent = formatRuntime(pump.year);
                        document.getElementById(`pump${pump.pumpIndex}-total`).textContent = formatRuntime(pump.total);
                    });
                }
            };

            for (let i = 1; i <= pumps; i++) {
                const rowHTML = `<div class="grid-cell">Pump ${i}</div>
                                 <div class="grid-cell" id="pump${i}-day">--</div>
                                 <div class="grid-cell" id="pump${i}-month">--</div>
                                 <div class="grid-cell" id="pump${i}-year">--</div>
                                 <div class="grid-cell" id="pump${i}-total">--</div>`;
                pumpGrid.insertAdjacentHTML('beforeend', rowHTML);
            }

            document.getElementById('updateAllButton').addEventListener('click', function() {
                console.log("Updating all runtimes...");
                ws.send('updateAllRuntimes');
            });

            document.getElementById('listFilesButton').addEventListener('click', function() {
                const container = document.getElementById('filesContainer');
                const downloadButton = document.getElementById('downloadFilesButton');
                // Toggle visibility
                if (container.style.display === 'none' || container.style.display === '') {
                    fetchFileList();  // Call fetchFileList to update the file list
                    container.style.display = 'block';
                    downloadButton.style.display = 'inline'; // Show download button when listing files
                } else {
                    container.style.display = 'none';
                    downloadButton.style.display = 'none'; // Hide download button when not listing files
                }
            });

            function fetchFileList() {
                fetch('/list-logs')
                .then(response => response.json())
                .then(files => {
                    const list = document.getElementById('filesContainer');
                    list.innerHTML = ''; // Clear current list
                    files.forEach(file => {
                        const label = document.createElement('label');
                        const checkbox = document.createElement('input');
                        checkbox.type = 'checkbox';
                        checkbox.value = file;
                        label.appendChild(checkbox);
                        label.appendChild(document.createTextNode(file));
                        list.appendChild(label);
                        list.appendChild(document.createElement('br'));
                    });
                });
            }

            document.getElementById('downloadFilesButton').addEventListener('click', function() {
                downloadSelected();
            });

            function downloadSelected() {
                const checkboxes = document.querySelectorAll('#filesContainer input[type="checkbox"]:checked');
                checkboxes.forEach(checkbox => {
                    const file = checkbox.value;
                    window.open('/download-log?file=' + encodeURIComponent(file), '_blank');
                });
            }
        });
    </script>
</body>
</html>


)rawliteral";

void setupSecondPageRoutes() {
    server.on("/second-page", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", secondPageHtml);
    });
}
