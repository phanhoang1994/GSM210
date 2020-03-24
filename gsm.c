/*
File: gsm.c
Creater: Phan Van Hoang
Date: 1-1-2019
*/
#include <stdio.h>
#include <string.h>
#include "inc/global.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "inc/gsm.h"
#include "inc/uart6.h"
#include "inc/task.h"
#include "inc/can_gsm.h"
#include "inc/gsm210.h"
#include "inc/sysclock.h"
#include "inc/uart4.h"


#define GSM_MSG_QUEUE_SIZE 100
//----------------------------------------------------------------------
	const char g_scGSMCenterPhoneNumber[]="01299221234"; //so tong dai thien nam
	//char g_scGSMCenterPhoneNumber[]="0327459285"; // Mr. vhoang
	
	char numberphone1[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
	char numberphone2[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
	char numberphone3[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
	char numberphone4[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
	char numberphone5[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
//---------------------------------------------------------------------
	char phonebook_index[] = {'0','0','0','0','0','0','0','0','0','0','\0'};
	const char *indexSim; 
	char index1_sim[]={'1','\0'};
	char index2_sim[]={'2','\0'};
	char index3_sim[]={'3','\0'};
	char index4_sim[]={'4','\0'};
	char index5_sim[]={'5','\0'};
	char select_index_sim[1];
	char index_sim[5] = {'1','2','3','4','5'};
//-----------------------------------------------------------------------
	char syntaxCheckAccount[5] = {'0','0','0','0','0'};
	unsigned char g_ucDataCheckAcccout[160];
//-----------------------------------------------------------------------
//global
eState g_eGSMState;
unsigned char g_ucError;
unsigned char lift_id[7];
bool g_bGSM_Error;
unsigned long ulTimerRestart;
unsigned char g_ucSignQuality;
extern unsigned char ucUART4FrameTx[UART4_FRAME_SIZE];
unsigned long ulTimerUpdate;
unsigned long ulTimerSendLcd;
unsigned long	ulTimerCheckAccount;
//sms
bool g_bGSMSendError;
//call
bool g_bGSMCall;
uint8_t g_ui8SendCmdCall;
//receive call
bool g_bGSMRevCall;
//reply from modul when call
bool connect_voice = 0;
bool busy = 0;
bool no_carrier = 0;
bool ring = 0;
//read phonebook
bool g_bReadPhonebook;
char g_cIndexReadPhone;
bool g_bReadPhonebook;
bool g_bSendDataLcd;
bool g_bReadSignal;
uint8_t g_bSendCmdCheckAccount;

volatile uint16_t g_ui16GSMRestartWaitting;
uint8_t g_ui8GSMCommandDelay;

uint8_t g_ui8CountButton; 		
uint8_t indexCheckAc;
//ma thang- loi
char gsm_response_liftID[23] = {'E', 'R', 'R', 'O', 'R', '0', '0','-','L','I','F','T',' ','I','D',':','x','x','x','x','x','x','x'};

tGSMMessage gsm_queue[GSM_MSG_QUEUE_SIZE];
tGSMMessage gsm_handle;
uint8_t gsm_queue_counter;
uint8_t gsm_write_index; 
uint8_t gsm_read_index;
uint8_t gsm_init_state;

eState (*gsm_send)(eATCmd);
eState (*gsm_receive)(void);

uint8_t ui8GSMSendATCommand(char *p);
uint8_t ui8GSMSendATCommand2(char *p);

eState eGSMSendMsg(eATCmd command);
eState eGSMSendCmd(eATCmd command);

eState eGSMWaitSendMsg(void);
eState eGSMWaitSendMsg1(void);
eState eGSMWaitOK(void);

void vReadPhonebook(void);
void ui8GSMSendATCmdReadNoPhone (char *index);
void eGSMWaitStatusReadNoPhone(const char * pString, char *numberphone);

uint8_t ui8GSMSendATCmdCall (char *numberphone,char index);
uint8_t ui8GSMSendATCmdDisCall (void);
void eGSMWaitStatusCall(void);
void vCheckAccount(void);

void vATCmdConfig(void);
void vGSMUpdate(void);
void vReadSignal (void);


//-------------------------------------------------------

uint8_t ui8GSMPostMsg(tGSMMessage msg){
	gsm_queue[gsm_write_index].phonenumber = msg.phonenumber;
	gsm_queue[gsm_write_index].message = msg.message;
	gsm_queue[gsm_write_index].command = msg.command;
	if(++gsm_queue_counter >= GSM_MSG_QUEUE_SIZE){ gsm_queue_counter = 0;}
 	if(++gsm_write_index >= GSM_MSG_QUEUE_SIZE){ gsm_write_index = 0;}
 return 1;
}

uint8_t ui8GSMGetMsg(tGSMMessage *p){
 if(gsm_queue_counter != 0){
		p->phonenumber = gsm_queue[gsm_read_index].phonenumber;
		p->message = gsm_queue[gsm_read_index].message;
		p->command = gsm_queue[gsm_read_index].command;
		if(++gsm_read_index >= GSM_MSG_QUEUE_SIZE){ gsm_read_index = 0;}
		--gsm_queue_counter;
		return 1;
	}
 return 0;
}
//---------------------------
//CONFIGURE INITIAL PARAMETER
void vGSMConfig(void){
	vATCmdConfig();
	_gsm_pwrkey_on();
	g_eGSMState = STATE_EXCUTE;
	gsm_send = 0;
	gsm_receive = 0;
	gsm_init_state = 0;
	g_ui16GSMRestartWaitting = 15; 	// 1.5 sec
	g_bGSMSendError = 0;
#if(FUNCTION_CALL==1)
	//read the phone number
	g_bReadPhonebook = 1;
	g_cIndexReadPhone = 1;
	//lcd
	g_ucLcdInstall = START_SETUP;
	g_bSendDataLcd = 1;
	
#endif
}
//------------------------------------
//AT COMMAND TO CONFIGURE MODUL HL6528
void vATCmdConfig(void){
	tGSMMessage msg;
		//Configure Common
			msg.phonenumber = 0;
			msg.message = 0;

			msg.command = AT_CMD_AT;	//INIT
			ui8GSMPostMsg(msg);

			msg.command = AT_CMD_ATE0;  //DISABLE ECHO
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_CPIN;//CHECK SIM READY
			ui8GSMPostMsg(msg);

			msg.command = AT_CMD_CNMI; //FORMAT TO RECEIVE NEW SMS
			ui8GSMPostMsg(msg);

			msg.command = AT_CMD_CMGF; //FORMAT SMS
			ui8GSMPostMsg(msg);

			msg.command = AT_CMD_CMGD; //DELETE SMS IN SIM
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_CPBS;	//Select Phonebook Memory Storage
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_CSCS;	//Format CSCS = GSM
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_COLP;	
			ui8GSMPostMsg(msg);
			
	//Configure parameter for mic and speaker C
	#if(FUNCTION_CALL==1)
			msg.command = AT_CMD_VIP;//Select mode mobie
			ui8GSMPostMsg(msg);

			msg.command = AT_CMD_CLVL; //LEVEL VOLUME MIC 
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_CRSL; //LEVEL VOLUME RING
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_KECHO; // ECHO CANCELLLATION
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_KNOISE; // NOISE CANCELLLATION
			ui8GSMPostMsg(msg);
			
			/*msg.command = AT_CMD_KVGR; // Receive Gain Selection
			ui8GSMPostMsg(msg);

			msg.command = AT_CMD_KVGT; // Transmit Gain Selection
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_VGR; // Receive Gain Selection
			ui8GSMPostMsg(msg);
				
			msg.command = AT_CMD_KMAP; // Microphone Analog Parameters
			ui8GSMPostMsg(msg);
			
			msg.command = AT_CMD_VGT; // Transmit Gain Selection 
			ui8GSMPostMsg(msg);*/
	#endif
}
//------------
//MAIN PROGRAM
uint8_t ui8GSMPoll(void){
	uint8_t status = TASK_PROCESS_UNFINISHED;

	if(gsm_init_state != 4){
		switch(gsm_init_state){
			case 0:	{
				if(g_ui16GSMRestartWaitting == 0){ gsm_init_state = 1;}
				break;
			}
			case 1:{
				_gsm_pwrkey_off();
				g_ui16GSMRestartWaitting = 30; //3 sec
				gsm_init_state = 2;
				break;
			}
			case 2:{
				if(g_ui16GSMRestartWaitting == 0){ gsm_init_state = 3;}
				break;
			}
			case 3:{
				gsm_init_state = 4;
				_gsm_pwrkey_on();
				g_ui16GSMRestartWaitting = 50; // Wait SIM ready 5s
				break;
			}
			default:{ break;}
		}
		return TASK_PROCESS_FINISHED;
	}

	if((g_ui16GSMRestartWaitting == 0) && (gsm_init_state == 4)){
		switch(g_eGSMState){
			case STATE_IDLE:{
				break;
			}
			case STATE_ERROR:{
				gsm_handle.phonenumber = 0;
				gsm_handle.message = 0;
				gsm_send = 0;
				g_eGSMState=STATE_EXCUTE;
				break;
			}
			case STATE_REQUEST:{
				if((gsm_send != 0) && (g_ui8GSMCommandDelay == 0)){ g_eGSMState = gsm_send(gsm_handle.command);}
			break;
			}
			case STATE_RECEIVE:{
				g_eGSMState = STATE_ERROR;
				if(gsm_receive != 0){ g_eGSMState = gsm_receive(); g_ui8GSMCommandDelay = 50;}

			break;
			}
			case STATE_EXCUTE:{
				if(g_bReadPhonebook==1){vReadPhonebook();}	//config HL6528RD and read numberphone
				else{
					vGSMUpdate();
					vCheckAccount();
					if((g_bReadSignal==1)&&(ulTimerUpdate == 0)){
						vReadSignal();
						ulTimerUpdate = 3000;
					}
				} 
				//-->Send data to LCD<--
				if((g_bSendDataLcd == 1)&&(ulTimerSendLcd == 0)){
					g_ucLcdAddr = 'A';
					vUART4Send(64);
					g_bSendDataLcd=0;
					ulTimerSendLcd = 2000;
				}
				//-->sendcommand to HL6528RD<--
				if(ui8GSMGetMsg(&gsm_handle) == 1){g_eGSMState = STATE_REQUEST;gsm_send = eGSMSendCmd;}
			break;
			}
			default:{
				g_eGSMState = STATE_IDLE;
			break;
			}
		}
	}
	status = TASK_PROCESS_FINISHED;
	return status;
}

//READ 5 THE PHONE NUMBER ON THE SIM
void vReadPhonebook(void)
{
		switch(g_cIndexReadPhone){
		//read "so1"
					case 1:	{ui8GSMSendATCmdReadNoPhone(index1_sim); g_cIndexReadPhone++;break;}
					case 2:	{eGSMWaitStatusReadNoPhone("so1",numberphone1); break;}
		//read "so2"
					case 3:	{ui8GSMSendATCmdReadNoPhone(index2_sim); g_cIndexReadPhone++;break;}
					case 4:	{eGSMWaitStatusReadNoPhone("so2",numberphone2); break;}
		//read "so3"
					case 5:	{ui8GSMSendATCmdReadNoPhone(index3_sim); g_cIndexReadPhone++;break;}
					case 6:	{eGSMWaitStatusReadNoPhone("so3",numberphone3); break;}
		//read "so4"
					case 7:	{ui8GSMSendATCmdReadNoPhone(index4_sim); g_cIndexReadPhone++;break;}
					case 8:	{eGSMWaitStatusReadNoPhone("so4",numberphone4); break;}
		//read "so5"
					case 9:	{ui8GSMSendATCmdReadNoPhone(index5_sim); g_cIndexReadPhone++;break;}
					case 10:{eGSMWaitStatusReadNoPhone("so5",numberphone5); break;}	
		//read finished
					case 11:{	g_bReadPhonebook=0;
										g_bReadSignal=1;
										g_bGSMCall=1;
										g_bGSMRevCall = 1;
										g_ucLcdInstall = STOP_SETUP;
										g_ucLcdMode = MODE_HOTLINE;
					break;
					}
					default:{break;}
		}
}
//-------------------------------------------------------
//UPDATE CODE ERRORXX, MAKE A CALL, RECEIVE THE PHONE CALL
void vGSMUpdate(void){
	tGSMMessage msg;
		uint8_t k;
		uint8_t i;
	//Update ERRORxx, Send sms error
	if((g_ucError != ERROR_NONE) && (g_bGSMSendError == 1)){
		for (k=0;k<7;k++){gsm_response_liftID[k+16] = lift_id[k];}
		gsm_response_liftID[5] = (g_ucError / 10) + 48;	//Hang chuc
		gsm_response_liftID[6] = (g_ucError % 10) + 48;	//Hang don vi
		
		msg.phonenumber = g_scGSMCenterPhoneNumber;
		msg.message = gsm_response_liftID;
		msg.command = AT_CMD_CMGS;
		ui8GSMPostMsg(msg);
		g_bGSMSendError = 0;
	}
#if(FUNCTION_CALL==1)
	//Make a call
	eGSMWaitStatusCall();
	if (g_bGSMCall == 1){
				switch (g_ui8CountButton)
				{
					case 0:{
							g_ui8SendCmdCall = ENABLE_CALL;
							if(no_carrier==1){_ledbutton_off();_gsm_audio_off();no_carrier = 0; for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;};}
					break;
					}
				//Make a call to "so1"
					case 1:{
								ui8GSMSendATCmdCall(numberphone1,'1');
								if((busy==1)|(no_carrier==1)){g_ui8CountButton = 2; busy = 0; no_carrier = 0; for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;};}
					break;
					}
					case 2:{ui8GSMSendATCmdDisCall();break;} //huy cuoc goi
				//Make a call to "so2"
					case 3:{
								ui8GSMSendATCmdCall(numberphone2,'2');
								if((busy==1)|(no_carrier==1)){g_ui8CountButton = 4; busy = 0; no_carrier = 0; for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;};}
					break;
					}
					case 4:{ui8GSMSendATCmdDisCall();break;}
				//Make a call to "so3"
					case 5:{
								ui8GSMSendATCmdCall(numberphone3,'3');
								if((busy==1)|(no_carrier==1)){g_ui8CountButton = 6; busy = 0; no_carrier = 0; for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;};}
					break;
					}
					case 6:{ui8GSMSendATCmdDisCall();break;}
				//Make a call to "so4"
					case 7:{
								ui8GSMSendATCmdCall(numberphone4,'4');
								if((busy==1)|(no_carrier==1)){g_ui8CountButton = 8; busy = 0; no_carrier = 0; for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;};}
					break;
					}
					case 8:{ui8GSMSendATCmdDisCall();break;}
				//Make a call to "so5"
					case 9:{
								ui8GSMSendATCmdCall(numberphone5,'5');
								if((busy==1)|(no_carrier==1)){g_ui8CountButton = 10; busy = 0; no_carrier = 0; for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;};}
					break;
					}
					case 10:{ui8GSMSendATCmdDisCall(); g_ui8CountButton = 0;break;}
					default: {break;}
			}
		}
	//Receive a Call
		if (g_bGSMRevCall == 1){
				if (ring == 1){
					msg.command = AT_CMD_ATA;
					ui8GSMPostMsg(msg);
					g_ucLcdMode = MODE_RECEIVE;
					g_ucLcdStatus = STATUS_VOICE;
					g_bSendDataLcd = 1;
					g_bReadSignal= 0;
					ulTimerUpdate = 2000;
					ring = 0;
				}
				if(no_carrier==1){
					g_ucLcdMode = MODE_HOTLINE;
					g_bSendDataLcd = 1;
					g_bGSMCall = 1;
					g_bReadSignal=1;
					no_carrier = 0;
					for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;}
				}
		}
	#endif
}
//--------------------------
//WRITE AT COMMAND TO BUFFER
eState eGSMSendCmd(eATCmd command){
	eState status = STATE_IDLE;
	char buf[100];
	//Command Common
			switch(command){
				case AT_CMD_AT:{ //Check Modul
					strcpy(buf,"AT");
					gsm_receive = eGSMWaitOK;
					break;
				}
				case AT_CMD_ATE0:{ //ATE0
					strcpy(buf,"ATE0");
					gsm_receive = eGSMWaitOK;
					break;
				}
				case AT_CMD_CNMI:{ //Set format of SMS message
					strcpy(buf,"AT+CNMI=2,1,0,0,0");
					gsm_receive = eGSMWaitOK;
					break;
				}
				case AT_CMD_CMGF:{ //Response in text mode
					strcpy(buf,"AT+CMGF=1");
					gsm_receive = eGSMWaitOK;
					break;
				}
				case AT_CMD_CPIN:{ //Sim ready
					strcpy(buf,"AT+CPIN?");
					gsm_receive = eGSMWaitOK;
					break;
				}
				case AT_CMD_CMGD:{ //clear all msg
					strcpy(buf,"AT+CMGD=1,4");
					gsm_receive = eGSMWaitOK;
					break;
				}
				// Read SMS #1
				case AT_CMD_CMGR:{ //AT+CMGR=1
					strcpy(buf, "AT+CMGR=1");	
					gsm_receive = eGSMWaitOK;	
				break;
				}
				case AT_CMD_CPBS:{//Choose place save
					strcpy(buf,"AT+CPBS=\"SM\"");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_CSCS:{//Choose format character
					strcpy(buf,"AT+CSCS=\"GSM\"");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_COLP:{//Connected Line Identification Presentation
					strcpy(buf,"AT+COLP=1");
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_CUSD:{
					strcpy(buf,"AT+CUSD=1,\"");
					strcat(buf,syntaxCheckAccount);	
					strcat(buf,"\",15");		
					break;
				}
				case AT_CMD_CIND:{
					strcpy(buf,"AT+CIND?");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_ATPSCPOF:{//Turn off modul
					strcpy(buf,"AT*PSCPOF");	
					ulTimerRestart = 3000;
					break;
				}
	//Config parameter voice
	#if(FUNCTION_CALL==1)
				case AT_CMD_VIP:{//Initialize Voice Parameters
					strcpy(buf,"AT+VIP=2");						
					gsm_receive = eGSMWaitOK;			
					break;
				}
				case AT_CMD_CLVL:{//Level volume
					strcpy(buf,"AT+CLVL=8");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_CRSL:{//Level ring
					strcpy(buf,"AT+CRSL=8");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_KECHO:{//Disable echo
					strcpy(buf,"AT+KECHO=1");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_KNOISE:{//Disable noise
					strcpy(buf,"AT+KNOISE=1,1");
					gsm_receive = eGSMWaitOK;	
					break;
				}
				/*case AT_CMD_KVGR:{//Receive gain selection
					strcpy(buf,"AT+KVGR=\"18\"");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_KVGT:{//Transmit gain selection
					strcpy(buf,"AT+KVGT=\"18\"");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_VGR:{//Receive gain selection
					strcpy(buf,"AT+VGR=128");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_VGT:{//Transmit gain selection
					strcpy(buf,"AT+VGT=128");	
					gsm_receive = eGSMWaitOK;	
					break;
				}
				case AT_CMD_KMAP:{//Microphone analog parameters
					strcpy(buf,"AT+KMAP=0,2,5");	
					gsm_receive = eGSMWaitOK;	
					break;
				}*/
			//Command AT for make a call
			case AT_CMD_ATD:{
				strcpy(buf, "ATD"); 					
				strcat(buf, gsm_handle.phonenumber);
				strcat(buf, ";");	
				_ledbutton_on();
				_gsm_audio_on()	;
				break;
			}
			//Command AT for hang up a call
			case AT_CMD_ATH:{
				strcpy(buf, "ATH"); 
				_ledbutton_off();
				_gsm_audio_off();
				break;
			}
			//Command AT for receive a call
			case AT_CMD_ATA:{
				strcpy(buf,"ATA"); 
				_gsm_audio_on();
				break;
			}
			//Command AT for find the phone number in the sim
			case AT_CMD_CPBF:{
				strcpy(buf, "AT+CPBF=\"so");	
					strcat(buf, indexSim);	
					strcat(buf, "\"");	
			break;
			}
#endif
	//Command AT for Send SMS
		case AT_CMD_CMGS:{
			strcpy(buf, "AT+CMGS=\""); 					
			strcat(buf, gsm_handle.phonenumber);
			strcat(buf, "\"");					
			gsm_receive = eGSMWaitSendMsg;
			break;
		}
		default:{ break;}
	}
	ui8GSMSendATCommand((char *)buf);
	return status;
}

//-----------
//WaitSendMsg
eState eGSMWaitSendMsg(void){
	char *p;
	eState status = STATE_ERROR;

	p = strstr((char *)g_ui8UART6BufferRx, "\n");
	if(p == NULL){ return status;}	
	++p;

	if(*p == '>'){
		gsm_send = eGSMSendMsg; 
		status = STATE_REQUEST;
		return status;
	}
	return status;
}

//-------
//SendMsg
eState eGSMSendMsg(eATCmd command){
	eState status = STATE_IDLE;
		gsm_receive = eGSMWaitOK;			
		ui8GSMSendATCommand2(gsm_handle.message); 
	return status;
}
//------
//WaitOK
eState eGSMWaitOK(void){
	bool search_ok = 0;
	bool search_error = 0;
	eState status = STATE_ERROR;
	tGSMMessage msg;
	
	if(strstr((char *)g_ui8UART6BufferRx, "OK") != NULL){ search_ok = 1;}
	if(strstr((char *)g_ui8UART6BufferRx, "ERROR") != NULL){search_error = 1;}
	//phan hoi "OK" --> 
	if(search_ok == 1){
		status = STATE_EXCUTE;
		return status;
	}
	//phan hoi ERR
	if(search_error == 1){
		msg.command = AT_CMD_ATPSCPOF; //--> send command AT power off module HL6528
		ui8GSMPostMsg(msg);	
		search_error = 0;
	}
		return status;
}

//----------------------------------------
//WAIT STATUS CALL: GET STATUS ON VOICE MODE
#if(FUNCTION_CALL==1)
void eGSMWaitStatusCall(void)
	{
	uint8_t i;
	if(strstr((char *)g_ui8UART6BufferRx, "+COLP") != NULL){g_ucLcdStatus = STATUS_VOICE; g_bSendDataLcd = 1;for (i=0;i<250;i++) {g_ui8UART6BufferRx[i] = 0;}}
	if(strstr((char *)g_ui8UART6BufferRx, "BUSY") != NULL){busy = 1;}
	if(strstr((char *)g_ui8UART6BufferRx, "NO CARRIER") != NULL){ no_carrier = 1;}
	if(strstr((char *)g_ui8UART6BufferRx, "RING") != NULL){ring = 1;g_bGSMCall = 0;}
	}
#endif
//-------------
//SEND AT COMMAND
uint8_t ui8GSMSendATCommand(char *p){
	uint8_t i = 0;

	if(p == NULL){ return 0;}

	for(i = 0; i < UART6_FRAME_SIZE; i++){ g_ui8UART6FrameTx[i] = 0;} // xoa frame
	strcpy((char *)g_ui8UART6FrameTx, p);
	strcat((char *)g_ui8UART6FrameTx, "\r");
	i = strlen((char *)g_ui8UART6FrameTx);
	vUART6Send(i); 
	return 1;
}

//--------------
//SEND AT COMMAND 2
uint8_t ui8GSMSendATCommand2(char *p){
	uint8_t i = 0;
	char ctrl_z[2] = {0x1A, '\0'}; // \0 = NULL

	if(p == NULL) return 0;

	for(i = 0; i< UART6_FRAME_SIZE; i++){ g_ui8UART6FrameTx[i] = 0;} 
	strcpy((char *)g_ui8UART6FrameTx, p);
	strcat((char *)g_ui8UART6FrameTx, ctrl_z);
	i = strlen((char *)g_ui8UART6FrameTx);
	vUART6Send(i);
	return 1;
}
//POST AT_CMD TO READ LEVEL SIGN
void vReadSignal (void)
{
	tGSMMessage msg;
	char *p;
	uint8_t i;
	
	msg.command = AT_CMD_CIND;	// Signal Quality
	ui8GSMPostMsg(msg);
	if(ui8GSMGetMsg(&gsm_handle) == 1){g_eGSMState = STATE_REQUEST;gsm_send = eGSMSendCmd;}
	if(strstr((char *)g_ui8UART6BufferRx, "+CIND") == NULL) {return;}
	else 
	{
		p = strstr((char *)g_ui8UART6BufferRx, "+CIND");
		for (i=0; i<9; i++){p++;}	
		g_ucSignQuality = *p;
		g_ucLcdSignal = g_ucSignQuality; 
		g_bSendDataLcd = 1;
	}
}
//POST AT_CMD TO MAKE CALL
uint8_t ui8GSMSendATCmdCall (char *numberphone,char index){
	tGSMMessage msg;
	uint8_t i;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
	if(g_ui8SendCmdCall == ENABLE_CALL){
			msg.phonenumber = numberphone;
			msg.command = AT_CMD_ATD;
			ui8GSMPostMsg(msg);
			g_ui8SendCmdCall = DISABLE_CALL;
			g_ucLcdMode = MODE_CALL;
			g_ucLcdStatus = STATUS_WAIT;
			g_ucLcdIndex = index;
			for(i=0;i<11;i++){g_ucLcdData[i+indexLCD]=numberphone[i];}
			g_bSendDataLcd = 1;
			g_bReadSignal = 0;
	}
	return 1;
}
//POST AT_CMD TO HANG UP
uint8_t ui8GSMSendATCmdDisCall (void){
	tGSMMessage msg;
	if(g_ui8SendCmdCall == DISABLE_CALL){
			msg.command = AT_CMD_ATH;
			ui8GSMPostMsg(msg);
			g_ui8SendCmdCall = ENABLE_CALL;
			g_ucLcdMode = MODE_HOTLINE;
			g_bSendDataLcd = 1;
			g_bReadSignal=1;
			ulTimerUpdate = 2000;
	}
	return 1;
}
//----------------------
//POST AT_CMD TO READ NUMBER PHONE
void ui8GSMSendATCmdReadNoPhone (char *index){
		tGSMMessage msg;
		indexSim = index;
		msg.command = AT_CMD_CPBF;
		ui8GSMPostMsg(msg);
}
//WAIT STARUS WHEN READ NUMBERPHONE
void eGSMWaitStatusReadNoPhone(const char * pString, char *numberphone)
{
		char *p;
		uint8_t i;
		if(strstr((char *)g_ui8UART6BufferRx, "+CPBF:") == NULL){return;}
		if((strstr((char *)g_ui8UART6BufferRx, pString))!= NULL){
			p = strstr((char *)g_ui8UART6BufferRx, "+CPBF");
			while(*p !='"'){p++;} //search "
				p++;
				i=0;
			while(*p != '"'){numberphone[i++]=*p; p++;}
			g_cIndexReadPhone++;
		}
}
void vCheckAccount(void)
{
	tGSMMessage msg;
	char *p;
	uint8_t i;
	
		//KIEM TRA NEU CO TIN NHAN --> gui lenh doc tin nhan
		if(strstr((char *)g_ui8UART6BufferRx, "+CMTI") != NULL){
			if(g_bSendCmdCheckAccount == 0){
				msg.command = AT_CMD_CMGR;
				ui8GSMPostMsg(msg);
				ulTimerUpdate = 60000;
				ulTimerCheckAccount = 1000;
				g_bSendCmdCheckAccount = 1;
				return;
			}
		}
		
		if((g_bSendCmdCheckAccount == 1)&&(ulTimerCheckAccount == 0)){
			if(strstr((char *)g_ui8UART6BufferRx,"KTTK") != NULL){
				p = strstr((char *)g_ui8UART6BufferRx, "KTTK"); 
				while( *p !='"'){p++;}
				p++;
				i = 0;
				while(*p != '"'){syntaxCheckAccount[i++]=*p; p++;}
				msg.command = AT_CMD_CMGD; //DELETE SMS IN SIM
				ui8GSMPostMsg(msg);
				msg.command = AT_CMD_CSCS;	
				ui8GSMPostMsg(msg);
				msg.command = AT_CMD_CUSD;	
				ui8GSMPostMsg(msg);
				return;
			}else{
				msg.command = AT_CMD_CMGD;	
				ui8GSMPostMsg(msg);
			}
			g_bSendCmdCheckAccount = 0;
			indexCheckAc = 1;
		}
		
		if((strstr((char *)g_ui8UART6BufferRx,"+CUSD: 2") != NULL)&&
			 (strstr((char *)g_ui8UART6BufferRx,"\", 15") != NULL)){	
				p = strstr((char *)g_ui8UART6BufferRx,"+CUSD");
				while( *p !='"'){p++;}
				p++;
				i=0;
				while(i<160){g_ucDataCheckAcccout[i++]=*p; p++;}
				if(ulTimerSendLcd == 0){
					switch(indexCheckAc){
							case 1: {for (i=0;i<40;i++){g_ucLcdIndex = '1';g_ucLcdData[indexLCD+i]=g_ucDataCheckAcccout[i];};indexCheckAc=2; break;} //doc data block1
							case 2: {for (i=0;i<40;i++){g_ucLcdIndex = '2';g_ucLcdData[indexLCD+i]=g_ucDataCheckAcccout[i+40];};indexCheckAc=3; break;} //doc data block2
							case 3: {for (i=0;i<40;i++){g_ucLcdIndex = '3';g_ucLcdData[indexLCD+i]=g_ucDataCheckAcccout[i+80];};indexCheckAc=4; break;} //doc data block3
							case 4: {for (i=0;i<40;i++){g_ucLcdIndex = '4';g_ucLcdData[indexLCD+i]=g_ucDataCheckAcccout[i+120];};indexCheckAc=0; break;} //doc data block4
							default : {break;}}
				}
				g_ucLcdMode = MODE_CHECKACCOUT;
				g_bSendDataLcd = 1;	
			}
			 if((ulTimerUpdate==0)&&(g_ucLcdMode == MODE_CHECKACCOUT)){
				 g_ucLcdMode = MODE_HOTLINE;
				 for (i=0;i<40;i++){g_ucLcdData[indexLCD+i] = 0;}
			 }
}

