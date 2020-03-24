#include "inc/global.h"
#include "inc/event.h"

#define EV_QUEUE_SIZE		30

#if EV_QUEUE_SIZE > 255
	#error "In event.h, event queue size is defined too large, please define it again! "
#endif
 
eEvent event_queue[EV_QUEUE_SIZE];
uint8_t event_counter;
uint8_t event_write_index;
uint8_t event_read_index;

uint8_t ui8PostEvent(eEvent event)
{
	event_queue[event_write_index] = event;
	if(++event_counter == EV_QUEUE_SIZE) event_counter = 0;
    if(++event_write_index == EV_QUEUE_SIZE) event_write_index = 0;
    return 1;
}

uint8_t ui8GetEvent(eEvent *event)
{
    if(event_counter!=0)
	{
    	*event = event_queue[event_read_index];
		if(++event_read_index == EV_QUEUE_SIZE) event_read_index = 0;
    	--event_counter;
		return 1;
	}
    return 0;
}
