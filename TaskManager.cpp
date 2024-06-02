#include "TaskManager.h"
#include "FileSystemManager.h"
#include "FirstWebpage.h"
#include "Logging.h"
#include "NetworkManager.h"
#include "PumpManager.h"
#include "RTCManager.h"
#include "SecondWebpage.h"
#include "TemperatureControl.h"
#include "TimeSync.h"
#include "WebServerManager.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


// Task handles for synchronization (add new handles as needed)
TaskHandle_t thSetupRTC = NULL;
TaskHandle_t thSetupNetwork = NULL;
TaskHandle_t thInitNTP = NULL;
TaskHandle_t thInitFileSystem = NULL;
TaskHandle_t thInitPumps = NULL;
TaskHandle_t thStartServer = NULL;
TaskHandle_t thSetupFirstPage = NULL;
TaskHandle_t thSetupSecondPage = NULL;
TaskHandle_t thSetupLogDataRoute = NULL;
TaskHandle_t thDateTimeTicker = NULL;
TaskHandle_t thUpdateTemperatures = NULL;  
TaskHandle_t thcheckTimeAndAct = NULL;
TaskHandle_t thcheckAndSyncTime = NULL;
TaskHandle_t thcheckAutoModeConditions = NULL;


void TaskSetupRTC(void *pvParameters) {
    setupRTC();  // Initialize the RTC
    xTaskNotifyGive(thSetupNetwork);  // Notify the next task (make sure thSetupNetwork is correctly defined)
    vTaskSuspend(NULL);  // Suspend this task indefinitely
}


void TaskSetupNetwork(void *pvParameters) {
    // Wait indefinitely for the notification from the previous task to proceed
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    setupNetwork();  // Setup network configurations
    xTaskNotifyGive(thInitNTP);  // Notify the next task
    vTaskSuspend(NULL);  // Suspend this task indefinitely
}

void TaskInitNTP(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    initNTP();
    xTaskNotifyGive(thInitFileSystem);
    vTaskSuspend(NULL);
}

void TaskInitFileSystem(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    initializeFileSystem();
    xTaskNotifyGive(thInitPumps);
    vTaskSuspend(NULL);
}

void TaskInitPumps(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    initializePumps();
    xTaskNotifyGive(thStartServer);
    vTaskSuspend(NULL);
}

void TaskStartServer(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    startServer();
    xTaskNotifyGive(thSetupFirstPage);
    vTaskSuspend(NULL);
}

void TaskSetupFirstPage(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    setupFirstPageRoutes();
    xTaskNotifyGive(thSetupSecondPage);
    vTaskSuspend(NULL);
}

void TaskSetupSecondPage(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    setupSecondPageRoutes();
    xTaskNotifyGive(thSetupLogDataRoute);
    vTaskSuspend(NULL);
}

void TaskSetupLogDataRoute(void *pvParameters) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    setupLogDataRoute();
    xTaskNotifyGive(thDateTimeTicker);
    vTaskSuspend(NULL);
}

// Task for dateTimeTicker functionality
void TaskDateTimeTicker(void *pvParameters) {
     TickType_t xLastWakeTime = xTaskGetTickCount(); // Get the current tick count. 
     for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        refreshCurrentTime();  
        }
}

// task to request new temperatures from DS18B20 & PT1000
void TaskUpdateTemperatures(void *pvParameters) {
     TickType_t xLastWakeTime = xTaskGetTickCount(); // Get the current tick count.
    for (;;) {
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        updateTemperatures();
        }
}

// task to check if it's time to execute logging functions
void TaskcheckTimeAndAct(void *pvParameters) {
     TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        checkTimeAndAct(); 
       }
}

// task called check if it's time to sync NTP time
void TaskcheckAndSyncTime(void *pvParameters) {
     TickType_t xLastWakeTime = xTaskGetTickCount(); 
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        checkAndSyncTime();
       }
}

// task called to determine if pumps in auto mode cycle on | off   
void TaskcheckAutoModeConditions(void *pvParameters) {
     TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        executecheckAutoModeConditions();
       }
}

//example to record high water memory usage and report remaining stack size
/*
void TaskcheckAndSyncTime(void *pvParameters) {
    // Get the current tick count for delay calculation.
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));

        // Perform the primary function of the task.
        checkAndSyncTime();

        // Check the remaining stack space.
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        Serial.print("Remaining Stack Space: ");
        Serial.println(uxHighWaterMark);
    }
}
*/


// Initialize all tasks
void startAllTasks() {
    xTaskCreate(TaskSetupRTC, "SetupRTC", 2048, NULL, 1, &thSetupRTC);
    xTaskCreate(TaskSetupNetwork, "SetupNetwork", 2048, NULL, 1, &thSetupNetwork);
    xTaskCreate(TaskInitNTP, "InitNTP", 2048, NULL, 1, &thInitNTP);
    xTaskCreate(TaskInitFileSystem, "InitFileSystem", 4096, NULL, 1, &thInitFileSystem);
    xTaskCreate(TaskInitPumps, "InitPumps", 2048, NULL, 1, &thInitPumps);
    xTaskCreate(TaskStartServer, "StartServer", 2048, NULL, 1, &thStartServer);
    xTaskCreate(TaskSetupFirstPage, "SetupFirstPage", 2048, NULL, 1, &thSetupFirstPage);
    xTaskCreate(TaskSetupSecondPage, "SetupSecondPage", 2048, NULL, 1, &thSetupSecondPage);
    xTaskCreate(TaskSetupLogDataRoute, "SetupLogDataRoute", 2048, NULL, 1, &thSetupLogDataRoute);
    xTaskCreate(TaskDateTimeTicker, "DateTimeTicker", 2048, NULL, 2, &thDateTimeTicker);
    xTaskCreate(TaskUpdateTemperatures, "UpdateTemperatures", 2048, NULL, 1, &thUpdateTemperatures);  
    xTaskCreate(TaskcheckTimeAndAct, "checkTimeAndAct", 4096, NULL, 2, &thcheckTimeAndAct);
    xTaskCreate(TaskcheckAndSyncTime, "checkAndSyncTime", 4096, NULL, 2, &thcheckAndSyncTime);
    xTaskCreate(TaskcheckAutoModeConditions, "checkAutoModeConditions", 2048, NULL, 2, &thcheckAutoModeConditions);
    
}

// Note: Adjust priorities as needed based on your system's specific requirements
