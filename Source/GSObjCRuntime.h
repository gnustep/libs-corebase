/* GSObjCRuntime.h
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: July, 2013
   
   This file is part of GNUstep CoreBase library.
   
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

#ifndef __GSOBJCRUNTIME_H__
#define __GSOBJCRUNTIME_H__

#include "config.h"
#include "CoreFoundation/CFBase.h"

GS_PRIVATE const void *CFTypeRetainCallBack (CFAllocatorRef alloc,
                                             const void *value);

GS_PRIVATE void CFTypeReleaseCallBack (CFAllocatorRef alloc, const void *value);

#if HAVE_LIBOBJC || HAVE_LIBOBJC2

#define BOOL OBJC_BOOL
#include <objc/runtime.h>
#undef BOOL

extern void **__CFRuntimeObjCClassTable;
extern UInt32 __CFRuntimeClassTableCount;

CF_INLINE void *
__CFISAForTypeID (CFTypeID typeID)
{
  return (__CFRuntimeObjCClassTable && typeID < __CFRuntimeClassTableCount ?
          __CFRuntimeObjCClassTable[typeID] : NULL);
}

CF_INLINE Boolean
CF_IS_OBJC (CFTypeID typeID, const void *obj)
{
#if defined(OBJC_SMALL_OBJECT_MASK)
  return (obj && (((unsigned long)obj & OBJC_SMALL_OBJECT_MASK) != 0
                  || typeID >= __CFRuntimeClassTableCount
                  || object_getClass ((id) obj) != __CFISAForTypeID (typeID)));
#else
  return (obj && (typeID >= __CFRuntimeClassTableCount
                  || object_getClass ((id) obj) != __CFISAForTypeID (typeID)));
#endif
}

#define CF_OBJC_FUNCDISPATCHV(typeID, rettype, obj, sel, ...) \
do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      return (rettype)imp((id)obj, s, ##__VA_ARGS__); \
    } \
  } while(0)

#define CF_OBJC_FUNCDISPATCHV_RETAINED(typeID, rettype, obj, sel, ...) \
do { \
  if (CF_IS_OBJC(typeID, obj)) \
    { \
      rettype (*imp)(id, SEL, ...); \
      static SEL s = NULL; \
      if (!s) \
        s = sel_registerName(sel); \
      imp = (rettype (*)(id, SEL, ...)) \
        class_getMethodImplementation (object_getClass((id)obj), s); \
      rettype val = imp((id)obj, s, ##__VA_ARGS__); \
      if (val != NULL) CFRetain(val); \
      return val; \
    } \
  } while(0)

#define CF_OBJC_CALLV(rettype, var, obj, sel, ...) \
do { \
  rettype (*imp)(id, SEL, ...); \
  static SEL s = NULL; \
  if (!s) \
    s = sel_registerName(sel); \
  imp = (rettype (*)(id, SEL, ...)) \
    class_getMethodImplementation (object_getClass((id)obj), s); \
  var = imp((id)obj, s, ##__VA_ARGS__); \
} while (0)

#define CF_OBJC_VOIDCALLV(obj, sel, ...) \
do { \
  void (*imp)(id, SEL, ...); \
  static SEL s = NULL; \
  if (!s) \
    s = sel_registerName(sel); \
  imp = (void (*)(id, SEL, ...)) \
    class_getMethodImplementation (object_getClass((id)obj), s); \
  imp((id)obj, s, ##__VA_ARGS__); \
} while (0)

#else

#define __CFISAForTypeID(typeID) (NULL)
#define CF_IS_OBJC(typeID, obj) (0)

#define CF_OBJC_FUNCDISPATCHV(typeID, rettype, obj, sel, ...)
#define CF_OBJC_FUNCDISPATCHV_RETAINED(typeID, rettype, obj, sel, ...)
#define CF_OBJC_CALLV(rettype, var, obj, sel, ...)
#define CF_OBJC_VOIDCALLV(obj, sel, ...)

#endif /* HAVE_LIBOBJC */

#endif /* __GSOBJCRUNTIME_H__ */
