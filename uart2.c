/*
* Filename.......: UART2.c
*
* Created by.....: Phan Trong Hieu<tronghieu@ecoh.vn>
* Description....: Functions is used to control UART2 of LM3S2965.
* Licence........: This file is property of ecoh.vn
* Creation Date..: 06-12-2009
* Last change....: 11-04-2013
* This File is changed to used for SOC210 project of ThienNam elevator JSC. 
*/

#include <stdio.h>
#include <stdbool.h>
#include <cstdint>
/* Scheduler includes. */
#include "inc/global.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "inc/uart2.h"
#include "inc/event.h"

char call_run[] = {'C','a','l','l','i','n','g',' ',' ',' ','\0'};

uint8_t g_ui8UART2FrameTx[UART2_FRAME_SIZE];
volatile uint8_t g_ui8UART2BufferRx[UART2_FRAME_SIZE];
volatile uint8_t g_ui8UART2CounterRx;
volatile eUartState g_eUART2StateRx;
uint32_t g_ui32UART2IntervalRx;
volatile uint32_t g_ui32UART2DelayTx;
volatile uint32_t g_ui32UART2DelayRx;
volatile uint32_t g_ui32UART2DelaySendRequest;

volatile uint8_t counter_uart2_tx;
volatile uint8_t read_uart2_tx;

void vUART2Config(void){
	g_ui32UART2IntervalRx = UART2_INTERVAL_RX;
	
	// Thiet lap thong so cho Port UART2 - giao tiep voi inverter
	// Toc do baud 11520, 8 bit, 1 stop, no parity
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1);
    	
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); 
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6);
	GPIOPinConfigure(GPIO_PD6_U2RX);

	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_7);
	GPIOPinConfigure(GPIO_PD7_U2TX);
			
	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

	
	UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), 9600,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));

	// Enable received interrupt and receive time out interrupt
  IntEnable(INT_UART2_TM4C123);
  UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);	// Interrupt receive and  timeout	   
	UARTFIFOLevelSet(UART2_BASE,UART_FIFO_TX2_8,UART_FIFO_RX2_8);
	g_eUART2StateRx = STATE_RX_IDLE;
}

//*-----------------------------------------------------------*/
// Ham xu ly ngat UART2
void vUART2Handler(void){
	uint8_t data, i;
	uint32_t status;

	// Doc trang thai tao ngat va xoa de cho ngat moi.
	status = UARTIntStatus(UART2_BASE, true);
	UARTIntClear(UART2_BASE, status);
		
	if(status == UART_INT_TX){
		i=0;
		while((i < 8) && (counter_uart2_tx > 0) && (read_uart2_tx >= 16))
		{
			UARTCharPutNonBlocking(UART2_BASE, g_ui8UART2FrameTx[read_uart2_tx]);
		 	++i;
			--counter_uart2_tx;
			++read_uart2_tx;
		}
	}	
	else{
		while(UARTCharsAvail(UART2_BASE)){
			g_ui32UART2DelayRx = g_ui32UART2IntervalRx;
			data = UARTCharGet(UART2_BASE);
			switch(g_eUART2StateRx){
				case STATE_RX_INIT:{
					break;
				}
				case STATE_RX_ERROR:{
					break;
				}		
				case STATE_RX_IDLE:{
					g_ui8UART2CounterRx = 0;
					g_ui8UART2BufferRx[g_ui8UART2CounterRx++] = data;
					g_eUART2StateRx = STATE_RX_RECEIVE;
					break;
				}
				case STATE_RX_RECEIVE:{
					if(g_ui8UART2CounterRx < UART2_FRAME_SIZE){
						g_ui8UART2BufferRx[g_ui8UART2CounterRx++] = data;
					}
					else{
						g_eUART2StateRx = STATE_RX_ERROR;
					}
					break;
				}
			}
		}	
	}
}

/*-----------------------------------------------------------*/
// Ham phat chuoi ky tu
void vUART2PutString(uint8_t *p){
	// Gui ky tu cho den het chuoi, khi do noi dung = 0.
	while(*p){ UARTCharPut(UART2_BASE, *p++);}
}


// Send message in buffer.
void vUART2Send(uint8_t len){
	// Put transmit frame to tx buffer.
	counter_uart2_tx = len;
	read_uart2_tx = 0;
	while((read_uart2_tx < 16) && (counter_uart2_tx > 0)){
		UARTCharPutNonBlocking(UART2_BASE, g_ui8UART2FrameTx[read_uart2_tx]);
		--counter_uart2_tx;
		++read_uart2_tx;
	}

	g_ui32UART2DelayTx = UART2_INTERVAL_TX; // >=11 character ,baudrate=2500 => 36ms
}

