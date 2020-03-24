/*
File: 
Creater: Phan Van Hoang
Date: 1-1-2019
*/
#include "inc/global.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "inc/event.h"
#include "inc/sysclock.h"
#include "inc/task.h"
#include "inc/gsm.h" 
#include "inc/gsm210.h"

extern volatile eUartState g_eUART6StateRx;
extern volatile uint32_t g_ui32UART6DelayRx;
extern volatile uint32_t g_ui32UART6DelayTx;
extern volatile uint32_t g_ui32UART6DelaySendRequest;
extern volatile uint32_t g_ui32CANDelaySendRequest;
extern unsigned long ulTimerRestart;
extern unsigned long ulTimerUpdate;
extern unsigned long ulTimeButton;
extern unsigned long ulTimerSendLcd;
extern unsigned long	ulTimerCheckAccount;

volatile uint16_t count_10ms;
volatile uint16_t count_100ms;
volatile uint16_t count_1s;
volatile uint8_t toggle;
volatile uint16_t count_100ms;
volatile uint16_t ulTimerButton;

//thuc hien cuoc goi: sau 10s --> neu khong bat may cho phep nhan nut ngat cuoc goi
extern bool connect_voice;

//button
extern uint8_t g_ui8CountButton;
bool g_bButton;
extern bool g_bReadPhonebook;
//input
unsigned long ulInport;
unsigned long ulSample[SAMPLE];
unsigned char ucSampleIndex;
//reset watch dog
extern bool g_bGSM_Error;

//1ms goi ham vSysTickHandler 1 lan
void vSysTickHandler(void){
	if(g_ui32UART6DelayRx > 1){ --g_ui32UART6DelayRx;} 	//UART6 - port rs485 with WISO218 GSM Module
	if(g_ui32UART6DelayTx > 1){ --g_ui32UART6DelayTx;}
	if(g_ui32UART6DelaySendRequest > 1){ --g_ui32UART6DelaySendRequest;}
	
	if(count_10ms > 0){ --count_10ms;}
	if(count_100ms > 0){ --count_100ms;}
	if(ulTimerRestart>0){--ulTimerRestart;}//khoi dong lai sau khi config khng thanh cong
	if(ulTimerButton>0){--ulTimerButton;}
	if (ulTimerUpdate>0){--ulTimerUpdate;}
	if (ulTimerSendLcd>0){--ulTimerSendLcd;}
	if (ulTimerCheckAccount>0){--ulTimerCheckAccount;}
	
	

	if(++count_1s >= 500){ count_1s = 0; if(toggle == 1){ _ledrun_on(); toggle = 0;} else { _ledrun_off(); toggle = 1;};}
}

void vSysTickConfig(void){
	SysTickPeriodSet((CPU_CLOCK_HZ / TICK_RATE_HZ)-1UL);
	SysTickIntEnable();
}

void vSysTickTimerExpired(void){
	ui8GSMPoll();
	if(count_10ms == 0){count_10ms = 10;if(g_ui8GSMCommandDelay > 0){ --g_ui8GSMCommandDelay;}}	
	//Post Event
	if(g_ui32UART6DelayTx == 1){ ui8PostEvent(EV_GSM_SEND_COMPLETE); g_ui32UART6DelayTx = 0;}	// GSM
	if(g_ui32UART6DelayRx == 1){if(g_eUART6StateRx==STATE_RX_RECEIVE){ ui8PostEvent(EV_GSM_RECEIVE_COMPLETE);}; g_eUART6StateRx=STATE_RX_IDLE; g_ui32UART6DelayRx = 0;}
	if(g_ui32UART6DelaySendRequest == 1){ ui8PostEvent(EV_GSM_ERROR); g_ui32UART6DelaySendRequest = 0;}

	if(count_100ms == 0){
	 	count_100ms = 100;
		if(g_ui16GSMRestartWaitting > 0){ --g_ui16GSMRestartWaitting;}
	}	
	vInput();
	if(_isset(ulInport,1)&&(g_bReadPhonebook==0)){ulTimerButton=100;}
	if((_isclear(ulInport,1))&&(ulTimerButton==1)){g_bButton = 1;ulTimerButton=0;}
	if((g_bButton==1)&&(g_bReadPhonebook==0)){g_ui8CountButton += 1; g_bButton = 0;}
	if(ulTimerRestart==1){g_bGSM_Error=1;}
}

void vInput(void)
{
	unsigned long result, all_or=0, all_and=0;
	unsigned char i;
	
	//INPUT
	ulSample[ucSampleIndex] = 0x00000000<< 8;
	//BUTTON
	ulSample[ucSampleIndex] |= (unsigned long)GPIOPinRead(GPIO_PORTG_BASE,GPIO_PIN_1);
	ulSample[ucSampleIndex] &= 0x00000002;
	
	all_or = ulSample[0];
	all_and = all_or;
	for(i=1;i<SAMPLE;i++)
	{
		all_or |= ulSample[i];
		all_and &= ulSample[i];
	}

	result = ~ulInport;
	result &= 0x00000002;
	
	result &= all_or;
	result |= all_and;
	
	ulInport = ~result;
	ulInport &= 0x00000002;
	
	if(++ucSampleIndex>=SAMPLE) ucSampleIndex=0;
}


