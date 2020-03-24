#ifndef __UART2_H__
#define __UART2_H__

#define UART2_FRAME_SIZE 	30
extern uint32_t g_ui32UART2IntervalRx;
extern volatile uint32_t g_ui32UART2DelaySendRequest;
extern volatile uint8_t g_ui8UART2BufferRx[];
extern volatile uint8_t g_ui8UART2CounterRx;
extern uint8_t g_ui8UART2FrameTx[];

extern void JTAGDisable(void);
extern void JTAGEnable(void);
extern void vUART2Config(void);
extern void vUART2Handler(void);
extern void vUART2PutString(uint8_t *str);
extern void vUART2Send(uint8_t len);

#endif
