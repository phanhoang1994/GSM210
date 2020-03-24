
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_eeprom.h"
#include "inc/hw_flash.h"
#include "inc/hw_ints.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/eeprom.h"
#include "driverlib/gpio.h"

#include "inc/global.h"
#include "inc/event.h"
#include "inc/sysclock.h"
#include "inc/task.h"
#include "inc/gsm.h" 
#include "inc/gsm210.h" 

void vEEPROMconfig(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	while (EEPROMInit() != EEPROM_INIT_OK);
}

bool write_eeprom(uint32_t *pdata, uint32_t add_epprom, uint32_t count)
{
	if (add_epprom+count >0x07FF)
		return false;
	else
		EEPROMProgram(pdata,add_epprom>>2,count>>2);
		return true;
}

bool read_eeprom(uint32_t *pBuff, uint32_t add_epprom, uint32_t count)
{
		if (add_epprom+count >0x07FF)
		return false;
	else
		EEPROMRead(pBuff,add_epprom>>2,count>>2);
		return true;
}
