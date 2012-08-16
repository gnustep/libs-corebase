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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFSet.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStream.h"

#include "GSPrivate.h"

static CFTypeID _kCFArrayTypeID = 0;
static CFTypeID _kCFBooleanTypeID = 0;
static CFTypeID _kCFDataTypeID = 0;
static CFTypeID _kCFDateTypeID = 0;
static CFTypeID _kCFDictionaryTypeID = 0;
static CFTypeID _kCFNumberTypeID = 0;
static CFTypeID _kCFStringTypeID = 0;
static Boolean  _kCFPropertyListTypeIDsInitialized = false;

struct CFPListIsValidContext
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
  struct CFPListIsValidContext *ctx = (struct CFPListIsValidContext*)context;
  
  if (ctx->isValid)
    ctx->isValid = value && CFPListTypeIsValid (value, ctx->fmt, ctx->set);
}

static void
CFDictionaryIsValidFunction (const void *key, const void *value,
                             void *context)
{
  struct CFPListIsValidContext *ctx = (struct CFPListIsValidContext*)context;
  
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
      struct CFPListIsValidContext ctx;
      
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
      struct CFPListIsValidContext ctx;
      
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

struct CFPListCopyContext
{
  CFOptionFlags  opts;
  CFAllocatorRef alloc;
  CFTypeRef      container;
};

static void
CFArrayCopyFunction (const void *value, void *context)
{
  CFPropertyListRef newValue;
  struct CFPListCopyContext *ctx = (struct CFPListCopyContext*)context;
  CFMutableArrayRef array = (CFMutableArrayRef)ctx->container;
  
  newValue = CFPropertyListCreateDeepCopy (ctx->alloc, value, ctx->opts);
  CFArrayAppendValue (array, newValue);
  CFRelease (newValue);
}

static void
CFDictionaryCopyFunction (const void *key, const void *value,
                          void *context)
{
  CFPropertyListRef newValue;
  struct CFPListCopyContext *ctx = (struct CFPListCopyContext*)context;
  CFMutableDictionaryRef dict = (CFMutableDictionaryRef)ctx->container;
  
  newValue = CFPropertyListCreateDeepCopy (ctx->alloc, value, ctx->opts);
  CFDictionaryAddValue (dict, key, newValue);
  CFRelease (newValue);
}

CFPropertyListRef
CFPropertyListCreateDeepCopy (CFAllocatorRef alloc, CFPropertyListRef plist,
                              CFOptionFlags opts)
{
  CFPropertyListRef copy;
  CFTypeID typeID;
  
  typeID = CFGetTypeID (plist);
  if (typeID == _kCFArrayTypeID)
    {
      CFIndex cnt;
      
      cnt = CFArrayGetCount (plist);
      if (opts == kCFPropertyListImmutable)
        {
          CFIndex i;
          CFTypeRef *values;
          CFArrayRef array;
          CFRange range;
          
          values = CFAllocatorAllocate (alloc, cnt * sizeof(CFTypeRef), 0);
          range = CFRangeMake (0, cnt);
          CFArrayGetValues (plist, range, values);
          for (i = 0 ; i < cnt ; ++i)
            values[i] = CFPropertyListCreateDeepCopy (alloc, values[i], opts);
          array = CFArrayCreate (alloc, values, cnt, &kCFTypeArrayCallBacks);
          for (i = 0 ; i < cnt ; ++i)
            CFRelease (values[i]);
        }
      else
        {
          struct CFPListCopyContext ctx;
          CFMutableArrayRef array;
          CFRange range;
          
          array = CFArrayCreateMutable (alloc, cnt, &kCFTypeArrayCallBacks);
          ctx.opts = opts;
          ctx.alloc = alloc;
          ctx.container = (CFTypeRef)array;
          range = CFRangeMake (0, cnt);
          CFArrayApplyFunction (array, range, CFArrayCopyFunction, &ctx);
        }
    }
  else if (typeID == _kCFDictionaryTypeID)
    {
      CFIndex cnt;
      
      cnt = CFDictionaryGetCount (plist);
      if (opts == kCFPropertyListImmutable)
        {
          CFIndex i;
          CFTypeRef *keys;
          CFTypeRef *values;
          CFDictionaryRef dict;
          
          keys = CFAllocatorAllocate (alloc, 2 * cnt * sizeof(CFTypeRef), 0);
          values = keys + cnt;
          CFDictionaryGetKeysAndValues (plist, keys, values);
          for (i = 0 ; i < cnt ; ++i)
            values[i] = CFPropertyListCreateDeepCopy (alloc, values[i], opts);
          dict = CFDictionaryCreate (alloc, keys, values, cnt,
                                     &kCFCopyStringDictionaryKeyCallBacks,
                                     &kCFTypeDictionaryValueCallBacks);
          for (i = 0 ; i < cnt ; ++i)
            CFRelease (values[i]);
        }
      else
        {
          struct CFPListCopyContext ctx;
          CFMutableDictionaryRef dict;
          
          dict = CFDictionaryCreateMutable (alloc, cnt,
                                            &kCFCopyStringDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks);
          ctx.opts = opts;
          ctx.alloc = alloc;
          ctx.container = (CFTypeRef)dict;
          CFDictionaryApplyFunction (dict, CFDictionaryCopyFunction, &ctx);
        }
    }
  else if (typeID == _kCFStringTypeID)
    {
      if (opts == kCFPropertyListMutableContainersAndLeaves)
        copy = CFStringCreateMutableCopy (alloc, 0, plist);
      else
        copy = CFStringCreateCopy (alloc, plist);
    }
  else if (typeID == _kCFDataTypeID)
    {
      if (opts == kCFPropertyListMutableContainersAndLeaves)
        copy = CFDataCreateMutableCopy (alloc, 0, plist);
      else
        copy = CFDataCreateCopy (alloc, plist);
    }
  else if (typeID == _kCFBooleanTypeID)
    {
      /* CFBoolean instances are singletons */
      return plist;
    }
  else if (typeID == _kCFDateTypeID || typeID == _kCFNumberTypeID)
    {
      /* Take advantage of the runtime functions. */
      return GSTypeCreateCopy (alloc, plist, typeID);
    }
  else
    {
      copy = NULL;
    }
  
  return copy;
}

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                              CFOptionFlags opts, CFPropertyListFormat *fmt,
                              CFErrorRef *error)
{
  const UInt8 *bytes;
  CFIndex length;
  CFReadStreamRef stream;
  CFPropertyListRef plist;
  
  bytes = CFDataGetBytePtr (data);
  length = CFDataGetLength (data);
  stream = CFReadStreamCreateWithBytesNoCopy (alloc, bytes, length,
                                              kCFAllocatorNull);
  plist = CFPropertyListCreateWithStream (alloc, stream, length, opts,
                                          fmt, error);
  CFRelease (stream);
  
  return plist;
}

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat *fmt, CFErrorRef *error)
{
  return NULL;
}

CFDataRef
CFPropertyListCreateData (CFAllocatorRef alloc, CFPropertyListRef plist,
                          CFPropertyListFormat fmt, CFOptionFlags opts,
                          CFErrorRef *error)
{
  CFDataRef data;
  CFWriteStreamRef stream;
  
  stream = CFWriteStreamCreateWithAllocatedBuffers (alloc, alloc);
  CFPropertyListWrite (plist, stream, fmt, opts, error);
  data = CFWriteStreamCopyProperty (stream, kCFStreamPropertyDataWritten);
  
  return data;
}

CFIndex
CFPropertyListWrite (CFPropertyListRef plist, CFWriteStreamRef stream,
                     CFPropertyListFormat fmt, CFOptionFlags opts,
                     CFErrorRef *error)
{
  return 0;
}



/* The following functions are marked as obsolete as of Mac OS X 10.6.  They
 * will be implemented here as wrappers around the new functions.
 */
CFDataRef
CFPropertyListCreateXMLData (CFAllocatorRef alloc, CFPropertyListRef plist)
{
  return CFPropertyListCreateData (alloc, plist, kCFPropertyListXMLFormat_v1_0,
                                   0, NULL);
}

CFIndex
CFPropertyListWriteToStream (CFPropertyListRef plist, CFWriteStreamRef stream,
                             CFPropertyListFormat fmt, CFStringRef *errStr)
{
  CFIndex ret;
  CFErrorRef err = NULL;
  
  ret = CFPropertyListWrite (plist, stream, fmt, 0, &err);
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

