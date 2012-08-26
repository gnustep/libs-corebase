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

static CFTypeID _kCFRunLoopTypeID = 0;
static CFTypeID _kCFRunLoopSourceTypeID = 0;
static CFTypeID _kCFRunLoopObserverTypeID = 0;
static CFTypeID _kCFRunLoopTimeTypeID = 0;

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
  _kCFRunLoopTimeTypeID = _CFRuntimeRegisterClass (&CFRunLoopTimerClass);
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
  return _kCFRunLoopTimeTypeID;
}

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
  return;
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

CFRunLoopSourceRef
CFRunLoopSourceCreate (CFAllocatorRef  alloc, CFIndex order,
                       CFRunLoopSourceContext *context)
{
  return NULL;
}

void
CFRunLoopSourceGetContext (CFRunLoopSourceRef source,
                           CFRunLoopSourceContext *context)
{
  return;
}

CFIndex
CFRunLoopSourceGetOrder (CFRunLoopSourceRef source)
{
  return 0;
}

void
CFRunLoopSourceInvalidate (CFRunLoopSourceRef source)
{
  return;
}

Boolean
CFRunLoopSourceIsValid (CFRunLoopSourceRef source)
{
  return false;
}

void
CFRunLoopSourceSignal (CFRunLoopSourceRef source)
{
  return;
}

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

CFRunLoopObserverRef
CFRunLoopObserverCreate (CFAllocatorRef  alloc, CFOptionFlags activities,
                         Boolean repeats, CFIndex order,
                         CFRunLoopObserverCallBack  callback,
                         CFRunLoopObserverContext *context)
{
  return NULL;
}

Boolean
CFRunLoopObserverDoesRepeat (CFRunLoopObserverRef observer)
{
  return false;
}

CFOptionFlags
CFRunLoopObserverGetActivities (CFRunLoopObserverRef observer)
{
  return 0;
}

void
CFRunLoopObserverGetContext (CFRunLoopObserverRef observer,
                             CFRunLoopObserverContext *context)
{
  return;
}

CFIndex
CFRunLoopObserverGetOrder (CFRunLoopObserverRef observer)
{
  return 0;
}

void
CFRunLoopObserverInvalidate (CFRunLoopObserverRef observer)
{
  return;
}

Boolean
CFRunLoopObserverIsValid (CFRunLoopObserverRef observer)
{
  return false;
}

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

CFRunLoopTimerRef
CFRunLoopTimerCreate (CFAllocatorRef  alloc, CFAbsoluteTime fireDate,
                      CFTimeInterval interval, CFOptionFlags flags,
                      CFIndex order, CFRunLoopTimerCallBack  callback,
                      CFRunLoopTimerContext *context)
{
  return NULL;
}

Boolean
CFRunLoopTimerDoesRepeat (CFRunLoopTimerRef timer)
{
  return false;
}

void
CFRunLoopTimerGetContext (CFRunLoopTimerRef timer,
                          CFRunLoopTimerContext *context)
{
  return;
}

CFTimeInterval
CFRunLoopTimerGetInterval (CFRunLoopTimerRef timer)
{
  return 0.0;
}

CFAbsoluteTime
CFRunLoopTimerGetNextFireDate (CFRunLoopTimerRef timer)
{
  return 0.0;
}

CFIndex
CFRunLoopTimerGetOrder (CFRunLoopTimerRef timer)
{
  return 0;
}

void
CFRunLoopTimerInvalidate (CFRunLoopTimerRef timer)
{
  return;
}

Boolean
CFRunLoopTimerIsValid (CFRunLoopTimerRef timer)
{
  return false;
}

void
CFRunLoopTimerSetNextFireDate (CFRunLoopTimerRef timer,
                                CFAbsoluteTime fireDate)
{
  return;
}

