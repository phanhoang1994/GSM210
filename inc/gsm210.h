/*
File:
Creater: Phan Van Hoang
Date: 1-1-2019
*/

#ifndef	__GSM210_H__
#define __GSM210_H__

#define const_sw_end_delay 			2000

// Define port
//---------------------------------------------------------

#define _ledrun_on()					{GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);}
#define _ledrun_off()					{GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);}

#define _gsm_pwrkey_on()			{GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);}
#define _gsm_pwrkey_off()			{GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0);}

#define _gsm_audio_on()			{GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_5, GPIO_PIN_5);}
#define _gsm_audio_off()				{GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_5, 0);}

#define _ledbutton_on()				{GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2);}
#define _ledbutton_off()			{GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);}

#define DIR_ON()							{GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6,GPIO_PIN_6);}
#define DIR_OFF()							{GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6,0);}

/* data send to lcd
		ucUART4FrameTx[0] --> thong bao cai dat khi bat dien
		ucUART4FrameTx[1] --> thong bao muc song dien thoai
		ucUART4FrameTx[2] --> trang thai cuoc goi
		ucUART4FrameTx[3] --> ten thue bao
		ucUART4FrameTx[4-13] --> so thue bao

*/
#define ADDRESS										'A'
#define START_SETUP								'S'
#define STOP_SETUP								'F'	

#define MODE_HOTLINE							'H'
#define MODE_CALL									'C'
#define MODE_RECEIVE							'R'
#define MODE_CHECKACCOUT					'M'

#define STATUS_WAIT								'W'
#define STATUS_VOICE							'V'

#define g_ucLcdAddr               ucUART4FrameTx[0]
#define g_ucLcdInstall            ucUART4FrameTx[1]
#define g_ucLcdSignal             ucUART4FrameTx[2]
#define g_ucLcdMode               ucUART4FrameTx[3]
#define g_ucLcdStatus             ucUART4FrameTx[4]
#define g_ucLcdIndex              ucUART4FrameTx[5] //use for name number phone or index check account
#define g_ucLcdData               ucUART4FrameTx    //use for number phone or data check account 
#define indexLCD                     6

#endif

