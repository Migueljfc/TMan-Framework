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
    int Task_precedIdx;
    char * Task_preced;
    TaskHandle_t Task_handle;
    int Task_phase;
    int Task_nact;
    int Task_nmiss;
    int Task_blkRes;          //1- block resources 0 - free resources
};


//recebe a handle para a task que servirá de Scheduler e a tickrate
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
    
    uint8_t msg[40];
    sprintf(msg,"Task %s received\n\r", name);
    PrintStr(msg);
    
    task.Task_handle = hdl;
    task.Task_name = pvPortMalloc(sizeof(char)*10);
    strcpy(task.Task_name, name);
    task.Task_status = 's';
    task.Task_id = idx;
    task.Task_nact = 0;
    task.Task_preced = pvPortMalloc(sizeof(char)*10);
    tasks[idx++] = task;
    
    return 1;
}

void TMAN_TaskRegisterAttributes(const signed char * name, int period, int deadline, int phase, const signed char * precedent) {
    Task* task = TMAN_Get_Task(name);
    task->Task_period = period;
    task->Task_deadline = tick + deadline;
    task->Task_nextAct = tick + phase;
    task->Task_phase = phase;
    task->Task_blkRes = 1;
    task->Task_nmiss = 0;
    task->Task_preced = precedent;
    task->Task_precedIdx = (int) TMAN_Get_Index(precedent);
    task->Task_status = 'p';
}
int TMAN_Get_Index(const signed char * name){   
    for(int i = 0; i < idx; i++){
        if (strcmp(tasks[i].Task_name, name) == 0){
            return i;
        }
    }
    
    return 99;
}

void TMAN_TaskWaitPeriod(const signed char * name) {
    Task* task = TMAN_Get_Task(name);
    if(task == NULL) return;
    if (tick > task->Task_deadline) {
        PrintStr("Deadline miss!");
        task->Task_nmiss++;
    }
    //task->Task_blkRes = 0;
    task->Task_status = 's';
   
    vTaskSuspend(task->Task_handle);
}

void TMAN_TaskStats(const signed char * name) {
    uint8_t msg[40];
    int index = tasks[TMAN_Get_Index(name)].Task_precedIdx;
    sprintf(msg,"%s nAct = %d \n\r", name, tasks[index].Task_nact);
    PrintStr(msg);
}

void TMAN_Scheduler(void* PvParameters) {
    PrintStr("Scheduler Started\r\n");
    
    const TickType_t period =  tickrate * portTICK_PERIOD_MS;
    uint8_t msg[40];    
    int blocked ;

    for(;;) {
        for(int i = 0; i < idx; i++){
           
            
            if(tasks[i].Task_status == 'r') {
                tasks[i].Task_blkRes = 1;
            }
            else{
                if(tasks[i].Task_status == 's'){
                    tasks[i].Task_deadline = tick + tasks[i].Task_deadline;
                   //tasks[i].Task_blkRes = 0;
                }
                if(tasks[i].Task_nextAct == tick && tasks[i].Task_status == 's'){
                    tasks[i].Task_nextAct = tasks[i].Task_nextAct + tasks[i].Task_period + tasks[i].Task_phase;
                    tasks[i].Task_deadline = tick + tasks[i].Task_deadline;
                    if(tasks[i].Task_deadline - tick < 0){
                        PrintStr("Deadline Miss2\n\r");
                        tasks[i].Task_nmiss++;
                        continue;
                    }
                    tasks[i].Task_status = 'p'; 
                }
                else{
                    if(tasks[i].Task_precedIdx != 99){
                        blocked = tasks[i].Task_precedIdx;
                        blocked = tasks[blocked].Task_blkRes;
                        sprintf(msg,"%s - %d \n\r", tasks[i].Task_name, blocked);
                        PrintStr(msg);

                    }
                    else{
                        blocked = 0;
                    }
                    
                    if(tasks[i].Task_status == 'p' && blocked == 0){ //&& tasks[tasks[i].Task_precedIdx].Task_blkRes == 0) {                
                        tasks[i].Task_status = 'r';
                        tasks[i].Task_nact++;
                        tasks[i].Task_blkRes = 0;
                        vTaskResume(tasks[i].Task_handle);
                    }
                    //if(tasks[i].Task_status == 'p') tasks[i].Task_blkRes = 1;
                }
            }
                
            
//            else{
//                if(tasks[i].Task_status == 'p' && tasks[tasks[i].Task_precedIdx].Task_blkRes == 0){
//                    PrintStr("AQUI2\r\n");
//
//                    tasks[i].Task_status = 'r';
//                    tasks[i].Task_nact++;
//                    vTaskResume(tasks[i].Task_handle);
//                }
//            }
        }
        
        vTaskDelay(period);
        tick++;
    }
}