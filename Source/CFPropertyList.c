/* CFPropertyList.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: August, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#include "CoreFoundation/CFPropertyList.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFSet.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStream.h"

static CFTypeID _kCFArrayTypeID = 0;
static CFTypeID _kCFBooleanTypeID = 0;
static CFTypeID _kCFDataTypeID = 0;
static CFTypeID _kCFDateTypeID = 0;
static CFTypeID _kCFDictionaryTypeID = 0;
static CFTypeID _kCFNumberTypeID = 0;
static CFTypeID _kCFStringTypeID = 0;
static Boolean  _kCFPropertyListTypeIDsInitialized = false;

struct CFPListContext
{
  Boolean  isValid;
  CFPropertyListFormat fmt;
  CFMutableSetRef set; /* Used to check for cycles */
};

static void
CFPropertyListInitTypeIDs (void)
{
  if (_kCFPropertyListTypeIDsInitialized)
    return;
  
  _kCFPropertyListTypeIDsInitialized = true;
  _kCFArrayTypeID = CFArrayGetTypeID ();
  _kCFBooleanTypeID = CFBooleanGetTypeID ();
  _kCFDataTypeID = CFDataGetTypeID ();
  _kCFDateTypeID = CFDateGetTypeID ();
  _kCFDictionaryTypeID = CFDictionaryGetTypeID ();
  _kCFNumberTypeID = CFNumberGetTypeID ();
  _kCFStringTypeID = CFStringGetTypeID ();
}

static Boolean
CFPListTypeIsValid (CFPropertyListRef plist, CFPropertyListFormat fmt,
                    CFMutableSetRef set);

static void
CFArrayIsValidFunction (const void *value, void *context)
{
  struct CFPListContext *ctx = (struct CFPListContext*)context;
  
  if (ctx->isValid)
    ctx->isValid = value && CFPListTypeIsValid (value, ctx->fmt, ctx->set);
}

static void
CFDictionaryIsValidFunction (const void *key, const void *value,
                             void *context)
{
  struct CFPListContext *ctx = (struct CFPListContext*)context;
  
  if (ctx->isValid)
    {
      ctx->isValid = key
                     && (CFGetTypeID (key) == _kCFStringTypeID)
                     && value
                     && CFPListTypeIsValid (value, ctx->fmt, ctx->set);
    }
}

static Boolean
CFPListTypeIsValid (CFPropertyListRef plist, CFPropertyListFormat fmt,
                    CFMutableSetRef set)
{
  CFTypeID typeID;
  
  CFPropertyListInitTypeIDs ();
  typeID = CFGetTypeID (plist);
  if (typeID == _kCFDataTypeID || typeID == _kCFStringTypeID)
    return true;
  if (fmt != kCFPropertyListOpenStepFormat)
    {
      /* These are not supported by the OPENSTEP property list format. */
      if (typeID == _kCFBooleanTypeID
          || typeID == _kCFDateTypeID
          || typeID == _kCFNumberTypeID)
        return true;
    }
  
  /* Check for cycles */
  if (CFSetContainsValue (set, plist))
    return false;
  
  CFSetAddValue (set, plist);
  if (typeID == _kCFArrayTypeID)
    {
      CFRange range;
      struct CFPListContext ctx;
      
      range = CFRangeMake (0, CFArrayGetCount(plist));
      ctx.isValid = true;
      ctx.fmt = fmt;
      ctx.set = set;
      CFArrayApplyFunction (plist, range, CFArrayIsValidFunction, &ctx);
      CFSetRemoveValue (set, plist);
      
      return ctx.isValid;
    }
  else if (typeID == _kCFDictionaryTypeID)
    {
      struct CFPListContext ctx;
      
      ctx.isValid = true;
      ctx.fmt = fmt;
      ctx.set = set;
      CFDictionaryApplyFunction (plist, CFDictionaryIsValidFunction, &ctx);
      CFSetRemoveValue (set, plist);
      
      return ctx.isValid;
    }
  
  return false;
}

Boolean
CFPropertyListIsValid (CFPropertyListRef plist, CFPropertyListFormat fmt)
{
  CFMutableSetRef set;
  Boolean ret;
  set = CFSetCreateMutable (NULL, 0, &kCFTypeSetCallBacks);
  ret = CFPListTypeIsValid (plist, fmt, set);
  CFRelease (set);
  return ret;
}

CFPropertyListRef
CFPropertyListCreateDeepCopy (CFAllocatorRef alloc, CFPropertyListRef pList,
                              CFOptionFlags opts)
{
  return NULL;
}

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                              CFOptionFlags opts, CFPropertyListFormat *fmt,
                              CFErrorRef *error)
{
  return NULL;
}

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat *fmt, CFErrorRef *error)
{
  return NULL;
}

CFDataRef
CFPropertyListCreateData (CFAllocatorRef alloc, CFPropertyListRef pList,
                          CFPropertyListFormat fmt, CFOptionFlags opts,
                          CFErrorRef *error)
{
  return NULL;
}

CFIndex
CFPropertyListWrite (CFPropertyListRef pList, CFWriteStreamRef stream,
                     CFPropertyListFormat fmt, CFOptionFlags opts,
                     CFErrorRef *error)
{
  return 0;
}



/* The following functions are marked as obsolete as of Mac OS X 10.6.  They
 * will be implemented here as wrappers around the new functions.
 */
CFDataRef
CFPropertyListCreateXMLData (CFAllocatorRef alloc, CFPropertyListRef pList)
{
  return CFPropertyListCreateData (alloc, pList, kCFPropertyListXMLFormat_v1_0,
                                   0, NULL);
}

CFIndex
CFPropertyListWriteToStream (CFPropertyListRef pList, CFWriteStreamRef stream,
                             CFPropertyListFormat fmt, CFStringRef *errStr)
{
  CFIndex ret;
  CFErrorRef err = NULL;
  
  ret = CFPropertyListWrite (pList, stream, fmt, 0, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }

  return ret;
}

CFPropertyListRef
CFPropertyListCreateFromXMLData (CFAllocatorRef alloc, CFDataRef data,
                                 CFOptionFlags opts, CFStringRef *errStr)
{
  CFPropertyListRef plist;
  CFErrorRef err = NULL;
  
  plist = CFPropertyListCreateWithData (alloc, data, opts, NULL, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }
  
  return plist;
}

CFPropertyListRef
CFPropertyListCreateFromStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat *fmt, CFStringRef *errStr)
{
  CFPropertyListRef plist;
  CFErrorRef err = NULL;
  
  plist = CFPropertyListCreateWithStream (alloc, stream, len, opts, fmt, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }
  
  return plist;
}

