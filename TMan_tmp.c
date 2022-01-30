/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <xc.h>

/* Kernel includes */
#include "FreeRTOS.h"
#include "task.h"

/* App includes */
#include "../UART/uart.h"

#include "TMan.h"

#define SCH_PRIO    60


/* Global Variables */
int tickrate;
int idx = 0;
Task* tasks;


//struct _TMan {
//    Task* tasks; // = (Task*) pvPortMalloc(sizeof(Task)*16);      // support for 16 tasks max
//    //int* tasks_ids = (int*) pvPortMalloc(sizeof(int*)*16);
//    int index; // = 0
//    int tickrate;   
//};


struct _Task {
    char* Task_name;
    int Task_id;
    int Task_period;
    int Task_deadline;
    int Task_phase;
    int Task_nact;
    int Task_nmiss;
    TickType_t xLastExecutionTime;
    char Task_status;
    TaskHandle_t Task_handle;
};



void TMAN_Init(int tick) {
    
    //GetTickCount();
//    TMan* TMan = (TMan*) pvPortMalloc(sizeof(TMan));
//    if(TMan == NULL) exit(1);
    
    tickrate = tick;
    
    tasks = (Task*) pvPortMalloc(sizeof(Task)*16);           // support for 16 tasks
    if (tasks == NULL) exit(1);
    
    xTaskCreate( TMAN_Scheduler, ( const signed char * const ) "Scheduler", configMINIMAL_STACK_SIZE, (void*) tick , SCH_PRIO, NULL );
    
    
//    return 0;
}

void TMAN_Close() {
    vPortFree(tasks);
    tasks = NULL;
    //*TMan = NULL;
}

Task* TMAN_Get_Task(const signed char* name){
    Task* task;
    
    //comentei pq estava a dar erro a compilar
    
    //task.Task_handle = xTaskGetHandle(name);
    //for(int i = 0; i < (sizeof(TMan->tasks/TMan->tasks[0])); i++){
    //    if (TMan->tasks[i].Task_handle == name){
    //        return TMan.tasks[i];
    //    }
    //}
    
    return task;
}

int TMAN_TaskAdd(const signed char * name) {
    Task* task = (Task*) pvPortMalloc(sizeof(Task));
    strcpy(task->Task_name, name); 
    task->Task_handle = xTaskGetHandle(name);
    
    
    tasks[idx++] = *task;
    
}

void TMAN_AddRegisterAttributes(const signed char * name, int period, int deadline) {
    Task* task = TMAN_Get_Task(name);
    task->Task_period = period;
    task->Task_deadline = deadline;
    
}

int TMAN_TaskWaitPeriod(const signed char * name) {
    
    //provavelmente deve ser preciso fazer aqui uma verificação qualquer antes de suspender
    
    vTaskSuspend(xTaskGetHandle(name));
}

void TMAN_TaskStats(const signed char * name) {
    
}


void TMAN_Scheduler(void* PvParameters) {
    const TickType_t period = (*(int*) PvParameters) / portTICK_PERIOD_MS;
    
    for(;;) {
        
        //fazer o check das tasks que têm de ser ativas
        
        vTaskDelay(period);
    }
}