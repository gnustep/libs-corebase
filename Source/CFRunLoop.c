/* CFRunLoop.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Author: Stefan Bidigaray <stefanbidi@gmail.com>
   Date: August, 2012
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFSet.h"
#include "GSPrivate.h"

/* From NSDate.m in GNUstep-base */
#define DISTANT_FUTURE	63113990400.0

static CFTypeID _kCFRunLoopTypeID = 0;
static CFTypeID _kCFRunLoopSourceTypeID = 0;
static CFTypeID _kCFRunLoopObserverTypeID = 0;
static CFTypeID _kCFRunLoopTimerTypeID = 0;

CONST_STRING_DECL(kCFRunLoopDefaultMode, "kCFRunLoopDefaultMode");
CONST_STRING_DECL(kCFRunLoopCommonModes, "kCFRunLoopCommonModes");

static CFRuntimeClass CFRunLoopClass =
{
  0,
  "CFRunLoop",
  NULL,
  NULL,
  NULL,
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
  NULL,
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
  NULL,
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
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

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



struct GSRunLoopContext
{
  CFStringRef mode;
  CFMutableArrayRef timers;
  CFMutableSetRef observers;
  CFMutableSetRef sources0;
  CFMutableSetRef sources1; /* This is only a place holder for now. */
};

struct __CFRunLoop
{
  CFRuntimeBase     parent;
  CFStringRef       _mode;
  CFMutableArrayRef _commonModes;
  CFMutableSetRef   _contexts;
};

CFRunLoopRef
CFRunLoopGetCurrent (void)
{
  return NULL;
}

CFRunLoopRef
CFRunLoopGetMain (void)
{
  return NULL;
}

void
CFRunLoopRun (void)
{
  SInt32 code;
  do
    {
      code = CFRunLoopRunInMode (kCFRunLoopDefaultMode, DISTANT_FUTURE, false);
    } while (code != kCFRunLoopRunFinished || code != kCFRunLoopRunStopped);
}

SInt32
CFRunLoopRunInMode (CFStringRef mode, CFTimeInterval seconds,
                    Boolean returnAfterSourceHandled)
{
  return 0;
}

void
CFRunLoopWakeUp (CFRunLoopRef rl)
{
  return;
}

void
CFRunLoopStop (CFRunLoopRef rl)
{
  return;
}

Boolean
CFRunLoopIsWaiting (CFRunLoopRef rl)
{
  return false;
}

void
CFRunLoopAddCommonMode (CFRunLoopRef rl, CFStringRef mode)
{
  return;
}

CFArrayRef
CFRunLoopCopyAllModes (CFRunLoopRef rl)
{
  return NULL;
}

CFStringRef
CFRunLoopCopyCurrentMode (CFRunLoopRef rl)
{
  return NULL;
}

#if __BLOCKS__
void
CFRunLoopPerformBlock (CFRunLoopRef rl, CFTypeRef mode, void (^block)(void))
{
  return;
}
#endif



struct __CFRunLoopSource
{
  CFRuntimeBase parent;
  GSMutex       _lock;
  CFIndex       _order;
  Boolean       _isSignaled;
  Boolean       _isValid;
  CFRunLoopSourceContext _context; /* FIXME: Handle version 1 contexts */
};

void
CFRunLoopAddSource (CFRunLoopRef rl, CFRunLoopSourceRef source,
                    CFStringRef mode)
{
  return;
}

Boolean
CFRunLoopContainsSource (CFRunLoopRef rl, CFRunLoopSourceRef source,
                         CFStringRef mode)
{
  return false;
}

void
CFRunLoopRemoveSource (CFRunLoopRef rl, CFRunLoopSourceRef source,
                       CFStringRef mode)
{
  return;
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
                /* FIXME */
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
  *context = source->_context;
}

CFIndex
CFRunLoopSourceGetOrder (CFRunLoopSourceRef source)
{
  return source->_order;
}

void
CFRunLoopSourceInvalidate (CFRunLoopSourceRef source)
{
  return;
}

Boolean
CFRunLoopSourceIsValid (CFRunLoopSourceRef source)
{
  return source->_isValid;
}

void
CFRunLoopSourceSignal (CFRunLoopSourceRef source)
{
  return;
}



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
};

void
CFRunLoopAddObserver (CFRunLoopRef rl, CFRunLoopObserverRef observer,
                      CFStringRef mode)
{
  return;
}

Boolean
CFRunLoopContainsObserver (CFRunLoopRef rl, CFRunLoopObserverRef observer,
                           CFStringRef mode)
{
  return false;
}

void
CFRunLoopRemoveObserver (CFRunLoopRef rl, CFRunLoopObserverRef observer,
                         CFStringRef mode)
{
  return;
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

void
CFRunLoopObserverInvalidate (CFRunLoopObserverRef observer)
{
  return;
}

Boolean
CFRunLoopObserverIsValid (CFRunLoopObserverRef observer)
{
  return observer->_isValid;
}



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
};

void
CFRunLoopAddTimer (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                   CFStringRef mode)
{
  return;
}

CFAbsoluteTime
CFRunLoopGetNextTimerFireDate (CFRunLoopRef rl, CFStringRef mode)
{
  return 0.0;
}

void
CFRunLoopRemoveTimer (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                      CFStringRef mode)
{
  return;
}

Boolean
CFRunLoopContainsTimer (CFRunLoopRef rl, CFRunLoopTimerRef timer,
                        CFStringRef mode)
{
  return false;
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
      new->_nextFireDate = fireDate;
      new->_interval = interval;
      // flags is ignored
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
  return (timer->_interval > 0.0);
}

void
CFRunLoopTimerGetContext (CFRunLoopTimerRef timer,
                          CFRunLoopTimerContext *context)
{
  *context = timer->_context;
}

CFTimeInterval
CFRunLoopTimerGetInterval (CFRunLoopTimerRef timer)
{
  return timer->_interval;
}

CFAbsoluteTime
CFRunLoopTimerGetNextFireDate (CFRunLoopTimerRef timer)
{
  return timer->_nextFireDate;
}

CFIndex
CFRunLoopTimerGetOrder (CFRunLoopTimerRef timer)
{
  return timer->_order;
}

void
CFRunLoopTimerInvalidate (CFRunLoopTimerRef timer)
{
  return;
}

Boolean
CFRunLoopTimerIsValid (CFRunLoopTimerRef timer)
{
  return timer->_isValid;
}

void
CFRunLoopTimerSetNextFireDate (CFRunLoopTimerRef timer,
                               CFAbsoluteTime fireDate)
{
  return;
}

