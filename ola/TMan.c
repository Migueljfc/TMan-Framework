
/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <xc.h>

/* Kernel includes */
#include "FreeRTOS.h"
#include "task.h"

/* App includes */
#include "../UART/uart.h"



struct _TMan {
    struct _Task* tasks = (struct _Task*) malloc(sizeof(struct _Task)*16);      // support for 16 tasks
    int index = 0;
    int tickrate;
};

struct _Task {
    char* Task_name;
    int Task_id;
    int Task_period;
    int Task_deadline;
    int Task_phase;
    
    int Task_nact;
    int Task_nmiss;
    
    char Task_status;
    
    TaskHandle_t Task_handle;
};


_TMan* TMAN_Init(int tick) {
    
    //GetTickCount();
    struct _TMan* TMan = (struct _TMan*) malloc(sizeof(_TMan));
    
    *TMan->tickrate = tick;
    
}

int TMAN_Close() {
    
}

int TMAN_TaskAdd(const signed char * name) {
    
    struct _Task task;
    strcpy(task.Task_name, name);
    
    task.Task_handle = xTaskGetHandle(name);
    
    
}

int TMAN_AddRegisterAttributes(const signed char * name, int period, int deadline, ) {
    
}

int TMAN_TaskWaitPeriod() {
    
}

void TMAN_TaskStats(struct _Task) {
    
}
