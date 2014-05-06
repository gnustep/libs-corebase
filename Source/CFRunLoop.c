/* CFRunLoop.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Author: Stefan Bidigaray <stefanbidi@gmail.com>
   Date: August, 2012
   
   Author: Lubos Dolezel <lubos@dolezel.info>
   Date: May, 2014
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFSet.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"

#ifndef _WIN32
#	include <unistd.h>
#	include <fcntl.h>
#	include <poll.h>
#	include <limits.h>
#endif
#ifdef HAVE_LIBDISPATCH
#	include <dispatch/dispatch.h>
#endif

/* From NSDate.m in GNUstep-base */
#define DISTANT_FUTURE	63113990400.0

static CFTypeID _kCFRunLoopTypeID = 0;
static CFTypeID _kCFRunLoopSourceTypeID = 0;
static CFTypeID _kCFRunLoopObserverTypeID = 0;
static CFTypeID _kCFRunLoopTimerTypeID = 0;

static CFRunLoopRef static_mainLoop = NULL;
static pthread_key_t static_loopKey;

CONST_STRING_DECL(kCFRunLoopDefaultMode, "kCFRunLoopDefaultMode");
CONST_STRING_DECL(kCFRunLoopCommonModes, "kCFRunLoopCommonModes");

typedef struct GSRunLoopContext * GSRunLoopContextRef;
struct GSRunLoopContext
{
  CFRunLoopActivity activities;
  CFMutableArrayRef timers;
  CFMutableSetRef observers;
  CFMutableSetRef sources0set;
  CFMutableArrayRef sources0;
  CFMutableSetRef sources1; /* This is only a place holder for now. */
  CFMutableArrayRef blocks;
};

struct __CFRunLoop
{
  CFRuntimeBase     parent;
  GSMutex           _lock;
  CFStringRef       _currentMode;
  CFMutableSetRef   _commonModes;
  CFMutableArrayRef _commonObjects;
  CFMutableDictionaryRef _contexts;

  int     _wakeUpPipe[2];
  Boolean _isWaiting; /* Whether the runloop is currently in a select/poll call */
  Boolean _stop; /* Whether the runloop was told to stop */
};

struct __CFRunLoopSource
{
  CFRuntimeBase parent;
  GSMutex       _lock;
  CFIndex       _order;
  Boolean       _isSignaled; // version 0 only
  Boolean       _isValid;
  union
    {
      CFRunLoopSourceContext _context;
      CFRunLoopSourceContext1 _context1;
    };
  CFRunLoopRef   _runloop;
};

struct __CFRunLoopObserver
{
  CFRuntimeBase parent;
  GSMutex       _lock;
  CFOptionFlags _activities;
  CFIndex       _order;
  Boolean       _repeats;
  Boolean       _isValid;
  CFRunLoopObserverCallBack _callback;
  CFRunLoopObserverContext  _context;
  CFRunLoopRef   _runloop;
};

struct __CFRunLoopTimer
{
  CFRuntimeBase  base;
  GSMutex        _lock;
  CFAbsoluteTime _nextFireDate;
  CFTimeInterval _interval;
  CFIndex        _order;
  Boolean        _isValid;
  CFRunLoopTimerCallBack _callback;
  CFRunLoopTimerContext  _context;
  CFRunLoopRef   _runloop;
};

static GSRunLoopContextRef
GSRunLoopContextNew (CFAllocatorRef alloc);

static void
GSRunLoopContextFinalize (GSRunLoopContextRef ctxt);

static void
GSRunLoopContextDeallocFunc (const void *key, const void *value, void *ctxt);

static void
CFRunLoopTimerRemoveFromRunLoop(CFRunLoopRef rl, CFRunLoopTimerRef timer);

static const void *
BlockRetainCallback(CFAllocatorRef allocator, const void *value)
{
  return (const void *) _Block_copy((void *) value);
}

static void
BlockReleaseCallback(CFAllocatorRef allocator, const void *value)
{
  _Block_release((void *) value);
}

static const CFArrayCallBacks _kBlockArrayCallbacks = {
  .version = 0,
  .retain = BlockRetainCallback,
  .release = BlockReleaseCallback,
  .copyDescription = NULL,
  .equal = NULL
};

static void
CFRunLoopFinalize (CFTypeRef cf)
{
  CFRunLoopRef rl = (CFRunLoopRef)cf;
  CFAllocatorRef alloc = CFGetAllocator (rl);

  CFRelease (rl->_commonModes);
  CFRelease (rl->_commonObjects);
  CFDictionaryApplyFunction (rl->_contexts, GSRunLoopContextDeallocFunc,
                             (void*)alloc);
  CFRelease (rl->_contexts);
}

static void
CFRunLoopSourceFinalize (CFTypeRef cf)
{
  CFRunLoopSourceRef source = (CFRunLoopSourceRef)cf;
  if (source->_context.release && source->_context.info)
    source->_context.release (source->_context.info);
}

static void
CFRunLoopObserverFinalize (CFTypeRef cf)
{
  CFRunLoopObserverRef rlo = (CFRunLoopObserverRef)cf;
  if (rlo->_context.release && rlo->_context.info)
    rlo->_context.release (rlo->_context.info);
}

static void
CFRunLoopTimerFinalize (CFTypeRef cf)
{
  CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)cf;
  if (rlt->_context.release && rlt->_context.info)
    rlt->_context.release (rlt->_context.info);
}

static CFRuntimeClass CFRunLoopClass =
{
  0,
  "CFRunLoop",
  NULL,
  NULL,
  CFRunLoopFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

static CFRuntimeClass CFRunLoopSourceClass =
{
  0,
  "CFRunLoopSource",
  NULL,
  NULL,
  CFRunLoopSourceFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

static CFRuntimeClass CFRunLoopObserverClass =
{
  0,
  "CFRunLoopObserver",
  NULL,
  NULL,
  CFRunLoopObserverFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

static CFRuntimeClass CFRunLoopTimerClass =
{
  0,
  "CFRunLoopTimer",
  NULL,
  NULL,
  CFRunLoopTimerFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

#define CFRUNLOOP_SIZE \
  sizeof(struct __CFRunLoop) - sizeof(CFRuntimeBase)

void
CFRunLoopInitialize (void)
{
  _kCFRunLoopTypeID = _CFRuntimeRegisterClass (&CFRunLoopClass);
  _kCFRunLoopSourceTypeID = _CFRuntimeRegisterClass (&CFRunLoopSourceClass);
  _kCFRunLoopObserverTypeID = _CFRuntimeRegisterClass (&CFRunLoopObserverClass);
  _kCFRunLoopTimerTypeID = _CFRuntimeRegisterClass (&CFRunLoopTimerClass);
}

CFTypeID
CFRunLoopGetTypeID (void)
{
  return _kCFRunLoopTypeID;
}

CFTypeID
CFRunLoopSourceGetTypeID (void)
{
  return _kCFRunLoopSourceTypeID;
}

CFTypeID
CFRunLoopObserverGetTypeID (void)
{
  return _kCFRunLoopObserverTypeID;
}

CFTypeID
CFRunLoopTimerGetTypeID (void)
{
  return _kCFRunLoopTimerTypeID;
}

static GSRunLoopContextRef
GSRunLoopContextNew (CFAllocatorRef alloc)
{
  GSRunLoopContextRef new;
  
  new = CFAllocatorAllocate (alloc, sizeof(struct GSRunLoopContext), 0);
  if (new)
    {
      new->timers = CFArrayCreateMutable (alloc, 0, &kCFTypeArrayCallBacks);
      new->observers = CFSetCreateMutable (alloc, 0, &kCFTypeSetCallBacks);
      new->sources0 = CFArrayCreateMutable (alloc, 0, &kCFTypeArrayCallBacks);
      new->sources0set = CFSetCreateMutable (alloc, 0, &kCFTypeSetCallBacks);
      new->sources1 = CFSetCreateMutable (alloc, 0, &kCFTypeSetCallBacks);
      new->blocks = CFArrayCreateMutable (alloc, 0, &_kBlockArrayCallbacks);
    }
  
  return new;
}

static void
GSRunLoopContextFinalize (GSRunLoopContextRef ctxt)
{
  CFRelease (ctxt->timers);
  CFRelease (ctxt->observers);
  CFRelease (ctxt->sources0);
  CFRelease (ctxt->sources0set);
  CFRelease (ctxt->sources1);
  CFRelease (ctxt->blocks);
}

static void
GSRunLoopContextDeallocFunc (const void *key, const void *value, void *ctxt)
{
  GSRunLoopContextFinalize ((GSRunLoopContextRef)value);
  CFAllocatorDeallocate ((CFAllocatorRef)ctxt, (void*)value);
}

static CFComparisonResult
Context0Comparator (const void *val1, const void *val2, void *ctxt)
{
  CFRunLoopSourceRef s1 = (CFRunLoopSourceRef) val1;
  CFRunLoopSourceRef s2 = (CFRunLoopSourceRef) val2;
  
  if (s1->_order < s2->_order)
    return kCFCompareLessThan;
  else if (s1->_order > s2->_order)
    return kCFCompareGreaterThan;
  else
    return kCFCompareEqualTo;
}

static void
GSRunLoopContextAddFunc (const void *value, void *context)
{
  CFTypeID typeID = CFGetTypeID (value);
  GSRunLoopContextRef ctxt = (GSRunLoopContextRef)context;
  if (typeID == _kCFRunLoopSourceTypeID)
    {
      CFRunLoopSourceRef source = (CFRunLoopSourceRef)value;
      if (source->_context.version == 0)
        {
          if (!CFSetContainsValue(ctxt->sources0set, value))
            {
              CFSetAddValue (ctxt->sources0set, value);
              CFArrayAppendValue (ctxt->sources0, value);
              CFArraySortValues (ctxt->sources0,
                                 CFRangeMake(0, CFArrayGetCount (ctxt->sources0)),
                                 Context0Comparator, NULL);
            }
        }
      else if (source->_context.version == 1)
        CFSetAddValue (ctxt->sources1, value);
    }
  else if (typeID == _kCFRunLoopObserverTypeID)
    {
      CFSetAddValue (ctxt->observers, value);
    }
  else if (typeID == _kCFRunLoopTimerTypeID)
    {
      CFRange range = CFRangeMake (0, CFArrayGetCount (ctxt->timers));
      if (CFArrayContainsValue (ctxt->timers, range, value) == false)
        CFArrayAppendValue (ctxt->timers, value);
    }
}

static GSRunLoopContextRef
GSRunLoopContextGet (CFRunLoopRef rl, CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  /* If the modes doesn't exist, create it */
  if (!CFDictionaryGetValueIfPresent (rl->_contexts, mode,
                                      (const void**)&ctxt))
    {
      ctxt = GSRunLoopContextNew (CFGetAllocator (rl));
      CFDictionaryAddValue (rl->_contexts, mode, (const void*)ctxt);
    }
  return ctxt;
}

static CFRunLoopRef
CFRunLoopCreate (void)
{
  CFRunLoopRef rl;

  rl = (CFRunLoopRef)_CFRuntimeCreateInstance (kCFAllocatorDefault,
                                               _kCFRunLoopTypeID,
                                               CFRUNLOOP_SIZE,
                                               0);
  rl->_commonModes = CFSetCreateMutable(kCFAllocatorDefault,
                                        0, &kCFTypeSetCallBacks);
  rl->_commonObjects = CFArrayCreateMutable(kCFAllocatorDefault,
                                        0, &kCFTypeArrayCallBacks);
  rl->_contexts = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                        0, &kCFTypeDictionaryKeyCallBacks,
                                        NULL);

  CFSetAddValue(rl->_commonModes, kCFRunLoopDefaultMode);
  
  pipe(rl->_wakeUpPipe);
  fcntl(rl->_wakeUpPipe[0], F_SETFL, O_NONBLOCK);
  fcntl(rl->_wakeUpPipe[1], F_SETFL, O_NONBLOCK);

  return rl;
}

static void
_CFRunLoopCreateThreadKey (void)
{
  pthread_key_create(&static_loopKey, (void(*)(void*)) CFRelease);
}

CFRunLoopRef
CFRunLoopGetCurrent (void)
{
  static pthread_once_t once = PTHREAD_ONCE_INIT;
  CFRunLoopRef rl;

  pthread_once(&once, _CFRunLoopCreateThreadKey);

  rl = pthread_getspecific(static_loopKey);
  if (rl == NULL)
    {
      rl = CFRunLoopCreate();
      pthread_setspecific(static_loopKey, rl);
    }

  return rl;
}

static void
_CFRunLoopCreateMain (void)
{
  static_mainLoop = CFRunLoopGetCurrent();
}

CFRunLoopRef
CFRunLoopGetMain (void)
{
  static pthread_once_t once = PTHREAD_ONCE_INIT;
  pthread_once(&once, _CFRunLoopCreateMain);
  return static_mainLoop;
}

void
CFRunLoopRun (void)
{
  SInt32 code;
  do
    {
      code = CFRunLoopRunInMode (kCFRunLoopDefaultMode, DISTANT_FUTURE, false);
    } while (code != kCFRunLoopRunFinished && code != kCFRunLoopRunStopped);
}

static void
CFRunLoopNotifyObservers (CFRunLoopRef rl, GSRunLoopContextRef context, CFRunLoopActivity activity)
{
  CFRunLoopObserverRef *observers;
  CFIndex i, count;

  GSMutexLock (&rl->_lock);
  count = CFSetGetCount(context->observers);
  observers = (CFRunLoopObserverRef*) CFAllocatorAllocate(NULL,
                                   sizeof(CFRunLoopObserverRef)*count, 0);
  CFSetGetValues(context->observers, (const void**) observers);
  GSMutexUnlock (&rl->_lock);

  for (i = 0; i < count; i++)
    CFRetain(observers[i]);

  for (i = 0; i < count; i++)
    {
      CFRunLoopObserverRef observer = observers[i];

      if (observer->_isValid && observer->_activities & activity)
        {
          observer->_callback(observer, activity, observer->_context.info);

          if (!observer->_repeats)
            observer->_isValid = false;
        }

      CFRelease(observer);
    }

  CFAllocatorDeallocate(NULL, (void*) observers);
}

static Boolean
CFRunLoopProcessTimers (CFRunLoopRef rl, CFAbsoluteTime now,
                        GSRunLoopContextRef context, Boolean singleSource)
{
  CFIndex i, count;
  CFRunLoopTimerRef *timers;
  Boolean hadTimer = false;

  CFRunLoopNotifyObservers(rl, context, kCFRunLoopBeforeTimers);

  GSMutexLock (&rl->_lock);
  // Make a copy of timers so that we don't hold the mutex when using the callback
  // and cause a deadlock.
  count = CFArrayGetCount(context->timers);
  timers = (CFRunLoopTimerRef*) CFAllocatorAllocate(NULL,
                                sizeof(CFRunLoopTimerRef)*count, 0);

  CFArrayGetValues(context->timers, CFRangeMake(0, count), (const void**) timers);
  GSMutexUnlock (&rl->_lock);

  for (i = 0; i < count; i++)
    CFRetain(timers[i]);

  for (i = 0; i < count; i++)
    {
      CFRunLoopTimerRef timer = timers[i];
      CFAbsoluteTime nextFireDate = CFRunLoopTimerGetNextFireDate(timer);
      
      if (CFRunLoopTimerIsValid(timer))
        {
          if (nextFireDate < now || fabs(now - nextFireDate) < 0.001)
            {
              hadTimer = true;

              if (CF_IS_OBJC(_kCFRunLoopTimerTypeID, timer))
                CF_OBJC_VOIDCALLV(timer, "fire");
              else
                timer->_callback(timer, timer->_context.info);

              // Compute the next time
              if (!CFRunLoopTimerDoesRepeat(timer))
                CFRunLoopTimerInvalidate(timer);
            }
          else
            CFRunLoopTimerSetNextFireDate(timer, now + CFRunLoopTimerGetInterval(timer));
        }
      else
        CFRunLoopTimerRemoveFromRunLoop(rl, timer);

      CFRelease(timer);
      
      if (singleSource && hadTimer)
        break;
    }

  CFAllocatorDeallocate(NULL, (void*) timers);
  return hadTimer;
}

static Boolean
CFRunLoopProcessSourcesVersion1 (CFRunLoopRef rl, CFAbsoluteTime now,
		GSRunLoopContextRef context, struct pollfd* pfd, int count)
{
  int i;
  Boolean hadSource = false;

  for (i = 0; i < count; i++)
    {
	  if (pfd[i].revents != 0)
        {
          hadSource = true;
          // TODO: find the corresponding source
        }
    }

  return hadSource;
}

static Boolean
CFRunLoopProcessSourcesVersion0 (CFRunLoopRef rl, CFAbsoluteTime now,
		GSRunLoopContextRef context, Boolean singleSource)
{
  CFIndex i, count;
  CFRunLoopSourceRef *sources;
  Boolean hadSource = false;

  // Notify observers with kCFRunLoopBeforeSources activity.
  CFRunLoopNotifyObservers(rl, context, kCFRunLoopBeforeSources);

  GSMutexLock (&rl->_lock);
  count = CFArrayGetCount(context->sources0);
  sources = (CFRunLoopSourceRef*) CFAllocatorAllocate(NULL,
                                  sizeof(CFRunLoopSourceRef)*count, 0);

  CFArrayGetValues(context->sources0, CFRangeMake(0, CFArrayGetCount(context->sources0)),
                   (const void**) sources);
  GSMutexUnlock (&rl->_lock);

  for (i = 0; i < count; i++)
    CFRetain(sources[i]);

  for (i = 0; i < count; i++)
    {
      CFRunLoopSourceRef source = sources[i];

      if (source->_isValid && source->_isSignaled)
        {
          hadSource = true;
          source->_isSignaled = false;
          source->_context.perform(source->_context.info);
        }

      CFRelease(source);
      
      if (singleSource && hadSource)
        break;
    }

  CFAllocatorDeallocate(NULL, (void*) sources);

  return hadSource;
}

static void
CFRunLoopPerformBlocks (GSRunLoopContextRef context)
{
  CFIndex i, count;

  count = CFArrayGetCount(context->blocks);

  for (i = 0; i < count; i++)
    {
      PerformBlockType block = (PerformBlockType)
                               CFArrayGetValueAtIndex(context->blocks, i);
      CALL_BLOCK_NO_ARGS(block);
    }

  CFArrayRemoveAllValues(context->blocks);
}

static void
CFRunLoopHasAnyValidSources_SourceApplier(const void *value, void *context)
{
  CFRunLoopSourceRef source = (CFRunLoopSourceRef) value;
  Boolean* hasSource = (Boolean*) context;
  
  if (source->_isValid)
    *hasSource = true;
}

static void
CFRunLoopHasAnyValidSources_TimerApplier(const void *value, void *context)
{
  CFRunLoopTimerRef timer = (CFRunLoopTimerRef) value;
  Boolean* hasSource = (Boolean*) context;
  
  if (timer->_isValid)
    *hasSource = true;
}

static Boolean
CFRunLoopHasAnyValidSources (CFRunLoopRef rl, GSRunLoopContextRef context)
{
  Boolean hasSource = false;
  
  GSMutexLock (&rl->_lock);
  CFArrayApplyFunction(context->sources0, CFRangeMake(0, CFArrayGetCount(context->sources0)),
                       CFRunLoopHasAnyValidSources_SourceApplier,
                       &hasSource);
  
  if (!hasSource)
    {
      CFArrayApplyFunction(context->timers, CFRangeMake(0,
                           CFArrayGetCount(context->timers)),
                           CFRunLoopHasAnyValidSources_TimerApplier,
                           &hasSource);
    }
  GSMutexUnlock (&rl->_lock);
  
  return hasSource;
}

Boolean
_CFRunLoopHasAnyValidSources (CFRunLoopRef rl, CFStringRef mode)
{
  GSRunLoopContextRef context = GSRunLoopContextGet(rl, mode);
  return CFRunLoopHasAnyValidSources(rl, context);
}

static void
Source1Applier(const void *value, void *context)
{
  struct pollfd** ppfd = (struct pollfd**) context;
  CFRunLoopSourceRef src = (CFRunLoopSourceRef) value;
  mach_port_t port;

  port = src->_context1.getPort(src->_context1.info);
  (*ppfd)->fd = port;
  (*ppfd)->events = POLLIN | POLLOUT;

  (*ppfd)++;
}

static struct pollfd*
CFRunLoopPrepareForPoll(struct pollfd* pfd, int* numSources,
        CFRunLoopRef rl, GSRunLoopContextRef context)
{
  int newSourceCount = 2;
  struct pollfd* pos;

  newSourceCount += CFSetGetCount(context->sources1);

  if (pfd == NULL)
    {
      pfd = (struct pollfd*) CFAllocatorAllocate(NULL,
                newSourceCount * sizeof(struct pollfd), 0);

      pfd[0].fd = rl->_wakeUpPipe[0];
      pfd[0].events = POLLIN;

#if HAVE_LIBDISPATCH
      if (CFRunLoopGetCurrent() == CFRunLoopGetMain())
        {
          // integrate libdispatch
          pfd[1].fd = dispatch_get_main_queue_eventfd_np();
          pfd[1].events = POLLIN;
        }
          else
#endif
        {
          pfd[1].fd = -1;
        }
    }
  else
    {
      pfd = (struct pollfd*) CFAllocatorReallocate(NULL,
              pfd, newSourceCount * sizeof(struct pollfd), 0);
    }

  pos = &pfd[2];

  GSMutexLock (&rl->_lock);
  CFSetApplyFunction(context->sources1, Source1Applier, &pos);
  GSMutexUnlock (&rl->_lock);
  
  *numSources = newSourceCount;

  return pfd;
}

SInt32
CFRunLoopRunInMode (CFStringRef mode, CFTimeInterval seconds,
                    Boolean returnAfterSourceHandled)
{
  /* This is the sequence of events:
   * 1. Notify observers with kCFRunLoopEntry activity.
   * 2. Notify observers with kCFRunLoopBeforeTimers activitiy.
   * 3. Notify observers with kCFRunLoopBeforeSources activity.
   * 4. Fire version 0 sources.
   * 5. If a version 1 source is ready and waiting to fire, process the
   *    event immediately. Go to step 9.
   * 6. Notify observers with kCFRunLoopBeforeWaiting activity.
   * 7. Put the thread to sleep until one of the following events occurs:
   *   - An event arrives for a port-based input source.
   *   - A timer fires.
   *   - The seconds argument expires.
   *   - Someone calls CFRunLoopWakeUp(). 
   * 8. Notify observers with kCFRunLoopAfterWaiting activity.
   * 9. Process the pending event.
   *   - Process timers with fire dates in the past.
   *   - If an input source fired, deliver the event.
   *   - If CFRunLoopWakeUp() was called but the run loop has not timed out,
   *     restart the loop. Go to step 2.
   * 10. Notify observers with kCFRunLoopExit activity.
   */
  CFRunLoopRef rl = CFRunLoopGetCurrent();
  const CFAbsoluteTime timeStart = CFAbsoluteTimeGetCurrent();
  CFAbsoluteTime timeNow = timeStart;
  SInt32 exitReason = 0;
  Boolean hadSource = false;
  struct pollfd* pfd = NULL;
  int numSources = 0;
  GSRunLoopContextRef context = GSRunLoopContextGet(rl, mode);

  rl->_currentMode = mode;

  // Notify observers with kCFRunLoopEntry activity.
  CFRunLoopNotifyObservers(rl, context, kCFRunLoopEntry);

  CFRunLoopProcessTimers(rl, timeStart, context, returnAfterSourceHandled);
  if (!returnAfterSourceHandled || !hadSource)
    hadSource |= CFRunLoopProcessSourcesVersion0(rl, timeStart, context, returnAfterSourceHandled);
  GSMutexUnlock (&rl->_lock);

  if (returnAfterSourceHandled && hadSource)
    {
      exitReason = kCFRunLoopRunHandledSource;
    }

  timeNow = CFAbsoluteTimeGetCurrent();

  while (exitReason == 0)
    {
      int sourcesFired, timeout;

      hadSource = false;

      if (!returnAfterSourceHandled)
        {
          if (seconds == 0)
            timeout = 0; // only one pass
          else if (seconds >= DISTANT_FUTURE)
            timeout = -1;
          else
            timeout = (int) ((seconds - (timeNow - timeStart)) * 1000);
        }
      else
        {
          timeout = -1;
        }
        
      if (!CFRunLoopHasAnyValidSources(rl, context))
        {
          exitReason = kCFRunLoopRunFinished;
          break;
        }

      if (timeout != 0)
        {
          // Check all timers in current mode and plan the timeout accordingly.
          CFAbsoluteTime nextTimer = CFRunLoopGetNextTimerFireDate(rl, mode);
          if (nextTimer < DISTANT_FUTURE)
            {
              int delay = (int) ( (nextTimer - timeNow)*1000 );
              // printf("(%f-%f)*1000=%d\n", nextTimer, timeNow, delay);
              if (timeout == -1 || delay < timeout)
                timeout = delay;
              if (timeout < 0)
                timeout = 0;
            }
        }

      if (rl->_stop)
        {
          exitReason = kCFRunLoopRunStopped;
          rl->_stop = false;
          break;
        }

      GSMutexLock (&rl->_lock);
      CFRunLoopPerformBlocks(context);
      GSMutexUnlock (&rl->_lock);
        
      // Notify observers with kCFRunLoopBeforeWaiting activity.
      CFRunLoopNotifyObservers(rl, context, kCFRunLoopBeforeWaiting);
      rl->_isWaiting = true;

      pfd = CFRunLoopPrepareForPoll(pfd, &numSources, rl, context);

      // printf("poll: %d ms\n", timeout);
      // printf("poll %d sources\n", numSources);
      sourcesFired = poll(pfd, numSources, timeout);

      rl->_isWaiting = false;
      // Notify observers with kCFRunLoopAfterWaiting activity.
      CFRunLoopNotifyObservers(rl, context, kCFRunLoopAfterWaiting);

      if (sourcesFired < 0) // error
        {
          // TODO: print explanation into the console?
          exitReason = kCFRunLoopRunFinished;
          break;
        }
      else if (sourcesFired > 0)
        {
          if (pfd[0].revents != 0)
            {
              int dummy;
              // printf("loop woken up!\n");

              // Remove everything from the notification pipe that woke us up
              while (read(pfd[0].fd, &dummy, sizeof(dummy)) > 0);
            }

#if HAVE_LIBDISPATCH
          if (pfd[1].revents != 0)
            {
              dispatch_main_queue_drain_np();
              hadSource = true;
            }
#endif

        }
        
      CFRunLoopProcessTimers(rl, timeNow, context,
                                          returnAfterSourceHandled);
      
      if (!returnAfterSourceHandled || !hadSource)
        {
          hadSource |= CFRunLoopProcessSourcesVersion0(rl, timeNow, context,
                                                   returnAfterSourceHandled);
        }
      if (!returnAfterSourceHandled || !hadSource)
        {
          hadSource |= CFRunLoopProcessSourcesVersion1(rl, timeNow, context,
                                                   &pfd[2], numSources-2);
        }

      if (returnAfterSourceHandled && hadSource)
        {
          exitReason = kCFRunLoopRunHandledSource;
          break;
        }

      timeNow = CFAbsoluteTimeGetCurrent();
      if (CFAbsoluteTimeGetCurrent() >= timeStart+seconds)
        {
          exitReason = kCFRunLoopRunTimedOut;
          break;
        }
    }

  // Notify observers with kCFRunLoopExit activity.
  CFRunLoopNotifyObservers(rl, context, kCFRunLoopExit);
  rl->_currentMode = NULL;

  CFAllocatorDeallocate(NULL, pfd);

  return exitReason;
}

void
CFRunLoopWakeUp (CFRunLoopRef rl)
{
  if (CFRunLoopIsWaiting(rl))
    {
      int dummy = 1;
      write(rl->_wakeUpPipe[1], &dummy, sizeof(dummy));
    }
}

void
CFRunLoopStop (CFRunLoopRef rl)
{
  if (rl->_currentMode != NULL)
    rl->_stop = true;
}

Boolean
CFRunLoopIsWaiting (CFRunLoopRef rl)
{
  return rl->_isWaiting;
}

void
CFRunLoopAddCommonMode (CFRunLoopRef rl, CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  
  if (CFSetContainsValue (rl->_commonModes, mode) == false)
    {
      GSRunLoopContextRef new;
      CFIndex cnt;
      CFSetAddValue (rl->_commonModes, mode);
      new = GSRunLoopContextNew (CFGetAllocator(rl));
      cnt = CFArrayGetCount(rl->_commonObjects);
      CFArrayApplyFunction (rl->_commonObjects, CFRangeMake (0, cnt),
                            GSRunLoopContextAddFunc, new);
      CFDictionaryAddValue (rl->_contexts, mode, new);
    }
  
  GSMutexUnlock (&rl->_lock);
}

static void
CFRunLoopCopyKeysFunc (const void *key, const void *value, void *context)
{
  CFMutableArrayRef array = (CFMutableArrayRef)context;
  CFArrayAppendValue (array, key);
}

CFArrayRef
CFRunLoopCopyAllModes (CFRunLoopRef rl)
{
  CFArrayRef ret;
  CFMutableArrayRef a;
  CFIndex cnt;
  
  GSMutexLock (&rl->_lock);
  
  cnt = CFDictionaryGetCount (rl->_contexts);
  a = CFArrayCreateMutable (NULL, cnt, &kCFTypeArrayCallBacks);
  CFDictionaryApplyFunction (rl->_contexts, CFRunLoopCopyKeysFunc, a);
  
  GSMutexUnlock (&rl->_lock);
  
  ret = CFArrayCreateCopy (NULL, a);
  CFRelease (a);
  
  return ret;
}

CFStringRef
CFRunLoopCopyCurrentMode (CFRunLoopRef rl)
{
  CFStringRef mode = rl->_currentMode;

  if (mode != NULL)
    return CFRetain (rl->_currentMode);
  else
    return NULL;
}

static void
CFRunLoopPerformBlock_nolock (CFRunLoopRef rl, CFTypeRef mode,
                              PerformBlockType block)
{
  if (CFGetTypeID(mode) == CFStringGetTypeID())
    {
      GSRunLoopContextRef ctxt;

      ctxt = GSRunLoopContextGet (rl, mode);
      CFArrayAppendValue(ctxt->blocks, block);
    }
  else if (CFGetTypeID(mode) == CFArrayGetTypeID())
    {
      CFIndex i, count;
      CFArrayRef array = (CFArrayRef) mode;
      
      count = CFArrayGetCount(array);

      for (i = 0; i < count; i++)
        {
          CFTypeRef m = CFArrayGetValueAtIndex(array, i);
          
          CFRunLoopPerformBlock_nolock(rl, m, block);
        }
    }
}

void
CFRunLoopPerformBlock (CFRunLoopRef rl, CFTypeRef mode, PerformBlockType block)
{
  GSMutexLock (&rl->_lock);
  CFRunLoopPerformBlock_nolock(rl, mode, block);
  GSMutexUnlock (&rl->_lock);
}



/* The next few function are used to add objects to the common modes
 */
struct common_mode_info
{
  CFRunLoopRef rl;
  CFTypeRef obj;
  Boolean ret;
};

/* Call the _nolock variant of the Add, Remove and Contain functions
 * when the run loop is already locked.
 */
static void
CFRunLoopAddSource_nolock (CFRunLoopRef rl, CFRunLoopSourceRef source,
                           CFStringRef mode)
{
  GSRunLoopContextRef ctxt;

  if (source->_runloop != NULL && source->_runloop != rl)
    return; // This source is already added elsewhere

  source->_runloop = rl;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  if (source->_context.version == 0)
    {
      if (!CFSetContainsValue(ctxt->sources0set, source))
        {
          CFSetAddValue (ctxt->sources0set, source);
          CFArrayAppendValue (ctxt->sources0, source);
          CFArraySortValues (ctxt->sources0,
                             CFRangeMake(0, CFArrayGetCount (ctxt->sources0)),
                             Context0Comparator, NULL);
        }
    }
  else if (source->_context.version == 1)
    CFSetAddValue (ctxt->sources1, source);

  CFRunLoopWakeUp(rl);
}

static void
CFRunLoopAddObserver_nolock (CFRunLoopRef rl, CFRunLoopObserverRef obs,
                             CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  CFMutableSetRef observers;

  if (obs->_runloop != NULL && obs->_runloop != rl)
    return; // This observer is already added elsewhere

  obs->_runloop = rl;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  observers = ctxt->observers;
  CFSetAddValue (observers, obs);
}

static void
CFRunLoopAddTimer_nolock (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                          CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  CFRange range;

  if (timer->_runloop != NULL && timer->_runloop != rl)
    return; // This timer is already added elsewhere

  timer->_runloop = rl;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  /* Only add the timer if one doesn't exist */
  range = CFRangeMake (0, CFArrayGetCount (ctxt->timers));
  if (CFArrayContainsValue (ctxt->timers, range, timer) == false)
    CFArrayAppendValue (ctxt->timers, timer);

  // Wake up the runloop so that it can recalculate the next timer date
  CFRunLoopWakeUp(rl);
}



static Boolean
CFRunLoopContainsSource_nolock (CFRunLoopRef rl, CFRunLoopSourceRef source,
                         CFStringRef mode)
{
  Boolean ret = false;
  GSRunLoopContextRef ctxt;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  if (source->_context.version == 0)
    ret = CFSetContainsValue (ctxt->sources0set, source);
  else if (source->_context.version == 1)
    ret = CFSetContainsValue (ctxt->sources1, source);
  return ret;
}

static Boolean
CFRunLoopContainsObserver_nolock (CFRunLoopRef rl, CFRunLoopObserverRef obs,
                                  CFStringRef mode)
{
  Boolean ret;
  GSRunLoopContextRef ctxt;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  ret = CFSetContainsValue (ctxt->observers, obs);

  return ret;
}

static Boolean
CFRunLoopContainsTimer_nolock (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                               CFStringRef mode)
{
  Boolean ret;
  GSRunLoopContextRef ctxt;
  CFRange range;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  range = CFRangeMake (0, CFArrayGetCount (ctxt->timers));
  ret = CFArrayContainsValue (ctxt->timers, range, timer);
  return ret;
}



static void
CFRunLoopRemoveSource_nolock (CFRunLoopRef rl, CFRunLoopSourceRef source,
                              CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  if (source->_context.version == 0)
    {
      CFIndex idx = CFArrayGetFirstIndexOfValue(ctxt->sources0,
                                                CFRangeMake(0, CFArrayGetCount(ctxt->sources0)),
                                                source);
     
      if (idx != -1)
        {
          CFSetRemoveValue (ctxt->sources0set, source);
          CFArrayRemoveValueAtIndex(ctxt->sources0, idx);
        }
    }
  else if (source->_context.version == 1)
    CFSetRemoveValue (ctxt->sources1, source);
}

static void
CFRunLoopRemoveObserver_nolock (CFRunLoopRef rl, CFRunLoopObserverRef obs,
                                CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  CFSetRemoveValue (ctxt->observers, obs);
}

static void
CFRunLoopRemoveTimer_nolock (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                             CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  CFIndex idx;
  CFRange range;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  range = CFRangeMake (0, CFArrayGetCount (ctxt->timers));
  idx = CFArrayGetFirstIndexOfValue (ctxt->timers, range, timer);
  if (idx != kCFNotFound)
    CFArrayRemoveValueAtIndex (ctxt->timers, idx);
}

static void
CFRunLoopCommonModesAddFunc (const void *value, void *context)
{
  struct common_mode_info *info = (struct common_mode_info*)context;
  CFTypeID typeID = CFGetTypeID (info->obj);
  if (typeID == _kCFRunLoopSourceTypeID)
    CFRunLoopAddSource_nolock (info->rl, (CFRunLoopSourceRef)info->obj,
                               (CFStringRef)value);
  else if (typeID == _kCFRunLoopObserverTypeID)
    CFRunLoopAddObserver_nolock (info->rl, (CFRunLoopObserverRef)info->obj,
                                 (CFStringRef)value);
  else if (typeID == _kCFRunLoopTimerTypeID)
    CFRunLoopAddTimer_nolock (info->rl, (CFRunLoopTimerRef)info->obj,
                              (CFStringRef)value);
}

static void
CFRunLoopCommonModesAdd (CFRunLoopRef rl, CFTypeRef obj)
{
  struct common_mode_info info = { rl, obj, false };
  CFSetApplyFunction (rl->_commonModes, CFRunLoopCommonModesAddFunc, &info);
  CFArrayAppendValue (rl->_commonObjects, obj);
}

static void
CFRunLoopCommonModesContainFunc (const void *value, void *context)
{
  Boolean ret = false;
  struct common_mode_info *info = (struct common_mode_info*)context;
  CFTypeID typeID = CFGetTypeID (info->obj);
  if (typeID == _kCFRunLoopSourceTypeID)
    ret = CFRunLoopContainsSource_nolock (info->rl, (CFRunLoopSourceRef)info->obj,
                                          (CFStringRef)value);
  else if (typeID == _kCFRunLoopObserverTypeID)
    ret = CFRunLoopContainsObserver_nolock (info->rl, (CFRunLoopObserverRef)info->obj,
                                            (CFStringRef)value);
  else if (typeID == _kCFRunLoopTimerTypeID)
    ret = CFRunLoopContainsTimer_nolock (info->rl, (CFRunLoopTimerRef)info->obj,
                                         (CFStringRef)value);

  if (ret)
    {
      info->ret = ret;
    }
}

static Boolean
CFRunLoopCommonModesContain (CFRunLoopRef rl, CFTypeRef obj)
{
  CFRange range = CFRangeMake (0, CFArrayGetCount (rl->_commonObjects));
  if (CFArrayContainsValue (rl->_commonObjects, range, obj))
    {
      return true;
    }
  else
    {
      struct common_mode_info info = { rl, obj, false };
      CFSetApplyFunction (rl->_commonModes, CFRunLoopCommonModesContainFunc,
                          &info);
      return info.ret;
    }
}

static void
CFRunLoopCommonModesRemoveFunc (const void *value, void *context)
{
  struct common_mode_info *info = (struct common_mode_info*)context;
  CFTypeID typeID = CFGetTypeID (info->obj);
  if (typeID == _kCFRunLoopSourceTypeID)
    CFRunLoopRemoveSource (info->rl, (CFRunLoopSourceRef)info->obj,
                           (CFStringRef)value);
  else if (typeID == _kCFRunLoopObserverTypeID)
    CFRunLoopRemoveObserver (info->rl, (CFRunLoopObserverRef)info->obj,
                             (CFStringRef)value);
  else if (typeID == _kCFRunLoopTimerTypeID)
    CFRunLoopRemoveTimer (info->rl, (CFRunLoopTimerRef)info->obj,
                          (CFStringRef)value);
}

static void
CFRunLoopCommonModesRemove (CFRunLoopRef rl, CFTypeRef obj)
{
  CFRange range;
  CFIndex idx;
  struct common_mode_info info = { rl, obj, false };
  range = CFRangeMake (0, CFArrayGetCount (rl->_commonObjects));
  idx = CFArrayContainsValue (rl->_commonObjects, range, obj);
  if (idx != kCFNotFound)
    CFArrayRemoveValueAtIndex (rl->_commonObjects, idx);
  
  CFSetApplyFunction (rl->_commonModes, CFRunLoopCommonModesRemoveFunc, &info);
}



void
CFRunLoopAddSource (CFRunLoopRef rl, CFRunLoopSourceRef source,
                    CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    CFRunLoopCommonModesAdd (rl, source);
  else
    CFRunLoopAddSource_nolock (rl, source, mode);
  GSMutexUnlock (&rl->_lock);
  
  /* Call the schedule callback if it exists, but do it only once */
  if (source->_context.version == 0 && source->_context.schedule != NULL)
    source->_context.schedule (source->_context.info, rl, mode);
}

void
CFRunLoopAddObserver (CFRunLoopRef rl, CFRunLoopObserverRef observer,
                      CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    CFRunLoopCommonModesAdd (rl, observer);
  else
    CFRunLoopAddObserver_nolock (rl, observer, mode);
  GSMutexUnlock (&rl->_lock);
}

void
CFRunLoopAddTimer (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                   CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    CFRunLoopCommonModesAdd (rl, timer);
  else
    CFRunLoopAddTimer_nolock (rl, timer, mode);
  GSMutexUnlock (&rl->_lock);
}



Boolean
CFRunLoopContainsSource (CFRunLoopRef rl, CFRunLoopSourceRef source,
                         CFStringRef mode)
{
  Boolean ret = false;
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    ret = CFRunLoopCommonModesContain (rl, source);
  else
    ret = CFRunLoopContainsSource_nolock (rl, source, mode);
  GSMutexUnlock (&rl->_lock);
  return ret;
}

Boolean
CFRunLoopContainsObserver (CFRunLoopRef rl, CFRunLoopObserverRef observer,
                           CFStringRef mode)
{
  Boolean ret;
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    ret = CFRunLoopCommonModesContain (rl, observer);
  else
    ret = CFRunLoopContainsObserver_nolock (rl, observer, mode);
  GSMutexUnlock (&rl->_lock);
  return ret;
}

Boolean
CFRunLoopContainsTimer (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                        CFStringRef mode)
{
  Boolean ret;
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    ret = CFRunLoopCommonModesContain (rl, timer);
  else
    ret = CFRunLoopContainsTimer_nolock (rl, timer, mode);
  GSMutexUnlock (&rl->_lock);
  return ret;
}



void
CFRunLoopRemoveSource (CFRunLoopRef rl, CFRunLoopSourceRef source,
                       CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    CFRunLoopCommonModesRemove (rl, source);
  else
    CFRunLoopRemoveSource_nolock (rl, source, mode);
  GSMutexUnlock (&rl->_lock);
  /* Call the cancel callback if it exists, but do it only once */
  if (source->_context.version == 0 && source->_context.cancel != NULL)
    source->_context.cancel (source->_context.info, rl, mode);
}

void
CFRunLoopRemoveObserver (CFRunLoopRef rl, CFRunLoopObserverRef observer,
                         CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    CFRunLoopCommonModesRemove (rl, observer);
  else
    CFRunLoopRemoveObserver_nolock (rl, observer, mode);
  GSMutexUnlock (&rl->_lock);
}

void
CFRunLoopRemoveTimer (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                      CFStringRef mode)
{
  GSMutexLock (&rl->_lock);
  if (mode == kCFRunLoopCommonModes)
    CFRunLoopCommonModesRemove (rl, timer);
  else
    CFRunLoopRemoveTimer_nolock (rl, timer, mode);
  GSMutexUnlock (&rl->_lock);
}



#define CFRUNLOOPSOURCE_SIZE \
  sizeof(struct __CFRunLoopSource) - sizeof(CFRuntimeBase)

CFRunLoopSourceRef
CFRunLoopSourceCreate (CFAllocatorRef  alloc, CFIndex order,
                       CFRunLoopSourceContext *context)
{
  CFRunLoopSourceRef new;
  
  new = (CFRunLoopSourceRef)_CFRuntimeCreateInstance (alloc,
                                                      _kCFRunLoopSourceTypeID,
                                                      CFRUNLOOPSOURCE_SIZE,
                                                      0);
  if (new)
    {
      GSMutexInitialize (&(new->_lock));
      new->_isValid = true;
      new->_order = order;
      if (context)
        {
          switch (context->version)
            {
              case 0:
                new->_context = *context;
                if (context->retain)
                  new->_context.info = (void*)context->retain (context->info);
                break;
              case 1:
                new->_context1 = *((CFRunLoopSourceContext1*)context);
                if (new->_context1.retain)
                  new->_context1.info = (void*)new->_context1.retain (((CFRunLoopSourceContext1*)context)->info);
                break;
            }
        }
    }
  
  return new;
}

void
CFRunLoopSourceGetContext (CFRunLoopSourceRef source,
                           CFRunLoopSourceContext *context)
{
  if (source->_context.version == 0)
    *context = source->_context;
  else if (source->_context.version == 1)
    *((CFRunLoopSourceContext1*) context) = source->_context1;
}

CFIndex
CFRunLoopSourceGetOrder (CFRunLoopSourceRef source)
{
  return source->_order;
}

static void
CFRunLoopSourceRemoveInvalidated(const void *key, const void *value, void *source)
{
  GSRunLoopContextRef ctxt = (GSRunLoopContextRef) value;
  CFIndex idx = CFArrayGetFirstIndexOfValue(ctxt->sources0,
                                            CFRangeMake(0, CFArrayGetCount(ctxt->timers)),
                                            (CFRunLoopSourceRef) source);
  
  if (idx != -1)
    {
      CFArrayRemoveValueAtIndex(ctxt->sources0, idx);
      CFSetRemoveValue(ctxt->sources0set, source);
    }
}

void
CFRunLoopSourceInvalidate (CFRunLoopSourceRef source)
{
  source->_isValid = false;
  
  if (source->_runloop != NULL)
    {
      GSMutexLock(&source->_runloop->_lock);
      
      CFIndex idx = CFArrayGetFirstIndexOfValue(source->_runloop->_commonObjects,
                      CFRangeMake(0, CFArrayGetCount(source->_runloop->_commonObjects)),
                      source);
      
      if (idx != -1)
        CFArrayRemoveValueAtIndex(source->_runloop->_commonObjects, idx);
      
      CFDictionaryApplyFunction(source->_runloop->_contexts,
                                CFRunLoopSourceRemoveInvalidated, source);
      
      GSMutexUnlock(&source->_runloop->_lock);
    }
}

Boolean
CFRunLoopSourceIsValid (CFRunLoopSourceRef source)
{
  return source->_isValid;
}

void
CFRunLoopSourceSignal (CFRunLoopSourceRef source)
{
  source->_isSignaled = true;
  CFRunLoopWakeUp(source->_runloop);
}



#define CFRUNLOOPOBSERVER_SIZE \
  sizeof(struct __CFRunLoopObserver) - sizeof(CFRuntimeBase)

CFRunLoopObserverRef
CFRunLoopObserverCreate (CFAllocatorRef  alloc, CFOptionFlags activities,
                         Boolean repeats, CFIndex order,
                         CFRunLoopObserverCallBack callback,
                         CFRunLoopObserverContext *context)
{
  CFRunLoopObserverRef new;
  
  new = (CFRunLoopObserverRef)_CFRuntimeCreateInstance (alloc,
                                                        _kCFRunLoopObserverTypeID,
                                                        CFRUNLOOPOBSERVER_SIZE,
                                                        0);
  if (new)
    {
      GSMutexInitialize (&(new->_lock));
      new->_activities = activities;
      new->_repeats = repeats;
      new->_order = order;
      new->_callback = callback;
      if (context)
        {
          new->_context = *context;
          if (context->retain)
            new->_context.info = (void*)context->retain (context->info);
        }
    }
  
  return new;
}

Boolean
CFRunLoopObserverDoesRepeat (CFRunLoopObserverRef observer)
{
  return observer->_repeats;
}

CFOptionFlags
CFRunLoopObserverGetActivities (CFRunLoopObserverRef observer)
{
  return observer->_activities;
}

void
CFRunLoopObserverGetContext (CFRunLoopObserverRef observer,
                             CFRunLoopObserverContext *context)
{
  *context = observer->_context;
}

CFIndex
CFRunLoopObserverGetOrder (CFRunLoopObserverRef observer)
{
  return observer->_order;
}

static void
CFRunLoopObserverRemoveInvalidated(const void *key, const void *value, void *observer)
{
  GSRunLoopContextRef ctxt = (GSRunLoopContextRef) value;
  CFSetRemoveValue(ctxt->observers, observer);
}

void
CFRunLoopObserverInvalidate (CFRunLoopObserverRef observer)
{
  observer->_isValid = false;
  
  if (observer->_runloop != NULL)
    {
      GSMutexLock(&observer->_runloop->_lock);
      
      CFIndex idx = CFArrayGetFirstIndexOfValue(observer->_runloop->_commonObjects,
                      CFRangeMake(0, CFArrayGetCount(observer->_runloop->_commonObjects)),
                      observer);
      
      if (idx != -1)
        CFArrayRemoveValueAtIndex(observer->_runloop->_commonObjects, idx);
      
      CFDictionaryApplyFunction(observer->_runloop->_contexts,
                                CFRunLoopObserverRemoveInvalidated, observer);
      
      GSMutexUnlock(&observer->_runloop->_lock);
    }
}

Boolean
CFRunLoopObserverIsValid (CFRunLoopObserverRef observer)
{
  return observer->_isValid;
}



CFAbsoluteTime
CFRunLoopGetNextTimerFireDate (CFRunLoopRef rl, CFStringRef mode)
{
  CFAbsoluteTime rv = DISTANT_FUTURE;
  CFIndex i, count;
  GSRunLoopContextRef context = GSRunLoopContextGet(rl, mode);
  
  count = CFArrayGetCount(context->timers);
  // printf("Current time: %f\n", CFAbsoluteTimeGetCurrent());

  for (i = 0; i < count; i++)
    {
      CFRunLoopTimerRef timer = (CFRunLoopTimerRef) CFArrayGetValueAtIndex(context->timers, i);

      // printf("Timer %p valid:%d nextFireDate:%f\n", timer, timer->_isValid, timer->_nextFireDate);
      if (CFRunLoopTimerIsValid(timer))
        {
          CFAbsoluteTime t = CFRunLoopTimerGetNextFireDate(timer);
          if (t < rv)
            rv = t;
        }
    }

  return rv;
}

#define CFRUNLOOPTIMER_SIZE \
  sizeof(struct __CFRunLoopTimer) - sizeof(CFRuntimeBase)

CFRunLoopTimerRef
CFRunLoopTimerCreate (CFAllocatorRef  alloc, CFAbsoluteTime fireDate,
                      CFTimeInterval interval, CFOptionFlags flags,
                      CFIndex order, CFRunLoopTimerCallBack  callback,
                      CFRunLoopTimerContext *context)
{
  CFRunLoopTimerRef new;
  
  new = (CFRunLoopTimerRef)_CFRuntimeCreateInstance (alloc,
                                                     _kCFRunLoopTimerTypeID,
                                                     CFRUNLOOPTIMER_SIZE,
                                                     0);
  if (new)
    {
      GSMutexInitialize (&(new->_lock));
      new->_isValid = true;
      new->_nextFireDate = fireDate;
      new->_interval = interval;
      /* 'flags' is ignored */
      new->_order = order;
      new->_callback = callback;
      if (context)
        {
          new->_context = *context;
          if (context->retain)
            new->_context.info = (void*)context->retain (context->info);
        }
    }
  
  return new;
}

Boolean
CFRunLoopTimerDoesRepeat (CFRunLoopTimerRef timer)
{
  if (CF_IS_OBJC(_kCFRunLoopTimerTypeID, timer))
    {
      CFTimeInterval iv;

      CF_OBJC_CALLV(CFTimeInterval, iv, timer, "timeInterval");
      return iv > 0.0;
    }
  
  return (timer->_interval > 0.0);
}

void
CFRunLoopTimerGetContext (CFRunLoopTimerRef timer,
                          CFRunLoopTimerContext *context)
{
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCHV(_kCFRunLoopTimerTypeID, void, timer,
                        "_cfContext:", context);
  
  *context = timer->_context;
}

CFTimeInterval
CFRunLoopTimerGetInterval (CFRunLoopTimerRef timer)
{
  CF_OBJC_FUNCDISPATCHV(_kCFRunLoopTimerTypeID, CFTimeInterval, timer,
                        "timeInterval");
  
  return timer->_interval;
}

CFAbsoluteTime
CFRunLoopTimerGetNextFireDate (CFRunLoopTimerRef timer)
{
  if (CF_IS_OBJC(_kCFRunLoopTimerTypeID, timer))
    {
      CFDateRef date;

      CF_OBJC_CALLV(CFDateRef, date, timer, "fireDate");
      return CFDateGetAbsoluteTime(date);
    }
  
  return timer->_nextFireDate;
}

CFIndex
CFRunLoopTimerGetOrder (CFRunLoopTimerRef timer)
{
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCHV(_kCFRunLoopTimerTypeID, CFIndex, timer,
                        "_cfOrder");
  
  return timer->_order;
}

static void
CFRunLoopTimerRemoveInvalidated(const void *key, const void *value, void *timer)
{
  GSRunLoopContextRef ctxt = (GSRunLoopContextRef) value;
  CFIndex idx = CFArrayGetFirstIndexOfValue(ctxt->timers,
                                            CFRangeMake(0, CFArrayGetCount(ctxt->timers)),
                                            (CFRunLoopTimerRef) timer);
  
  if (idx != -1)
    CFArrayRemoveValueAtIndex(ctxt->timers, idx);
}

static void
CFRunLoopTimerRemoveFromRunLoop(CFRunLoopRef rl, CFRunLoopTimerRef timer)
{
    GSMutexLock(&rl->_lock);
      
    CFIndex idx = CFArrayGetFirstIndexOfValue(rl->_commonObjects,
                    CFRangeMake(0, CFArrayGetCount(rl->_commonObjects)),
                    timer);
      
    if (idx != -1)
      CFArrayRemoveValueAtIndex(rl->_commonObjects, idx);
      
    CFDictionaryApplyFunction(rl->_contexts,
                              CFRunLoopTimerRemoveInvalidated, timer);
      
    GSMutexUnlock(&rl->_lock);
}

void
CFRunLoopTimerInvalidate (CFRunLoopTimerRef timer)
{
  CF_OBJC_FUNCDISPATCHV(_kCFRunLoopTimerTypeID, void, timer,
                        "invalidate");
  
  timer->_isValid = false;
}

Boolean
CFRunLoopTimerIsValid (CFRunLoopTimerRef timer)
{
  CF_OBJC_FUNCDISPATCHV(_kCFRunLoopTimerTypeID, Boolean, timer,
                        "isValid");
  
  return timer->_isValid;
}

void
CFRunLoopTimerSetNextFireDate (CFRunLoopTimerRef timer,
                               CFAbsoluteTime fireDate)
{
  if (CF_IS_OBJC(_kCFRunLoopTimerTypeID, timer))
    {
      CFDateRef date = CFDateCreate(NULL, fireDate);
      CF_OBJC_VOIDCALLV(timer, "setFireDate:", date);

      return;
    }
  
  timer->_nextFireDate = fireDate;
  // Wake up the runloop so that it can recalculate the next timer date
  // but since timers should be planned on current runloop only (NSTimer
  // says so), this may not be necessary.
  CFRunLoopWakeUp(timer->_runloop);
}

