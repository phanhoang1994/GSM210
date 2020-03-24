#ifndef __EVENT_H__
#define __EVENT_H__

typedef enum
{
	EV_IDLE,
	EV_GSM_ERROR,
	EV_GSM_SEND_COMPLETE,
	EV_GSM_RECEIVE_COMPLETE,
} eEvent;

extern uint8_t ui8PostEvent(eEvent event);
extern uint8_t ui8GetEvent(eEvent *event);

#endif
