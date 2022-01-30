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

#define SCH_PRIO    tskIDLE_PRIORITY -4


/* Global Variables */
unsigned int tick = 0;
int tickrate;
int idx = 0;
Task* tasks;


struct _Task {
    char* Task_name;
    int Task_id;
    int Task_period;
    int Task_deadline;
    unsigned int Task_nextAct;
    
    TickType_t xLastExecutionTime;
    char Task_status;
    
    TaskHandle_t Task_handle;
};


//recebe a handle para a task que servir� de Scheduler e a tickrate
void TMAN_Init(TaskHandle_t scheduler, int tick) {
    tasks = (Task*) pvPortMalloc(sizeof(Task)*16);           // support for 16 tasks
    if (tasks == NULL) exit(1);

    tickrate = tick;
}

void TMAN_Close() {
    vPortFree(tasks);
    tasks = NULL;
    idx = 0;
    tickrate = 0;
}

Task* TMAN_Get_Task(const signed char* name){   
    
    for(int i = 0; i < idx; i++){
        if (strcmp(tasks[i].Task_name, name) == 0){
            return &tasks[i];
        }
    }
    
    return NULL;
}

//verifica se existe alguma task com esse nome, cria task
//devolve 1 se for bem sucedido, 0 se der erro
int TMAN_TaskAdd(const signed char * name) {
    for(int i = 0; i < idx; i++) {
        if (strcmp(tasks[i].Task_name, name) == 0) {
            return 0;
        }
    }
    
    Task task;
    TaskHandle_t hdl = xTaskGetHandle(name);
    
    PrintStr("task received");
    
    task.Task_handle = hdl;
    task.Task_name = pvPortMalloc(sizeof(char)*10);
    strcpy(task.Task_name, name);
    task.Task_status = 's';
    task.Task_id = idx;
    
    tasks[idx++] = task;
    
    return 1;
}

void TMAN_TaskRegisterAttributes(const signed char * name, int period, int deadline) {
    Task* task = TMAN_Get_Task(name);
    task->Task_period = period;
    task->Task_deadline = tick + task->Task_deadline;
    task->Task_nextAct = tick + task->Task_period;
    
    task->Task_status = 'p';
}

void TMAN_TaskWaitPeriod(const signed char * name) {
    Task* task = TMAN_Get_Task(name);
    if(task == NULL) return;
    task->Task_status = 's';
    
    //provavelmente deve ser preciso fazer aqui uma verifica��o qualquer antes de suspender
    
    vTaskSuspend(task->Task_handle);
}

void TMAN_TaskStats(const signed char * name) {
    
}

void TMAN_Scheduler(void* PvParameters) {
    PrintStr("Scheduler Started");
    
    const TickType_t period =  tickrate * portTICK_PERIOD_MS;
    
    uint8_t msg[40];    
     
    int maxPriorityIndex;
    for(;;) {
//        maxPriorityIndex = 0;

        
        for(int i = 0; i < idx; i++){         
            if(tasks[i].Task_nextAct == tick /*&& tasks[i].Task_status == 's'*/){
                tasks[i].Task_nextAct = tick + tasks[i].Task_period;
                tasks[i].Task_status = 'p'; 
//                    if (tasks[i]->Task_priority > maxPriorityIndex){
//                        maxPriorityIndex = i;
//                    }
            }
            if(tasks[i].Task_status == 'p'){
//                if (tasks[i]->Task_priority > maxPriorityIndex){
//                    maxPriorityIndex = i;
//                }
                
                vTaskResume(tasks[i].Task_handle);
                tasks[i].Task_status = 'r';
            }
        }
////        tasks[maxPriorityIndex]->Task_status = 'r';
//      
        
        vTaskDelay(period);
        tick++;
    }
    
}