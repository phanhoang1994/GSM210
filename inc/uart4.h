#ifndef __UART4_H__
#define __UART4_H__

#define UART4_FRAME_SIZE 				64
typedef enum
{
    STATE_RX4_INIT,          
    STATE_RX4_ERROR,         
    STATE_RX4_IDLE,          
    STATE_RX4_RECEIVE        
} eUartStateHandler;

extern void vUART4Config(void);
extern void vUART4Handler(void);
extern void vUART4PutString(unsigned char *pucStr);
extern void vUART4Send(unsigned char ucLen);
#endif
