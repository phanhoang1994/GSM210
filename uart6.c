/*
File: 
Creater: Phan Van Hoang
Date: 1-1-2019
*/
/* Scheduler includes. */
#include "inc/global.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "inc/uart6.h"
#include "inc/event.h"

uint8_t g_ui8UART6FrameTx[UART6_FRAME_SIZE];

volatile uint32_t g_ui32UART6DelayTx;
volatile uint32_t g_ui32UART6DelayRx;
volatile uint32_t g_ui32UART6DelaySendRequest;
volatile uint8_t g_ui8UART6BufferRx[UART6_FRAME_SIZE];
volatile uint8_t g_ui8UART6CounterRx;

volatile eUartState g_eUART6StateRx;
volatile uint8_t counter_uart6_tx;
volatile uint8_t read_uart6_tx;


void vUART6Config(void){

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
	
	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4);
	GPIOPinConfigure(GPIO_PD4_U6RX);

	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_5);
	GPIOPinConfigure(GPIO_PD5_U6TX);
		
	GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	
	UARTConfigSetExpClk(UART6_BASE, SysCtlClockGet(), 115200,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE));

	// Enable received interrupt and receive time out interrupt
    IntEnable(INT_UART6_TM4C123);
    UARTIntEnable(UART6_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);	// Interrupt receive and  timeout			   
	UARTFIFOLevelSet(UART6_BASE,UART_FIFO_TX2_8,UART_FIFO_RX2_8);
	g_eUART6StateRx = STATE_RX_IDLE;
}

/*-----------------------------------------------------------*/
// Ham xu ly ngat UART6
void vUART6Handler(void){
    uint8_t data, i;
	uint32_t status;

	// Doc trang thai tao ngat va xoa de cho ngat moi.
    status = UARTIntStatus(UART6_BASE, true);
    UARTIntClear(UART6_BASE, status);
		
	if(status == UART_INT_TX){
		i = 0;
		while((i < 8) && (counter_uart6_tx > 0) && (read_uart6_tx >= 16)){
			UARTCharPutNonBlocking(UART6_BASE, g_ui8UART6FrameTx[read_uart6_tx]);
		 	++i;
			--counter_uart6_tx;
			++read_uart6_tx;
		}
		g_ui32UART6DelayTx = UART6_INTERVAL_TX;
	}	

//read uart
	else{
		while(UARTCharsAvail(UART6_BASE)){
			//g_ui32UART6DelaySendRequest = 0;
			g_ui32UART6DelayRx = UART6_INTERVAL_RX;
			data = UARTCharGet(UART6_BASE); //lenh doc
			switch(g_eUART6StateRx){
				case STATE_RX_INIT:{
					break;
				}
				case STATE_RX_ERROR:{
					break;
				}		
				case STATE_RX_IDLE:{
					g_ui8UART6CounterRx = 0;
					g_ui8UART6BufferRx[g_ui8UART6CounterRx++] = data;
					g_ui8UART6BufferRx[g_ui8UART6CounterRx] = '\0';
					g_eUART6StateRx = STATE_RX_RECEIVE;
					break;
				}
				case STATE_RX_RECEIVE:{
					if(g_ui8UART6CounterRx < (UART6_FRAME_SIZE - 1)){
						g_ui8UART6BufferRx[g_ui8UART6CounterRx++] = data;
						g_ui8UART6BufferRx[g_ui8UART6CounterRx] = '\0';
					}
					else{
						g_eUART6StateRx = STATE_RX_ERROR;
					}
					break;
				}
			}
		}
	}
}

/*-----------------------------------------------------------*/
// Ham phat chuoi ky tu
void vUART6PutString(uint8_t *p){
	// Gui ky tu cho den het chuoi, khi do noi dung = 0.
	while(*p){ UARTCharPut(UART6_BASE, *p++);}
}

// Send message in buffer.
void vUART6Send(uint8_t len){
	// Put transmit frame to tx buffer.
	counter_uart6_tx = len;
	read_uart6_tx = 0;
	while((read_uart6_tx < 16) && (counter_uart6_tx > 0)){
		UARTCharPutNonBlocking(UART6_BASE, g_ui8UART6FrameTx[read_uart6_tx]);
		--counter_uart6_tx;
		++read_uart6_tx;
	}
	
	g_ui32UART6DelayTx = UART6_INTERVAL_TX;
	g_ui32UART6DelaySendRequest = UART6_INTERVAL_REQ;
}
