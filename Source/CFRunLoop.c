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
#include "CoreFoundation/CFDictionary.h"
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

typedef struct GSRunLoopContext * GSRunLoopContextRef;
struct GSRunLoopContext
{
  CFRunLoopActivity activities;
  CFMutableArrayRef timers;
  CFMutableSetRef observers;
  CFMutableSetRef sources0;
  CFMutableSetRef sources1; /* This is only a place holder for now. */
};

struct __CFRunLoop
{
  CFRuntimeBase     parent;
  GSMutex           _lock;
  CFStringRef       _currentMode;
  CFMutableSetRef   _commonModes;
  CFMutableArrayRef _commonObjects;
  CFMutableDictionaryRef _contexts;
};

struct __CFRunLoopSource
{
  CFRuntimeBase parent;
  GSMutex       _lock;
  CFIndex       _order;
  Boolean       _isSignaled;
  Boolean       _isValid;
  CFRunLoopSourceContext _context; /* FIXME: Handle version 1 contexts */
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
};

static GSRunLoopContextRef
GSRunLoopContextNew (CFAllocatorRef alloc);

static void
GSRunLoopContextFinalize (GSRunLoopContextRef ctxt);

static void
GSRunLoopContextDeallocFunc (const void *key, const void *value, void *ctxt);

static void
CFRunLoopFinalize (CFTypeRef cf)
{
  CFRunLoopRef rl = (CFRunLoopRef)cf;
  CFAllocatorRef alloc = CFGetAllocator (rl);
  CFRelease (rl->_currentMode);
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
      new->sources0 = CFSetCreateMutable (alloc, 0, &kCFTypeSetCallBacks);
      new->sources1 = CFSetCreateMutable (alloc, 0, &kCFTypeSetCallBacks);
    }
  
  return new;
}

static void
GSRunLoopContextFinalize (GSRunLoopContextRef ctxt)
{
  CFRelease (ctxt->timers);
  CFRelease (ctxt->observers);
  CFRelease (ctxt->sources0);
  CFRelease (ctxt->sources1);
}

static void
GSRunLoopContextDeallocFunc (const void *key, const void *value, void *ctxt)
{
  GSRunLoopContextFinalize ((GSRunLoopContextRef)value);
  CFAllocatorDeallocate ((CFAllocatorRef)ctxt, (void*)value);
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
        CFSetAddValue (ctxt->sources0, value);
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
   *   - Process timers with fire dates in the pass.
   *   - If an input source fired, deliver the event.
   *   - If CFRunLoopWakeUp() was called but the run loop has not timed out,
   *     restart the loop. Go to step 2.
   * 10. Notify observers with kCFRunLoopExit activity.
   */
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
  return CFRetain (rl->_currentMode);
}

#if __BLOCKS__
void
CFRunLoopPerformBlock (CFRunLoopRef rl, CFTypeRef mode, void (^block)(void))
{
  return;
}
#endif



/* The next few function are used to add objects to the common modes
 */
struct common_mode_info
{
  CFRunLoopRef rl;
  CFTypeRef obj;
};

/* Call the _nolock variant of the Add, Remove and Contain functions
 * when the run loop is already locked.
 */
static void
CFRunLoopAddSource_nolock (CFRunLoopRef rl, CFRunLoopSourceRef source,
                           CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  CFMutableSetRef sources;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  if (source->_context.version == 0)
    sources = ctxt->sources0;
  else if (source->_context.version == 1)
    sources = ctxt->sources1;
  CFSetAddValue (sources, source);
}

static void
CFRunLoopAddObserver_nolock (CFRunLoopRef rl, CFRunLoopObserverRef obs,
                             CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  CFMutableSetRef observers;
  
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
  
  ctxt = GSRunLoopContextGet (rl, mode);
  /* Only add the timer if one doesn't exist */
  range = CFRangeMake (0, CFArrayGetCount (ctxt->timers));
  if (CFArrayContainsValue (ctxt->timers, range, timer) == false)
    CFArrayAppendValue (ctxt->timers, timer);
}



static Boolean
CFRunLoopContainsSource_nolock (CFRunLoopRef rl, CFRunLoopSourceRef source,
                         CFStringRef mode)
{
  Boolean ret = false;
  GSRunLoopContextRef ctxt;
  
  ctxt = GSRunLoopContextGet (rl, mode);
  if (source->_context.version == 0)
    ret = CFSetContainsValue (ctxt->sources0, source);
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
  
  GSMutexLock (&rl->_lock);
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
  
  GSMutexLock (&rl->_lock);
  ctxt = GSRunLoopContextGet (rl, mode);
  if (source->_context.version == 0)
    CFSetRemoveValue (ctxt->sources0, source);
  else if (source->_context.version == 1)
    CFSetRemoveValue (ctxt->sources1, source);
}

static void
CFRunLoopRemoveObserver_nolock (CFRunLoopRef rl, CFRunLoopObserverRef obs,
                                CFStringRef mode)
{
  GSRunLoopContextRef ctxt;
  
  GSMutexLock (&rl->_lock);
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
  struct common_mode_info info = { rl, obj };
  CFSetApplyFunction (rl->_commonModes, CFRunLoopCommonModesAddFunc, &info);
  CFArrayAppendValue (rl->_commonObjects, obj);
}

static void
CFRunLoopCommonModesContainFunc (const void *value, void *context)
{
  struct common_mode_info *info = (struct common_mode_info*)context;
  CFTypeID typeID = CFGetTypeID (info->obj);
  if (typeID == _kCFRunLoopSourceTypeID)
    CFRunLoopContainsSource_nolock (info->rl, (CFRunLoopSourceRef)info->obj,
                                    (CFStringRef)value);
  else if (typeID == _kCFRunLoopObserverTypeID)
    CFRunLoopContainsObserver_nolock (info->rl, (CFRunLoopObserverRef)info->obj,
                                      (CFStringRef)value);
  else if (typeID == _kCFRunLoopTimerTypeID)
    CFRunLoopContainsTimer_nolock (info->rl, (CFRunLoopTimerRef)info->obj,
                                   (CFStringRef)value);
}

static Boolean
CFRunLoopCommonModesContain (CFRunLoopRef rl, CFTypeRef obj)
{
  Boolean ret;
  CFRange range = CFRangeMake (0, CFArrayGetCount (rl->_commonObjects));
  if (CFArrayContainsValue (rl->_commonObjects, range, obj))
    {
      ret = true;
    }
  else
    {
      struct common_mode_info info = { rl, obj };
      CFSetApplyFunction (rl->_commonModes, CFRunLoopCommonModesContainFunc,
                          &info);
    }
  
  return false;
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
  struct common_mode_info info = { rl, obj };
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



CFAbsoluteTime
CFRunLoopGetNextTimerFireDate (CFRunLoopRef rl, CFStringRef mode)
{
  return 0.0;
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
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCH0(_kCFRunLoopTimerTypeID, Boolean, timer,
                        "_cfDoesRepeat");
  
  return (timer->_interval > 0.0);
}

void
CFRunLoopTimerGetContext (CFRunLoopTimerRef timer,
                          CFRunLoopTimerContext *context)
{
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCH1(_kCFRunLoopTimerTypeID, void, timer,
                        "_cfContext:", context);
  
  *context = timer->_context;
}

CFTimeInterval
CFRunLoopTimerGetInterval (CFRunLoopTimerRef timer)
{
  CF_OBJC_FUNCDISPATCH0(_kCFRunLoopTimerTypeID, CFTimeInterval, timer,
                        "timeInterval");
  
  return timer->_interval;
}

CFAbsoluteTime
CFRunLoopTimerGetNextFireDate (CFRunLoopTimerRef timer)
{
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCH0(_kCFRunLoopTimerTypeID, CFAbsoluteTime, timer,
                        "_cfFireDate");
  
  return timer->_nextFireDate;
}

CFIndex
CFRunLoopTimerGetOrder (CFRunLoopTimerRef timer)
{
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCH0(_kCFRunLoopTimerTypeID, CFIndex, timer,
                        "_cfOrder");
  
  return timer->_order;
}

void
CFRunLoopTimerInvalidate (CFRunLoopTimerRef timer)
{
  CF_OBJC_FUNCDISPATCH0(_kCFRunLoopTimerTypeID, void, timer,
                        "invalidate");
  
  
}

Boolean
CFRunLoopTimerIsValid (CFRunLoopTimerRef timer)
{
  CF_OBJC_FUNCDISPATCH0(_kCFRunLoopTimerTypeID, Boolean, timer,
                        "isValid");
  
  return timer->_isValid;
}

void
CFRunLoopTimerSetNextFireDate (CFRunLoopTimerRef timer,
                               CFAbsoluteTime fireDate)
{
  /* FIXME: need this method in NSTimer. */
  CF_OBJC_FUNCDISPATCH1(_kCFRunLoopTimerTypeID, void, timer,
                        "_cfSetNextFireDate:", fireDate);
  
  return;
}

