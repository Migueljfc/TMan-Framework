/*
 * Paulo Pedreiras
 * Miguel Cabral
 * Diogo Vicente, Sept/2021
 *
 * FREERTOS demo for ChipKit MAX32 board
 * - Creates two periodic tasks
 * - One toggles Led LD4, other is a long (interfering)task that 
 *      activates LD5 when executing 
 * - When the interfering task has higher priority interference becomes visible
 *      - LD4 does not blink at the right rate
 *
 * Environment:
 * - MPLAB X IDE v5.45
 * - XC32 V2.50
 * - FreeRTOS V202107.00
 *
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"


/* App includes */
#include "../UART/uart.h"
#include "queue.h"

#include "TMan.h"

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define SCH_PRIO  	(tskIDLE_PRIORITY)
#define A_PRIO  	(tskIDLE_PRIORITY + 1)
#define B_PRIO	    (tskIDLE_PRIORITY + 2)
#define C_PRIO	    (tskIDLE_PRIORITY + 3)

/* Global variables*/
QueueHandle_t xQueue1;


/*
 * Prototypes and tasks
 */

//ainda está em pseudo codigo
#define f(x,y) (1/(1+pow(x,2)))*y
void vTaskFunction(void* pvParameters) {
    
    int i_limit = 100000, j_limit = 5000;
    const signed char* name = (char*) pvParameters;
    
    uint8_t msg[80];
    TickType_t xLastExecutionTime;    
    

    while (1) {
        PrintStr("boi\n\r");
        TMAN_TaskWaitPeriod(name);
//        xLastExecutionTime = xTaskGetTickCount();
        
        sprintf(msg,"%s, %d\n\r", name, xTaskGetTickCount()); //tem de ser com a cena da UART
        PrintStr(msg);
        
        for(int i = 0; i < i_limit; i++) {
//            for (int j = 0; j < j_limit; j++) {
//                f(i,j);
//                //computation to consume time
//            }
        }
        
        sprintf(msg,"%d", (int) xTaskGetTickCount() - xLastExecutionTime);
        PrintStr(msg);
        //other stuff if needed
//        vTaskDelay((const TickType_t) 1000);
    }
}

void vMonitor(void* pvParameters) {
//    PrintStr("lançada");
    char c;
    for (;;) {
        xQueuePeek(xQueue1, &c, 0);
        PutChar(c);
    }
}

/*
 * Create the demo tasks then start the scheduler.
 */
int mainTMan( void )
{
    TaskHandle_t sch = NULL;        //handle for the scheduler task
    TRISAbits.TRISA3 = 0;
    PORTAbits.RA3 = 0;
    
    // Init UART and redirect stdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

    __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
    
    /* Welcome message*/
    printf("\n\n******************\n\r");
    printf("Starting TMAN Demo\n\r");
    printf("******************\n\r");
    
    /* Create the tasks defined within this file. */
    
    xQueue1 = xQueueCreate(50,sizeof(char));    
    
  
//    xTaskCreate( vMonitor, ( const signed char * const ) "Monitor", configMINIMAL_STACK_SIZE, NULL, A_PRIO, NULL );
//    xQueueSend(xQueue1, "ola" , (TickType_t) 0);

    
    //Create scheduler task and start TMan
    xTaskCreate( TMAN_Scheduler, ( const signed char * const ) "Scheduler", configMINIMAL_STACK_SIZE, NULL, A_PRIO-1, &sch);
    TMAN_Init(sch, 1000);
   
    xTaskCreate( vTaskFunction, ( const signed char * const ) "A", configMINIMAL_STACK_SIZE, (void*) "A" , A_PRIO, NULL);
    TMAN_TaskAdd("A");
    

  
//    xTaskCreate( vTaskFunction, ( const signed char * const ) "B", configMINIMAL_STACK_SIZE, (void*) "B", B_PRIO, NULL );
//    TMAN_TaskAdd("B");
    
    /* Finally start the scheduler. */
	vTaskStartScheduler();
    

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}



