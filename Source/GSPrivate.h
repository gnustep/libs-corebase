/* GSPrivate.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
   This file is part of GNUstep CoreBase library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#ifndef __GSPRIVATE_H__
#define __GSPRIVATE_H__

#include "CoreFoundation/CFBase.h"
#define BOOL OBJC_BOOL
#include <objc/runtime.h>
#undef BOOL



#if defined(_WIN32)

#include <windows.h>

#define GSMutex CRITICAL_SECTION
#define GSMutexInitialize(x) InitializeCriticalSection(x)
#define GSMutexLock(x) EnterCriticalSection(x)
#define GSMutexUnlock(x) LeaveCriticalSection(x)
#define GSMutexDestroy(x) DeleteCriticalSection(x)

#if defined(_WIN64)

#define GSAtomicIncrementCFIndex(ptr) \
  InterlockedIncrement64((LONGLONG volatile*)ptr)
#define GSAtomicDecrementCFIndex(ptr) \
  InterlockedDecrement64((LONGLONG volatile*)ptr)
  
#else /* _WIN64 */

#define GSAtomicIncrementCFIndex(ptr) InterlockedIncrement((LONG volatile*)ptr)
#define GSAtomicDecrementCFIndex(ptr) InterlockedDecrement((LONG volatile*)ptr)

#endif

#else /* _WIN32 */

#include <pthread.h>

#define GSMutex pthread_mutex_t
#define GSMutexInitialize(x) pthread_mutex_init(x, NULL)
#define GSMutexLock(x) pthread_mutex_lock(x)
#define GSMutexUnlock(x) pthread_mutex_unlock(x)
#define GSMutexDestroy(x) pthraed_mutex_destroy(x)

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <machine/atomic.h>
#define GSAtomicIncrementCFIndex(ptr) atomic_fetchadd_long ((u_long*)(ptr), 1)
#define GSAtomicDecrementCFIndex(ptr) atomic_fetchadd_long ((u_long*)(ptr), -1)
#elif defined(__llvm__) \
      || (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#define GSAtomicIncrementCFIndex(ptr) __sync_add_and_fetch((long*)(ptr), 1)
#define GSAtomicDecrementCFIndex(ptr) __sync_sub_and_fetch((long*)(ptr), 1)
#endif

#endif /* _WIN32 */



const void *
CFTypeRetainCallBack (CFAllocatorRef allocator, const void *value);

void
CFTypeReleaseCallBack (CFAllocatorRef alloc, const void *value);



extern Class *__CFRuntimeObjCClassTable;
extern UInt32 __CFRuntimeClassTableCount;

CF_INLINE void *
__CFISAForTypeID (CFTypeID typeID)
{
  return (void *)(typeID < __CFRuntimeClassTableCount ?
    __CFRuntimeObjCClassTable[typeID] : NULL);
}

CF_INLINE Boolean
CF_IS_OBJC (CFTypeID typeID, const void *obj)
{
  return (obj && (typeID >= __CFRuntimeClassTableCount
          || object_getClass((id)obj) != __CFISAForTypeID (typeID)));
}

#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel) do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return imp((id)obj, s); \
    } \
  } while(0)

#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1) do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return imp((id)obj, s, a1); \
    } \
  } while(0)

#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2) do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return (rettype)imp((id)obj, s, a1, a2); \
    } \
  } while(0)

#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3) do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return (rettype)imp((id)obj, s, a1, a2, a3); \
    } \
  } while(0)

#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4) do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return (rettype)imp((id)obj, s, a1, a2, a3, a4); \
    } \
  } while(0)

#define CF_OBJC_FUNCDISPATCH5(typeID, rettype, obj, sel, a1, a2, a3, a4, a5) \
do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return (rettype)imp((id)obj, s, a1, a2, a3, a4, a5); \
    } \
  } while(0)

#endif /* __GSPRIVATE_H__ */