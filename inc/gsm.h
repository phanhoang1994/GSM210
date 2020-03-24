/*
File: 
Creater: Phan Van Hoang
Date: 1-1-2019
*/
#ifndef __GSM_H__
#define __GSM_H__

#include "global.h"

#define CTRL_Z 	26 //Ascii character for ctr+z. End of a SMS.
#define CR    	13 //Ascii character for carriage return. 
#define CTRL_M 	13 //Ascii character for carriage return. 
#define LF    	10 //Ascii character for line feed. 
#define  ERROR_NONE 0

typedef enum
{
	AT_CMD_AT,
	AT_CMD_ATE0,
	AT_CMD_CNMI,
	AT_CMD_CPIN,
	AT_CMD_CGCLASS,
	AT_CMD_CMGF,
	AT_CMD_CMGD,
	AT_CMD_CMGS,
	AT_CMD_CMGR,
	AT_CMD_CMTI,
	AT_CMD_ATD,
	AT_CMD_ATH,
	AT_CMD_ATA,
	AT_CMD_VIP,
	AT_CMD_CLVL,
	AT_CMD_CRSL,
	AT_CMD_KECHO,
	AT_CMD_KNOISE,
	AT_CMD_CPBF,
	AT_CMD_KPCMCFG,
	AT_CMD_KVGR,
	AT_CMD_KVGT,
	AT_CMD_VGR,
	AT_CMD_VGT,
	AT_CMD_KMAP,
	AT_CMD_CPBS,
	AT_CMD_CSCS,
	AT_CMD_CIND,
	AT_CMD_COLP,
	AT_CMD_CUSD,
	AT_CMD_ATPSCPOF,
} eATCmd;

typedef struct
{
	const char *phonenumber;
	char *message;
	eATCmd command;
} tGSMMessage;

typedef struct
{
	const char *command;
	void (*function)(void);
} tGSMCommand;


extern void vGSMConfig(void);
extern uint8_t ui8GSMPoll(void);
extern uint8_t ui8GSMPostMsg(tGSMMessage message);
extern uint8_t ui8GSMGetMsg(tGSMMessage *message);

#endif
