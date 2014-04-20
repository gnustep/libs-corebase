#include <CoreFoundation/CFRunLoop.h>

#include "../CFTesting.h"
#include <pthread.h>
#include <unistd.h>

int timesTimer1Fired = 0, timesTimer2Fired = 0;
void timerCallback(CFRunLoopTimerRef timer, void *info);
void timerCallback2(CFRunLoopTimerRef timer, void *info);
void* createSecondTimerThread(void* p);

int main()
{
	// Test plan:
	// Create a repeated timer, check if it repeats and invalidate it after 2 repeats.
	// Asynchronously, while the runloop is waiting, plan another, singleshot timer
	// to see, if the runloop reschedules correctly.
	// After all timers are invalid, the runloop should exit.
	
	CFRunLoopRef rl;
	CFRunLoopTimerRef timer1;
	
	rl = CFRunLoopGetMain ();
	PASS_CF(rl != NULL, "Got main run loop.");
	
	timer1 = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + 1, 1,
					0, 0, timerCallback, NULL);
	
	CFRunLoopAddTimer(rl, timer1, kCFRunLoopCommonModes);
	CFRelease(timer1);
	
	CFRunLoopRun();
	PASS_CF(timesTimer1Fired == 2, "Timer 1 fired twice and loop exited");
	
	return 0;
}

void timerCallback(CFRunLoopTimerRef timer, void *info)
{
	timesTimer1Fired++;
	
	printf("Timer 1 fired...\n");
	
	if (timesTimer1Fired == 1)
	{
		pthread_t thread;
		pthread_create(&thread, NULL, createSecondTimerThread, NULL);
		
	}
	else if (timesTimer1Fired >= 2)
		CFRunLoopTimerInvalidate(timer);
}

void timerCallback2(CFRunLoopTimerRef timer, void *info)
{
	printf("Timer 2 fired\n");
	PASS_CF(timesTimer1Fired == 1, "Timer 2 fired while timer 1 fired only once");
	
	timesTimer2Fired++;
	
	PASS_CF(timesTimer2Fired == 1, "Timer 2 fires only once");
}

void* createSecondTimerThread(void* p)
{
	CFRunLoopRef rl = CFRunLoopGetMain ();
	CFRunLoopTimerRef timer2;
	
	while (!CFRunLoopIsWaiting(rl))
		usleep(10*1000);
	
	timer2 = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + 0.25, 0,
					0, 0, timerCallback2, NULL);
	CFRunLoopAddTimer(rl, timer2, kCFRunLoopCommonModes);
	CFRelease(timer2);
	
	return NULL;
}
