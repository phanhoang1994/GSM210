#ifndef __TASK_H__
#define __TASK_H__
 
#define TASK_PROCESS_UNFINISHED 0
#define TASK_PROCESS_FINISHED  	1

#define NUM_OF_TASK				3

#define TASK_IDLE				0
#define TASK_GSM				1
#define TASK_CAN				2
#define TASK_J6					3


extern void vTaskStartScheduler(void);
extern void vTaskScheduler(void);
extern uint8_t ui8TaskIncrementTick(void);

#endif
