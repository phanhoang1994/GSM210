#ifndef __SYSCLOCK_H__
#define __SYSCLOCK_H__

extern void vSysTickConfig(void);
extern void vSysTickHandler(void);
extern void vSysTickTimerExpired(void);
void vInput(void);

#endif
