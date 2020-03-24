/* Scheduler includes. */
//#define PART_TM4C123AE6PM

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_uart.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"

#include "inc/uart4.h"
#include "inc/global.h"


unsigned char ucUART4FrameTx[UART4_FRAME_SIZE];

volatile unsigned char ucUART4DelayTx;
volatile unsigned char ucUART4DelayRx;
volatile unsigned char ucUART4BufferRx[UART4_FRAME_SIZE];
volatile unsigned char ucUART4CounterRx;

volatile eUartStateHandler eUART4StateRx;
static volatile unsigned char ucUART4CounterTx;
static volatile unsigned char ucUART4RdIndexTx;

void vUART4Config(void)
{
	// Enable the peripherals used by the application.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0);
 
 	// Thiet lap thong so cho Port UART4 - giao tiep voi inverter
	// Toc do baud 11520, 8 bit, 1 stop, no parity
  	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); 

	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4);
	GPIOPinConfigure(GPIO_PC4_U4RX);

	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_5);
	GPIOPinConfigure(GPIO_PC5_U4TX);
	
 	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);	
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	HWREG(GPIO_PORTC_BASE + GPIO_O_AMSEL) &= ~0x30;
	
	UARTConfigSetExpClk(UART4_BASE, SysCtlClockGet(), 9600,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_EVEN));
                           	
	#if(DEBUG_VIEW==1)
		#warning "UART4 baurate is 19200, 8bit, 1stop, even parity"
	#endif

	// Enable received interrupt and receive time out interrupt
    IntEnable(INT_UART4_TM4C123);
    UARTIntEnable(UART4_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);	// Interrupt receive and  timeout			   
	UARTFIFOLevelSet(UART4_BASE,UART_FIFO_TX2_8,UART_FIFO_RX2_8);
	//DIR_OFF();
	eUART4StateRx=STATE_RX4_IDLE;
}

//*-----------------------------------------------------------*/
// Ham xu ly ngat UART4
void vUART4Handler(void)
{
    unsigned char ucData,i;
	unsigned long ulStatus;

	// Doc trang thai tao ngat va xoa de cho ngat moi.
    ulStatus = UARTIntStatus(UART4_BASE, true);
    UARTIntClear(UART4_BASE, ulStatus);
		
	if(ulStatus==UART_INT_TX)
	{
		i=0;
		while((i<8)&(ucUART4CounterTx>0)&(ucUART4RdIndexTx>=16))
		{
			UARTCharPutNonBlocking(UART4_BASE, ucUART4FrameTx[ucUART4RdIndexTx]);
		 	++i;
			--ucUART4CounterTx;
			++ucUART4RdIndexTx;
		}
		ucUART4DelayTx = TR_TX4 ;
	}	
	else
	{
		while(UARTCharsAvail(UART4_BASE))
		{
			ucUART4DelayRx = TR_RX4;
			ucData = UARTCharGet(UART4_BASE);
			switch(eUART4StateRx)
			{
				case STATE_RX4_INIT:
				{

					break;
				}
				case STATE_RX4_ERROR:
				{

					break;
				}		
				case STATE_RX4_IDLE:
				{
					ucUART4CounterRx = 0;
					ucUART4BufferRx[ucUART4CounterRx++] = ucData;
					eUART4StateRx = STATE_RX4_RECEIVE;
					break;
				}
				case STATE_RX4_RECEIVE:
				{
					if(ucUART4CounterRx<UART4_FRAME_SIZE)
					{
						ucUART4BufferRx[ucUART4CounterRx++] = ucData;
					}
					else
					{
						eUART4StateRx = STATE_RX4_ERROR;
					}
					break;
				}
			}
		}	
	}
}

/*-----------------------------------------------------------*/
// Ham phat chuoi ky tu
void vUART4PutString(unsigned char *pucStr)
{
	// Gui ky tu cho den het chuoi, khi do noi dung = 0.
	while(*pucStr){UARTCharPut(UART4_BASE, *pucStr++);};
}


// Send message in buffer.
void vUART4Send(unsigned char ucLen)
{
	// Put transmit frame to tx buffer.
	ucUART4CounterTx=ucLen;
	ucUART4RdIndexTx=0;
	DIR_ON();
	while((ucUART4RdIndexTx<16)&(ucUART4CounterTx>0))
	{
		UARTCharPutNonBlocking(UART4_BASE, ucUART4FrameTx[ucUART4RdIndexTx]);
		--ucUART4CounterTx;
		++ucUART4RdIndexTx;
	}

	// Transmit again.	
	ucUART4DelayTx = TR_TX4 ; // >=11 character ,baudrate=2500 => 36ms
}

