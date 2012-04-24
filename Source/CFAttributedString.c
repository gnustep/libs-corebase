/* CFAttributedString.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: April, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
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

/* The implementation of CFAttributeString is based almost entirely on
 * GSAttributedString in the GNUstep Base Library.
 */

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFAttributedString.h"
#include "CoreFoundation/CFBag.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "GSPrivate.h"

static CFTypeID _kCFAttributedStringTypeID = 0;
static CFMutableBagRef _kCFAttributedStringCache = NULL;
static GSMutex _kCFAttributedStringCacheLock;

struct __CFAttributedString
{
  CFRuntimeBase parent;
  CFStringRef   _string;
  CFArrayRef    _infoArray;
};

enum
{
  _kCFAttributedStringIsMutable = (1<<0)
};

CF_INLINE Boolean
CFAttributedStringIsMutable (CFAttributedStringRef str)
{
  return ((CFRuntimeBase *)str)->_flags.info & _kCFAttributedStringIsMutable ?
    true : false;
}

CF_INLINE void
CFAttributedStringSetMutable (CFAttributedStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFAttributedStringIsMutable;
}

static void
CFAttributedStringFinalize (CFTypeRef cf)
{
  CFAttributedStringRef str = (CFAttributedStringRef)cf;
  
  CFRelease (str->_string);
  CFRelease (str->_infoArray);
}

static Boolean
CFAttributedStringEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFAttributedStringRef str1 = (CFAttributedStringRef)cf1;
  CFAttributedStringRef str2 = (CFAttributedStringRef)cf2;
  
  if (CFEqual (str1->_string, str2->_string)
      && CFEqual (str1->_infoArray, str2->_infoArray))
    return true;
  
  return false;
}

static CFHashCode
CFAttributedStringHash (CFTypeRef cf)
{
  CFHashCode hash;
  CFAttributedStringRef str = (CFAttributedStringRef)cf;
  
  hash = CFHash (str->_string);
  hash += CFArrayGetCount (str->_infoArray);
  
  return hash;
}

CFRuntimeClass CFAttributedStringClass =
{
  0,
  "CFAttributedString",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFAttributedStringCreateCopy,
  CFAttributedStringFinalize,
  CFAttributedStringEqual,
  CFAttributedStringHash,
  NULL,
  NULL
};

void CFAttributeStringInitialize (void)
{
  _kCFAttributedStringTypeID =
    _CFRuntimeRegisterClass (&CFAttributedStringClass);
  GSMutexInitialize (&_kCFAttributedStringCacheLock);
}



static CFDictionaryRef
CFAttributedStringCacheAttribute (CFDictionaryRef attribs)
{
  CFDictionaryRef cachedAttr;
  
  GSMutexLock (&_kCFAttributedStringCacheLock);
  
  if (_kCFAttributedStringCache == NULL)
    {
      _kCFAttributedStringCache =
        CFBagCreateMutable (kCFAllocatorSystemDefault, 0, &kCFTypeBagCallBacks);
      cachedAttr = NULL;
    }
  else
    {
      cachedAttr = CFBagGetValue (_kCFAttributedStringCache, attribs);
    }
  
  if (cachedAttr == NULL)
    {
      CFBagAddValue (_kCFAttributedStringCache, attribs);
      cachedAttr = attribs;
    }
  
  GSMutexUnlock (&_kCFAttributedStringCacheLock);
  
  return cachedAttr;
}

static void
CFAttributedStringUncacheAttribute (CFDictionaryRef attribs)
{
  GSMutexLock (&_kCFAttributedStringCacheLock);
  
  CFBagRemoveValue (_kCFAttributedStringCache, attribs);
  
  GSMutexUnlock (&_kCFAttributedStringCacheLock);
}

CFTypeID
CFAttributedStringGetTypeID (void)
{
  return _kCFAttributedStringTypeID;
}

CFAttributedStringRef
CFAttributedStringCreate (CFAllocatorRef alloc, CFStringRef str,
  CFDictionaryRef attribs)
{
  return NULL;
}

CFAttributedStringRef
CFAttributedStringCreateCopy (CFAllocatorRef alloc, CFAttributedStringRef str)
{
  return NULL;
}

CFAttributedStringRef
CFAttributedStringCreateWithSubstring (CFAllocatorRef alloc,
  CFAttributedStringRef str, CFRange range)
{
  return NULL;
}

CFIndex
CFAttributedStringGetLength (CFAttributedStringRef str)
{
  return CFStringGetLength (str->_string);
}

CFStringRef
CFAttributedStringGetString (CFAttributedStringRef str)
{
  return str->_string;
}

CFTypeRef
CFAttributedStringGetAttribute (CFAttributedStringRef str, CFIndex loc,
  CFStringRef attrName, CFRange *effRange)
{
  return NULL;
}

CFDictionaryRef
CFAttributedStringGetAttributes (CFAttributedStringRef str, CFIndex loc,
  CFRange *effRange)
{
  return NULL;
}

CFTypeRef
CFAttributedStringGetAttributeAndLongestEffectiveRange (
  CFAttributedStringRef str, CFIndex loc, CFStringRef attrName,
  CFRange inRange, CFRange *longestEffRange)
{
  return NULL;
}

CFDictionaryRef
CFAttributedStringGetAttributesAndLongestEffectiveRange (
  CFAttributedStringRef str, CFIndex loc, CFRange inRange,
  CFRange *longestEffRange)
{
  return NULL;
}



CFMutableAttributedStringRef
CFAttributedStringCreateMutable (CFAllocatorRef alloc, CFIndex maxLength)
{
  return NULL;
}

CFMutableAttributedStringRef
CFAttributedStringCreateMutableCopy (CFAllocatorRef alloc, CFIndex maxLength,
  CFAttributedStringRef str)
{
  return NULL;
}

void
CFAttributedStringBeginEditing (CFMutableAttributedStringRef str)
{
  
}

void
CFAttributedStringEndEditing (CFMutableAttributedStringRef str)
{
  
}

CFMutableStringRef
CFAttributedStringGetMutableString (CFMutableAttributedStringRef str)
{
  if (CFAttributedStringIsMutable(str))
    return (CFMutableStringRef)str->_string;
  
  return NULL;
}

void
CFAttributedStringRemoveAttribute (CFMutableAttributedStringRef str,
  CFRange range, CFStringRef attrName)
{
  
}

void
CFAttributedStringReplaceString (CFMutableAttributedStringRef str,
  CFRange range, CFStringRef repl)
{
  
}

void
CFAttributedStringReplaceAttributedString (CFMutableAttributedStringRef str,
  CFRange range, CFAttributedStringRef repl)
{
  
}

void
CFAttributedStringSetAttribute (CFMutableAttributedStringRef str,
  CFRange range, CFStringRef attrName, CFTypeRef value)
{
  
}

void
CFAttributedStringSetAttributes (CFMutableAttributedStringRef str,
  CFRange range, CFDictionaryRef repl, Boolean clearOtherAttribs)
{
  
}
