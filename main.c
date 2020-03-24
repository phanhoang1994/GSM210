/*
File: 
Creater: Phan Van Hoang
Date: 1-1-2019
*/

// define de su dung chuong trinh trong ROM
#define TARGET_IS_TM4C123_RA1
#include "inc/global.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/watchdog.h"
#include "driverlib/flash.h"

#include "inc/sysclock.h"
#include "inc/gsm.h"
#include "inc/task.h"
#include "inc/idletask.h"
#include "inc/can_gsm.h"
#include "inc/uart6.h"
#include "inc/uart4.h"


volatile uint8_t wdt;
extern bool g_bGSM_Error;
void vHardwareConfig(void);
void vWDTConfig(void);
extern void vCANConfig(void);

const uint32_t MASK[32] = {
	0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000
};

//-------------------------------------
void vWDTHandler(void){ 
	if(wdt == 1){ wdt = 0; WatchdogIntClear(WATCHDOG0_BASE);}
}

int main(void){
	vHardwareConfig();
	vTaskStartScheduler();
	
	while(1){
		vTaskScheduler();
		if(g_bGSM_Error==0){wdt = 1;}
	}
}

void vHardwareConfig(void){
	/* Set the clocking to run from the PLL at 50 MHz */
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

	vMainConfig();
	vUART6Config();
	vUART4Config();
	vGSMConfig();
	vSysTickConfig();
	vWDTConfig();
	IntMasterEnable();
} 

void vWDTConfig(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	IntEnable(INT_WATCHDOG);
	WatchdogReloadSet(WATCHDOG0_BASE, SysCtlClockGet());
	WatchdogResetEnable(WATCHDOG0_BASE);
	WatchdogEnable(WATCHDOG0_BASE);
}

void vMainConfig(void){ 
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_AFSEL) &= ~0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_DEN) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_AFSEL) &= ~0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_DEN) |= 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;


	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);	// Led run
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_3);	// Led call
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_2);	// Led smse
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1);	// Led re_call
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);	// Led 
	
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);	// Led sign1
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0);	// Led sign2
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);	// Led sign3
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_2);	// Led sign4
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_3);	// Led sign5
	
	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_5);	// En_audio
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1); // Pwrkey
	
	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);	// X1O
	
	GPIOPinTypeGPIOInput(GPIO_PORTG_BASE, GPIO_PIN_1);	// X1I
	GPIOPadConfigSet(GPIO_PORTG_BASE,GPIO_PIN_1,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}




