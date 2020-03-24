/*
File: task.c
Ecoh Advance Control System
Creater: Phan Trong Hieu
Date: 07-09-2012
website: www.ecoh.vn
*/
#include "inc/global.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "inc/event.h"
#include "inc/task.h"
#include "inc/gsm.h"
#include "inc/can_gsm.h"
#include "inc/idletask.h"
#include "inc/sysclock.h"

extern eState g_eGSMState;

void vTaskStartScheduler(void){
	// The first task in schedule.
	ui8GSMPoll();
	// Enable systick.
	SysTickEnable();
}

void vTaskScheduler(void){
	eEvent event;
	
	vSysTickTimerExpired();	
	if(ui8GetEvent(&event)==1)	{
  		switch(event)		{
			case EV_IDLE:{
				break;
			}				
			case EV_GSM_ERROR:{
			 	g_eGSMState = STATE_ERROR;
				break;
			} 
			case EV_GSM_SEND_COMPLETE:{
				break;
			}			
			case EV_GSM_RECEIVE_COMPLETE:{
			   	g_eGSMState = STATE_RECEIVE;
				break;
			}
		}
	}
}
