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
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"



#define UDATE_TO_ABSOLUTETIME(d) \
  (((d) / 1000.0) - kCFAbsoluteTimeIntervalSince1970)
#define ABSOLUTETIME_TO_UDATE(at) \
  (((at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0)

#if defined(_WIN32)

#include <windows.h>

#define GSMutex CRITICAL_SECTION
#define GSMutexInitialize(x) InitializeCriticalSection(x)
#define GSMutexLock(x) EnterCriticalSection(x)
#define GSMutexUnlock(x) LeaveCriticalSection(x)
#define GSMutexDestroy(x) DeleteCriticalSection(x)

#if defined(_WIN64)
#define GSAtomicIncrementCFIndex(ptr) \
  InterlockedIncrement64((LONGLONG volatile*)(ptr))
#define GSAtomicDecrementCFIndex(ptr) \
  InterlockedDecrement64((LONGLONG volatile*)(ptr))
#define GSAtomicCompareAndSwapCFIndex(ptr, oldv, newv) \
  InterlockedCompareExchange64((LONGLONG volatile*)(ptr), (newv), (oldv))
#else
#define GSAtomicIncrementCFIndex(ptr) \
  InterlockedIncrement((LONG volatile*)(ptr))
#define GSAtomicDecrementCFIndex(ptr) \
  InterlockedDecrement((LONG volatile*)(ptr))
#define GSAtomicCompareAndSwapCFIndex(ptr, oldv, newv) \
  InterlockedCompareExchange((LONG volatile*)(ptr), (newv), (oldv))
#endif /* _WIN64 */


#define GSAtomicCompareAndSwapPointer(ptr, oldv, newv) \
  InterlockedCompareExchangePointer((ptr), (newv), (oldv))

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
#define GSAtomicIncrementCFIndex(ptr) atomic_fetchadd_long ((ptr), 1)
#define GSAtomicDecrementCFIndex(ptr) atomic_fetchadd_long ((ptr), -1)
#define GSAtomicCompareAndSwapCFIndex(ptr, oldv, newv) \
  atomic_cmpset_long((ptr), (oldv), (newv))
#define GSAtomicCompareAndSwapPointer(ptr, oldv, newv) \
  atomic_cmpset_ptr((ptr), (oldv), (newv))

#elif defined(__llvm__) \
      || (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))

#define GSAtomicIncrementCFIndex(ptr) __sync_add_and_fetch((long*)(ptr), 1)
#define GSAtomicDecrementCFIndex(ptr) __sync_sub_and_fetch((long*)(ptr), 1)
#define GSAtomicCompareAndSwapCFIndex(ptr, oldv, newv) \
  __sync_val_compare_and_swap((long*)(ptr), (long)(oldv), (long)(newv))
#define GSAtomicCompareAndSwapPointer(ptr, oldv, newv) \
  __sync_val_compare_and_swap((void**)(ptr), (void*)(oldv), (void*)(newv))

#endif

#endif /* _WIN32 */



CFIndex
GSBSearch (const void *array, const void *key, CFRange range, CFIndex size,
  CFComparatorFunction comp, void *ctxt);

/* All three of these function are taken from Thomas Wang's website
 * (http://www.concentric.net/~Ttwang/tech/inthash.htm).  As far as I know,
 * these algorithm are in the public domain.
 */
CF_INLINE UInt64
GSHashInt64 (UInt64 value)
{
  register UInt64 hash = value;
  hash = (~hash) + (hash << 21);
  hash = hash ^ (hash >> 24);
  hash = (hash + (hash << 3)) + (hash << 8);
  hash = hash ^ (hash >> 14);
  hash = (hash + (hash << 2)) + (hash << 4);
  hash = hash ^ (hash >> 28);
  hash = hash + (hash << 31);
  return hash;
}

CF_INLINE UInt32
GSHashInt32 (UInt32 value)
{
  register UInt32 hash = value;
  hash = ~hash + (hash << 15);
  hash = hash ^ (hash >> 12);
  hash = hash + (hash << 2);
  hash = hash ^ (hash >> 4);
  hash = (hash + (hash << 3)) + (hash << 11);
  hash = hash ^ (hash >> 16);
  return hash;
}

CF_INLINE UInt32
GSHashInt64ToInt32 (UInt64 value)
{
  register UInt64 hash = value;
  hash = (~hash) + (hash << 18);
  hash = hash ^ (hash >> 31);
  hash = (hash + (hash << 2)) + (hash << 4);
  hash = hash ^ (hash >> 11);
  hash = hash + (hash << 6);
  hash = hash ^ (hash >> 22);
  return (UInt32)hash;
}

CF_INLINE CFHashCode
GSHashPointer (const void *value)
{
#if defined(__LP64__) || defined(_WIN64)
/* 64-bit operating systems */
  return (CFHashCode)GSHashInt64 ((UInt64)value);
#else
/* 32-bit operating systems */
  return (CFHashCode)GSHashInt32 ((UInt32)value);
#endif
}

CF_INLINE CFHashCode
GSHashBytes (const void *bytes, CFIndex length)
{
  CFHashCode ret = 0;
  if (length > 0)
    {
      register CFIndex idx;
      register const char *p = bytes;
      
      for (idx = 0 ; idx < length ; ++idx)
        ret = (ret << 5) + ret + p[idx];
      
      ret &= 0x0fffffff;
      if (ret == 0)
        ret = 0x0fffffff;
    }
  else
    {
      ret = 0x0ffffffe;
    }
  
  return ret;
}



struct __CFConstantString
{
  CFRuntimeBase  _parent;
  void          *_contents;
  CFIndex        _count;
  CFHashCode     _hash;
  CFAllocatorRef _deallocator;
};

#if defined (_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define CONST_STRING_DECL(var, str) \
  static struct __CFConstantString __ ## var ## __ = \
    { {0, 0, {1, 0, 0}}, (void*)str, sizeof(str) - 1, 0, NULL }; \
  DLL_EXPORT const CFStringRef var = (CFStringRef) & __ ## var ## __;

#define CHAR_IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define CHAR_IS_HEX(c) (CHAR_IS_DIGIT(c) \
  || ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
#define CHAR_IS_ASCII(c) ((c) < 128)
#define CHAR_IS_UPPER_CASE(c) ((c) >= 'A' && (c) <= 'Z')
#define CHAR_IS_LOWER_CASE(c) ((c) >= 'a' && (c) <= 'z')
#define CHAR_IS_ALPHA(c) (CHAR_IS_UPPER_CASE(c) || CHAR_IS_LOWER_CASE(c))


CF_INLINE Boolean
__CFStringEncodingIsSupersetOfASCII (CFStringEncoding encoding)
{
  if (encoding == kCFStringEncodingASCII)
    return true;
  
  switch (encoding & 0xF00)
    {
      case 0x100:
        return encoding == kCFStringEncodingUTF8 ? true : false;
      case 0x000: /* MacOS codepage */
        if (encoding == kCFStringEncodingMacJapanese
            || encoding == kCFStringEncodingMacArabic
            || encoding == kCFStringEncodingMacHebrew
            || encoding == kCFStringEncodingMacUkrainian
            || encoding == kCFStringEncodingMacSymbol
            || encoding == kCFStringEncodingMacDingbats)
          return false;
      case 0x200: /* ISO-8859-* */
      case 0x400: /* DOS codepage */
      case 0x500: /* Windows codepage */
        return true;
    }
  return false;
}

CF_INLINE CFStringEncoding
GSStringGetFileSystemEncoding (void)
{
#if defined(_WIN32)
  return kCFStringEncodingUTF16;
#else
  return CFStringGetSystemEncoding ();
#endif
}

#if __BIG_ENDIAN__
#define UTF16_ENCODING kCFStringEncodingUTF16BE
#define UTF16_BOM_HI 0xFE
#define UTF16_BOM_LO 0xFF
#else
#define UTF16_ENCODING kCFStringEncodingUTF16LE
#define UTF16_BOM_HI 0xFF
#define UTF16_BOM_LO 0xFE
#endif
#define UTF16_BOM 0xFEFF

/* This function converts a Unicode string to a specified encoding.
 * It returns the number of bytes consumed.
 * If you need to know how many characters were consumed, you must compare
 * the value of *src before the function call to the one after the function
 * has returned.
 * WARNING: Never tell this function to convert to kCFStringEncodingUTF16
 * because it will add a BOM even if one already exists, regardless of
 * isExternalRepresentation.
 */
CFIndex
GSStringEncodingFromUnicode (CFStringEncoding encoding, char *dst,
  CFIndex dstLength, const UniChar **src, CFIndex srcLength, char lossByte,
  Boolean isExternalRepresentation, CFIndex *bytesNeeded);

/* This function converts a Unicode string to a specified encoding.
 * It return the number of characters converted.
 * If you need to know how many bytes were consumed, you must compare
 * the value of *src before the function call to the one after the function
 * has returned.
 * WARNING: Never tell this function to convert from kCFStringEncodingUTF16
 * because it will add a BOM.
 */
CFIndex
GSStringEncodingToUnicode (CFStringEncoding encoding, UniChar *dst,
  CFIndex dstLength, const char **src, CFIndex srcLength,
  Boolean isExternalRepresentation, CFIndex *bytesNeeded);

void
_CFStringAppendFormatAndArgumentsAux (CFMutableStringRef outputString,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFDictionaryRef formatOptions, CFStringRef formatString, va_list args);

CFStringRef
_CFStringCreateWithFormatAndArgumentsAux (CFAllocatorRef alloc,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFDictionaryRef formatOptions, CFStringRef formatString, va_list args);

const void *
CFTypeRetainCallBack (CFAllocatorRef allocator, const void *value);

void
CFTypeReleaseCallBack (CFAllocatorRef alloc, const void *value);

const char *
CFLocaleGetCStringIdentifier (CFLocaleRef locale);

void
GSRuntimeConstantInit (CFTypeRef cf, CFTypeID typeID);

#define GS_MAX(a,b) (a > b ? a : b)
#define GS_MIN(a,b) (a < b ? a : b)



#if defined(_MSC_VER)
CF_INLINE void *
__CFISAForTypeID (CFTypeID typeID)
{
  return NULL;
}

CF_INLINE Boolean
CF_IS_OBJC (CFTypeID typeID, const void *obj)
{
  return false;
}

#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel)
#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1)
#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2)
#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3)
#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4)
#define CF_OBJC_FUNCDISPATCH5(typeID, rettype, obj, sel, a1, a2, a3, a4, a5)
#else
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
#endif /* defined(_MSC_VER) */

#endif /* __GSPRIVATE_H__ */
