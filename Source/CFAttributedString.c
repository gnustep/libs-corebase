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

typedef struct
{
  CFIndex         index;
  CFDictionaryRef attrib;
} Attr;

struct __CFAttributedString
{
  CFRuntimeBase parent;
  CFStringRef   _string;
  CFIndex       _attribCount;
  Attr         *_attribs;
};

enum
{
  _kCFAttributedStringIsMutable = (1<<0),
  _kCFAttributedStringIsInline =  (1<<1)
};

CF_INLINE Boolean
CFAttributedStringIsMutable (CFAttributedStringRef str)
{
  return ((CFRuntimeBase *)str)->_flags.info & _kCFAttributedStringIsMutable ?
    true : false;
}

CF_INLINE Boolean
CFAttributedStringIsInline (CFAttributedStringRef str)
{
  return ((CFRuntimeBase *)str)->_flags.info & _kCFAttributedStringIsInline ?
    true : false;
}

CF_INLINE void
CFAttributedStringSetMutable (CFAttributedStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFAttributedStringIsMutable;
}

CF_INLINE void
CFAttributedStringSetInline (CFAttributedStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFAttributedStringIsInline;
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

static void
CFAttributedStringFinalize (CFTypeRef cf)
{
  CFIndex idx;
  CFAttributedStringRef str = (CFAttributedStringRef)cf;
  
  CFRelease (str->_string);
  
  for (idx = 0 ; idx < str->_attribCount ; ++idx)
    CFAttributedStringUncacheAttribute (str->_attribs[idx].attrib);
  
  if (!CFAttributedStringIsInline(str))
    CFAllocatorDeallocate (CFGetAllocator(str), str->_attribs);
}

static Boolean
CFAttributedStringEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFAttributedStringRef str1 = (CFAttributedStringRef)cf1;
  CFAttributedStringRef str2 = (CFAttributedStringRef)cf2;
  
  if (CFEqual (str1->_string, str2->_string)
      && str1->_attribCount == str2->_attribCount)
    {
      CFIndex idx;
      
      for (idx = 0 ; idx < str1->_attribCount ; ++idx)
        if (!CFEqual (str1->_attribs[idx].attrib, str2->_attribs[idx].attrib))
          return false;
      
      return true;
    }
  
  return false;
}

static CFHashCode
CFAttributedStringHash (CFTypeRef cf)
{
  CFHashCode hash;
  CFAttributedStringRef str = (CFAttributedStringRef)cf;
  
  hash = CFHash (str->_string);
  hash += str->_attribCount;
  
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

void CFAttributedStringInitialize (void)
{
  _kCFAttributedStringTypeID =
    _CFRuntimeRegisterClass (&CFAttributedStringClass);
  GSMutexInitialize (&_kCFAttributedStringCacheLock);
}



CFTypeID
CFAttributedStringGetTypeID (void)
{
  return _kCFAttributedStringTypeID;
}

#define CFATTRIBUTESTRING_SIZE sizeof(CFRuntimeClass) \
  - sizeof(struct __CFAttributedString)

CFAttributedStringRef
CFAttributedStringCreate (CFAllocatorRef alloc, CFStringRef str,
                          CFDictionaryRef attribs)
{
  struct __CFAttributedString *new;
  
  new = (struct __CFAttributedString *)_CFRuntimeCreateInstance (alloc,
    _kCFAttributedStringTypeID, CFATTRIBUTESTRING_SIZE + sizeof(Attr), 0);
  
  if (new)
    {
      new->_string = CFStringCreateCopy (alloc, str);
      new->_attribCount = 1;
      new->_attribs = (Attr*)&new[1];
      new->_attribs[0].index = 0;
      new->_attribs[0].attrib = CFAttributedStringCacheAttribute (attribs);
      
      CFAttributedStringSetInline (new);
    }
  
  return new;
}

CFAttributedStringRef
CFAttributedStringCreateCopy (CFAllocatorRef alloc, CFAttributedStringRef str)
{
  return NULL;
}

CFAttributedStringRef
CFAttributedStringCreateWithSubstring (CFAllocatorRef alloc,
                                       CFAttributedStringRef str,
                                       CFRange range)
{
  return NULL;
}

CFIndex
CFAttributedStringGetLength (CFAttributedStringRef str)
{
  CF_OBJC_FUNCDISPATCH0 (_kCFAttributedStringTypeID, CFIndex, str, "length");
  
  return CFStringGetLength (str->_string);
}

CFStringRef
CFAttributedStringGetString (CFAttributedStringRef str)
{
  CF_OBJC_FUNCDISPATCH0 (_kCFAttributedStringTypeID, CFStringRef, str,
                         "string");
  
  return str->_string;
}

CFTypeRef
CFAttributedStringGetAttribute (CFAttributedStringRef str, CFIndex loc,
                                CFStringRef attrName, CFRange *effRange)
{
  CFDictionaryRef attribs;
  
  attribs = CFAttributedStringGetAttributes (str, loc, effRange);
  return CFDictionaryGetValue (attribs, attrName);
}

static CFComparisonResult
CFAttributedStringCompareAttribute (const void *v1, const void *v2, void *ctxt)
{
  Attr *attr1 = (Attr*)v1;
  Attr *attr2 = (Attr*)v2;
  
  return attr1->index < attr2->index ? kCFCompareLessThan :
    (attr1->index == attr2->index ? kCFCompareEqualTo : kCFCompareGreaterThan);
}

CFDictionaryRef
CFAttributedStringGetAttributes (CFAttributedStringRef str, CFIndex loc,
                                 CFRange *effRange)
{
  Attr attr;
  CFIndex idx;
  
  CF_OBJC_FUNCDISPATCH2 (_kCFAttributedStringTypeID, CFDictionaryRef, str,
                         "attributesAtIndex:effectiveRange:", loc, effRange);
  
  attr.index = loc;
  idx = GSBSearch (str->_attribs, &attr, CFRangeMake(0, str->_attribCount),
                   sizeof(Attr), CFAttributedStringCompareAttribute, NULL);
  
  if (effRange)
    {
      CFIndex start;
      CFIndex end;
      
      start = str->_attribs[idx].index;
      effRange->location = start;
      
      if (idx < str->_attribCount)
        end = str->_attribs[idx + 1].index;
      else
        end = CFStringGetLength(str->_string);
      
      effRange->length = end - start;
    }
  
  return str->_attribs[idx].attrib;
}

CFTypeRef
CFAttributedStringGetAttributeAndLongestEffectiveRange (
  CFAttributedStringRef str, CFIndex loc, CFStringRef attrName,
  CFRange inRange, CFRange *longestEffRange)
{
  return NULL; /* FIXME */
}

CFDictionaryRef
CFAttributedStringGetAttributesAndLongestEffectiveRange (
  CFAttributedStringRef str, CFIndex loc, CFRange inRange,
  CFRange *longestEffRange)
{
  return NULL; /* FIXME */
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
  CF_OBJC_FUNCDISPATCH0 (_kCFAttributedStringTypeID, void, str,
                         "beginEditing");
}

void
CFAttributedStringEndEditing (CFMutableAttributedStringRef str)
{
  CF_OBJC_FUNCDISPATCH0 (_kCFAttributedStringTypeID, void, str,
                         "endEditing");
}

CFMutableStringRef
CFAttributedStringGetMutableString (CFMutableAttributedStringRef str)
{
  CF_OBJC_FUNCDISPATCH0 (_kCFAttributedStringTypeID, CFMutableStringRef, str,
                         "mutableString");
  
  if (CFAttributedStringIsMutable(str))
    return (CFMutableStringRef)str->_string;
  
  return NULL;
}

void
CFAttributedStringRemoveAttribute (CFMutableAttributedStringRef str,
                                   CFRange range, CFStringRef attrName)
{
  CF_OBJC_FUNCDISPATCH2 (_kCFAttributedStringTypeID, void, str,
                         "removeAttribute:range:", attrName, range);
}

void
CFAttributedStringReplaceString (CFMutableAttributedStringRef str,
                                 CFRange range, CFStringRef repl)
{
  CF_OBJC_FUNCDISPATCH2 (_kCFAttributedStringTypeID, void, str,
                         "replaceCharactersInRange:withString:", range, repl);
}

void
CFAttributedStringReplaceAttributedString (CFMutableAttributedStringRef str,
                                           CFRange range,
                                           CFAttributedStringRef repl)
{
  CF_OBJC_FUNCDISPATCH2 (_kCFAttributedStringTypeID, void, str,
                         "replaceCharactersInRange:withAttributeString:",
                         range, repl);
}

void
CFAttributedStringSetAttribute (CFMutableAttributedStringRef str,
  CFRange range, CFStringRef attrName, CFTypeRef value)
{
  CF_OBJC_FUNCDISPATCH3 (_kCFAttributedStringTypeID, void, str,
                         "setAttribute:value:range:", attrName, value, range);
}

void
CFAttributedStringSetAttributes (CFMutableAttributedStringRef str,
                                 CFRange range, CFDictionaryRef repl,
                                 Boolean clearOtherAttribs)
{
  CF_OBJC_FUNCDISPATCH2 (_kCFAttributedStringTypeID, void, str,
                         "setAttributes:range:", repl, range);
}
