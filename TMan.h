/* 
 * File:   TMan.h
 * Author: miguel
 *
 * Created on 20 de Dezembro de 2021, 18:27
 */

#ifndef TMAN_H
#define	TMAN_H

typedef struct _Task Task;

void TMAN_Init(TaskHandle_t scheduler, int tick ); 
void TMAN_Close();

Task* TMAN_Get_Task(const signed char* name);

int TMAN_TaskAdd(const signed char * name);

void TMAN_TaskRegisterAttributes(const signed char * name, int period, int deadline);

void TMAN_TaskWaitPeriod(const signed char * name);

void TMAN_TaskStats(const signed char * name );

void TMAN_Scheduler(void* PvParameters);


#endif	/* TMAN_H */