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

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"


/* App includes */
#include "../UART/uart.h"
#include "semphr.h" 
#include "queue.h"

//#include "TMan.c"

/* Set the tasks' period (in system ticks) */
#define LED_FLASH_PERIOD_MS 	( 250 / portTICK_RATE_MS ) 
#define INTERF_PERIOD_MS 	( 3000 / portTICK_RATE_MS )
#define ACQ_PERIOD_MS     (100 / portTICK_RATE_MS)

/* Control the load task execution time (# of iterations)*/
/* Each unit corresponds to approx 50 ms*/
#define INTERF_WORKLOAD          ( 20)

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define A_PRIO  	(tskIDLE_PRIORITY + 1)
#define B_PRIO	    (tskIDLE_PRIORITY + 2)
#define C_PRIO	    (tskIDLE_PRIORITY + 3)

/* Global variables*/
QueueHandle_t xQueue1, xQueue2;



/*
 * Prototypes and tasks
 */
void vDataAqc(void *pvParam)
{
    
   
     // Variable declarations;
    //struct Temperature *res;
    float temp;
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = portTICK_PERIOD_MS*(ACQ_PERIOD_MS);
   
    if(xQueue1 != 0){
    // Main loop
        while (1) {
            vTaskDelayUntil(&xLastWakeTime,xFrequency);
            // Get one sample
            IFS1bits.AD1IF = 0; // Reset interrupt flag
            AD1CON1bits.ASAM = 1; // Start conversion
            while (IFS1bits.AD1IF == 0); // Wait fo EOC

            // Convert to 0..3.3V 
            temp = (ADC1BUF0 * 3.3) / 1023;
            temp = (temp * 100) / 3.3;
           
            // Output result
            //sprintf(msg,"Voltage: %f\n\r",res);
            //PrintStr(msg); 
            //printf("Temp:%f",(res-2.7315)/.01); // For a LM335 directly connected
            xQueueSend(xQueue1, (void*) &temp, ( TickType_t ) 0 );

        }
    }

}
 

void vDataProc(void *pvParam)
{
    double sum = 0.0;
    double a[5] = {0,0,0,0,0};
    //struct Temperature *res;
    //struct Average *avg;
    float res;
    int avg;
    if( xQueue1 != 0 && xQueue2 != 0 ){
        while(1){
           
            if(xQueue1 != 0){
                
                if( xQueuePeek( xQueue1, &( res ), ( TickType_t ) 10 ) ){
                 
                    a[1] = a[0];
                    a[2] = a[1];
                    a[3] = a[2];
                    a[4] = a[3];
                    a[0] = res;

                    for(int i = 0; i < 5; i++){
                        sum += a[i]; 
                    }
                    avg =(int) sum/5;
                    sum = 0;
                    /* We have finished accessing the shared resource.  Reset the
                    Queue. */
                    xQueueReset(xQueue1);
                       
                    xQueueSend(xQueue2, (void*) &avg, ( TickType_t ) 0 );
                    
                }
                else{
                   //Failed to peek the message after 10 ticks
                }        
            }
            
        }
          
    } 
}

void vDataConvert(void *pvParam)
{
    uint8_t msg [80];
    //struct Average *avg;
    int avg;
    if( xQueue2 != 0 )
    {
        while(1){
          
           if( xQueuePeek( xQueue2, &( avg ), ( TickType_t ) 10 ) ){
            
                
                sprintf(msg,"AERAGE OF LAST 5 TEMPERATURE SAMPLES: %d\n\r",avg);
                PrintStr(msg);  
                xQueueReset(xQueue2);            
          
            }
            else
            {
                //Failed to peek the message after 10 ticks
            }
        }
    }
 
}

//ainda está em pseudo codigo
void vTaskFunction(void* pvParameters) {
    
    //comentei para nao dar erro
//    for (;;) {
//        TMAN_TaskWaitPeriod();
//        task.xLastExecutionTime = xTaskGetTickCount();
//        
//        printf("Nome, Numero"); //tem de ser com a cena da UART
//        
//        for(int i = 0, i < ; i++) {
//            for (int j = 0; j < ; j++) {
//                //computation to consume time
//            }
//        }
//        
//        //other stuff if needed
//    }
}

/*
 * Create the demo tasks then start the scheduler.
 */
int mainTMan( void )
{
    
    // Set RA3 (LD4) and RC1 (LD5) as outputs
    TRISAbits.TRISA3 = 0;
    TRISCbits.TRISC1 = 0;
    PORTAbits.RA3 = 0;
    PORTCbits.RC1 = 0;
    
    
	// Init UART and redirect stdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

    __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
       
    // Disable JTAG interface as it uses a few ADC ports
    DDPCONbits.JTAGEN = 0;
    
    // Initialize ADC module
    // Polling mode, AN0 as input
    // Generic part
    AD1CON1bits.SSRC = 7; // Internal counter ends sampling and starts conversion
    AD1CON1bits.CLRASAM = 1; //Stop conversion when 1st A/D converter interrupt is generated and clears ASAM bit automatically
    AD1CON1bits.FORM = 0; // Integer 16 bit output format
    AD1CON2bits.VCFG = 0; // VR+=AVdd; VR-=AVss
    AD1CON2bits.SMPI = 0; // Number (+1) of consecutive conversions, stored in ADC1BUF0...ADCBUF{SMPI}
    AD1CON3bits.ADRC = 1; // ADC uses internal RC clock
    AD1CON3bits.SAMC = 16; // Sample time is 16TAD ( TAD = 100ns)
    // Set AN0 as input
    AD1CHSbits.CH0SA = 0; // Select AN0 as input for A/D converter
    TRISBbits.TRISB0 = 1; // Set AN0 to input mode
    AD1PCFGbits.PCFG0 = 0; // Set AN0 to analog mode
    // Enable module
    AD1CON1bits.ON = 1; // Enable A/D module (This must be the ***last instruction of configuration phase***)
   
    
    /* Welcome message*/
    printf("\n\n *********************************************\n\r");
    printf("Starting TEMPERATURE ACQ FreeRTOS Demo - A4 APP \n\r");
    printf("*********************************************\n\r");
    
    
    
    
    
    
    
    
    
    
    
    // Queues
    
    // xQueue1 = xQueueCreate(1,sizeof(float));
    // xQueue2 = xQueueCreate(1,sizeof(int));
      
    /* Create the tasks defined within this file. */
	
    TMAN_Init(5);
    
    xTaskCreate( vDataAqc, ( const signed char * const ) "Task A", configMINIMAL_STACK_SIZE, NULL, A_PRIO, NULL );
    TMAN_TaskAdd("Task A");
    
    
    xTaskCreate( vDataProc, ( const signed char * const ) "Task B", configMINIMAL_STACK_SIZE, NULL, B_PRIO, NULL );
    xTaskCreate( vDataConvert, ( const signed char * const ) "Task C", configMINIMAL_STACK_SIZE, NULL, C_PRIO, NULL );

    
    
    
    /* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}



