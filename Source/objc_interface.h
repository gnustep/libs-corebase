/* objc_interface.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: July, 2011
   
   This file is part of GNUstep CoreBase library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef __OBJC_INTERFACE_H__
#define __OBJC_INTERFACE_H__ 1

#include "CoreFoundation/CFRuntime.h"
#include <objc/runtime.h>



extern Class *__CFRuntimeObjCClassTable;
extern UInt32 __CFRuntimeClassTableCount;

static inline void
CFRuntimeBridgeClass (CFTypeID typeID, const char *cls)
{
  __CFRuntimeObjCClassTable[typeID] = (Class)objc_getClass (cls);
}

/* These functions are declared in CFInternal.h, but since corebase
   doesn't have this file, they'll be done in here. */
static inline void *
__CFISAForTypeID (CFTypeID typeID)
{
  return (void *)(typeID < __CFRuntimeClassTableCount ?
    __CFRuntimeObjCClassTable[typeID] : 0);
}

static inline Boolean
CF_IS_OBJC (CFTypeID typeID, const void *obj)
{
  return (typeID >= __CFRuntimeClassTableCount
          || object_getClass((id)obj) != __CFISAForTypeID (typeID));
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

#endif /* __OBJC_INTERFACE_H__ */
