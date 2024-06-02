#ifndef TASKMANAGER_H
#define TASKMANAGER_H

// Task function declarations
void TaskRTC(void *pvParameters);
void TaskNetwork(void *pvParameters);
void TaskTimeSync(void *pvParameters);
void TaskFileSystem(void *pvParameters);
void TaskPumps(void *pvParameters);
void TaskServer(void *pvParameters);
void TaskFirstPage(void *pvParameters);
void TaskSecondPage(void *pvParameters);
void TaskLogDataRoute(void *pvParameters);
void TaskDateTimeTicker(void *pvParameters);
void TaskUpdateTemperatures(void *pvParameters);

void startAllTasks();

#endif
