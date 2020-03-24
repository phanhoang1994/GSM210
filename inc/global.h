#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifndef PART_TM4C123AE6PM
	#define PART_TM4C123AE6PM
#endif

#include <stdio.h>
#include <stdbool.h>
#include <cstdint>


#include "config.h"
#include "gsm210.h"

#define CPU_CLOCK_HZ							50000000
#define TICK_RATE_HZ							1000

// Comm with AT COMMAND GSM
#define UART6_INTERVAL_TX					25
#define UART6_INTERVAL_RX					25
#define UART6_INTERVAL_REQ				10000	//10s



#define TR_TX4										10
#define TR_RX4										25
#define TR_UPDATE									50000

#define CAN_INTERVAL_REQ					50		//50ms

#define FLASH_LOCK								0x0001F000
#define SAMPLE 										4

#define ENABLE_CALL								1
#define DISABLE_CALL 						0

typedef enum {
	STATE_RX_INIT,
	STATE_RX_ERROR, 
	STATE_RX_IDLE,
	STATE_RX_RECEIVE
} eUartState;

typedef enum {
	STATE_IDLE,
	STATE_ERROR,
	STATE_EXCUTE,
	STATE_REQUEST,
	STATE_RECEIVE,
} eState;

typedef union {
	uint32_t LONG;
	uint16_t INT[2];
	uint8_t BYTE[4];
} DWORD;

typedef union
{
	uint16_t INT;
	uint8_t BYTE[2];
} WORD;

// gsm
extern bool g_bGSMSendError;
extern uint8_t g_ui8GSMCommandDelay;
extern volatile uint16_t g_ui16GSMRestartWaitting;
extern volatile uint16_t g_ui16TaskSequentiallyWaiting;


// main.c

extern uint8_t g_ui8MainData[];
extern const uint32_t MASK[];

#define _clear(x,y) (x &= ~MASK[y]) 
#define _set(x,y) (x |= MASK[y])
#define _toggle(x,y)(x ^= MASK[y])
#define _isset(x,y) ((x & MASK[y]) != 0)
#define _isclear(x,y) ((x & MASK[y]) == 0) 
#define _low(x)			(uint8_t)(x)
#define _high(x)		(uint8_t)(x >> 8)
#define _value_int(x)	_low(x), _high(x)
#endif

