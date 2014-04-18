#include <CoreFoundation/CFRunLoop.h>

#include "../CFTesting.h"
#include <pthread.h>
#include <unistd.h>

CFRunLoopSourceRef rls1 = NULL;
CFRunLoopSourceRef rls2 = NULL;
pthread_t thread2 = 0;

void perform1 (void *info)
{
  SInt32 ret;
  CFIndex *i = (CFIndex*)info;
  ret = CFRunLoopRunInMode (CFSTR("another_mode"), 3.0, false);
  PASS_CF(ret == kCFRunLoopRunTimedOut, "Run loop run in 'another_mode' returned"
       " '%d'", ret);
  (*i)++;
}

void perform2 (void *info)
{
  CFIndex *i = (CFIndex*)info;
  (*i)++;
}

void* delay_signal(void* arg)
{
	//CFRunLoopSourceRef src = (CFRunLoopSourceRef) arg;
	int i;
	
	for (i = 0; i < 2; i++)
	{
		sleep(1);
	
		printf("Signalling!\n");
		CFRunLoopSourceSignal(rls1);
		CFRunLoopSourceSignal(rls2);
	}
	
	return NULL;
}

void schedule2 (void* info, CFRunLoopRef rl, CFStringRef mode)
{
	if (!thread2)
	{
		pthread_create(&thread2, NULL, delay_signal, rls2);
	}
}

int main (void)
{
  CFRunLoopRef rl;
  CFIndex i1 = 0;
  CFIndex i2 = 0;
  CFRunLoopSourceContext c1 = { 0 };
  CFRunLoopSourceContext c2 = { 0 };
  SInt32 ret;
  
  rl = CFRunLoopGetMain ();
  PASS_CF(rl != NULL, "Got main run loop.");
  
  c1.info = &i1;
  // c1.schedule = schedule1;
  c1.perform = perform1;
  c2.info = &i2;
  c2.schedule = schedule2;
  c2.perform = perform2;
  rls1 = CFRunLoopSourceCreate (NULL, 0, &c1);
  PASS_CF(rls1 != NULL, "First run loop source create.");
  rls2 = CFRunLoopSourceCreate (NULL, 0, &c2);
  PASS_CF(rls2 != NULL, "Second run loop source create.");
  
  CFRunLoopAddSource (rl, rls1, kCFRunLoopDefaultMode);
  CFRunLoopAddSource (rl, rls2, kCFRunLoopDefaultMode);
  CFRunLoopAddSource (rl, rls2, CFSTR("another_mode"));
  
  ret = CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.0, true);
  PASS_CF(ret == kCFRunLoopRunHandledSource, "Run loop handled sources.  Exit"
          " code '%d'.", ret);
  PASS_CF(i1 == 1, "First run loop source was performed %d time(s).", (int)i1);
  PASS_CF(i2 == 2, "Second run loop source was performed %d time(s).", (int)i2);
  
  CFRunLoopRemoveSource (rl, rls2, CFSTR("another_mode"));
  CFRunLoopSourceInvalidate (rls2);
  CFRunLoopSourceInvalidate (rls1);
  
  ret = CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.0, true);
  PASS_CF(ret == kCFRunLoopRunFinished, "Run loop handled sources.  Exit"
          " code '%d'.", ret);
  
  CFRelease (rls1);
  CFRelease (rls2);
  CFRelease (rl);
  
  return 0;
}
