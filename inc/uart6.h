#ifndef __UART6_H__
#define __UART6_H__

#define UART6_FRAME_SIZE 	250

typedef struct
{
	uint8_t *phonenumber;
	uint8_t *message;
} tGSMSendMsg;

extern volatile uint32_t g_ui32UART6SendRequest;
extern volatile uint32_t g_ui32UART6DelayTx;
extern volatile uint32_t g_ui32UART6DelayRx;
extern volatile uint8_t g_ui8UART6BufferRx[];
extern volatile uint8_t g_ui8UART6CounterRx;
extern uint8_t g_ui8UART6FrameTx[];

extern void vUART6Config(void);
extern void vUART6Handler(void);
extern void vUART6PutString(uint8_t *str);
extern void vUART6Send(uint8_t len);
#endif
