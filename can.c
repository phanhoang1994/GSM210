#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_can.h"

#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/can.h"

#include "inc/global.h"
#include "inc/can_gsm.h"

#include "driverlib/pin_map.h"
#include "driverlib/rom.h"


tCANMsgObject xMsgObject1Status;

unsigned char ucCAN_out_GSM[8]={0x02,'T','1','8','I','0','2','0'};
unsigned char g_ucError;
unsigned char lift_id[7];
extern bool g_bGSMSendError;


volatile unsigned char ucCANBusoff;

void vCAN1Handler(void)
{
	uint8_t i;
 	unsigned long ulStatus, ulCtrl;
	
	ulStatus = CANIntStatus(CAN1_BASE, CAN_INT_STS_CAUSE);
	switch(ulStatus)
	{
		case MSGOBJ0_NUM_OUT_GSM:
		{
	 	 	CANMessageGet(CAN1_BASE, MSGOBJ0_NUM_OUT_GSM, &xMsgObject1Status, 1);
			for(i=0;i<7;i++){lift_id[i]=ucCAN_out_GSM[i+1];}
			if (g_ucError == ucCAN_out_GSM[0]){break;}
			else {g_ucError = ucCAN_out_GSM[0]; g_bGSMSendError = 1;}
			break;
		}
		default:
		{
			ulCtrl = CANStatusGet(CAN1_BASE, CAN_STS_CONTROL);
			if(ulCtrl & CAN_STATUS_BUS_OFF){ ucCANBusoff = 1;}
			break;
		}
	}
	
	CANIntClear(CAN1_BASE, ulStatus);
}

void CANConfigureNetwork(void)
{
	//------------------------------------------------------------
	// CAN0
	//------------------------------------------------------------
	// DispStatus
	// Rx: SOC -> GSM
	xMsgObject1Status.ui32MsgID = MSGOBJ_ID_OUT_GSM;
	xMsgObject1Status.ui32MsgIDMask = 0xFFFFFFFF;
	xMsgObject1Status.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
	xMsgObject1Status.ui32MsgLen = 8;
	xMsgObject1Status.pui8MsgData = ucCAN_out_GSM;
	CANMessageSet(CAN1_BASE, MSGOBJ0_NUM_OUT_GSM, &xMsgObject1Status, MSG_OBJ_TYPE_RX);
}
 
void vCANConfig(void)
{
	//CAN1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_CAN1RX);
	GPIOPinConfigure(GPIO_PA1_CAN1TX); 
	GPIOPinTypeCAN(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN1);
	CANInit(CAN1_BASE);

	CANBitRateSet(CAN1_BASE, SysCtlClockGet(), 100000);
	CANEnable(CAN1_BASE);
	CANIntEnable(CAN1_BASE, CAN_INT_MASTER | CAN_INT_ERROR);

	CANConfigureNetwork();
	ucCANBusoff = 0;
 	IntEnable(INT_CAN1_TM4C123);
}
