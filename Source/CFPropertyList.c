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
static Boolean _kCFPropertyListTypeIDsInitialized = false;

struct CFPListIsValidContext
{
  Boolean isValid;
  CFPropertyListFormat fmt;
  CFMutableSetRef set;          /* Used to check for cycles */
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
  struct CFPListIsValidContext *ctx = (struct CFPListIsValidContext *) context;

  if (ctx->isValid)
    ctx->isValid = value && CFPListTypeIsValid (value, ctx->fmt, ctx->set);
}

static void
CFDictionaryIsValidFunction (const void *key, const void *value, void *context)
{
  struct CFPListIsValidContext *ctx = (struct CFPListIsValidContext *) context;

  if (ctx->isValid)
    {
      ctx->isValid = key
        && (CFGetTypeID (key) == _kCFStringTypeID)
        && value && CFPListTypeIsValid (value, ctx->fmt, ctx->set);
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
          || typeID == _kCFDateTypeID || typeID == _kCFNumberTypeID)
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

      range = CFRangeMake (0, CFArrayGetCount (plist));
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
  CFOptionFlags opts;
  CFAllocatorRef alloc;
  CFTypeRef container;
};

static void
CFArrayCopyFunction (const void *value, void *context)
{
  CFPropertyListRef newValue;
  struct CFPListCopyContext *ctx = (struct CFPListCopyContext *) context;
  CFMutableArrayRef array = (CFMutableArrayRef) ctx->container;

  newValue = CFPropertyListCreateDeepCopy (ctx->alloc, value, ctx->opts);
  CFArrayAppendValue (array, newValue);
  CFRelease (newValue);
}

static void
CFDictionaryCopyFunction (const void *key, const void *value, void *context)
{
  CFPropertyListRef newValue;
  struct CFPListCopyContext *ctx = (struct CFPListCopyContext *) context;
  CFMutableDictionaryRef dict = (CFMutableDictionaryRef) ctx->container;

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

          values = CFAllocatorAllocate (alloc, cnt * sizeof (CFTypeRef), 0);
          range = CFRangeMake (0, cnt);
          CFArrayGetValues (plist, range, values);
          for (i = 0; i < cnt; ++i)
            values[i] = CFPropertyListCreateDeepCopy (alloc, values[i], opts);
          array = CFArrayCreate (alloc, values, cnt, &kCFTypeArrayCallBacks);
          for (i = 0; i < cnt; ++i)
            CFRelease (values[i]);

          copy = array;
        }
      else
        {
          struct CFPListCopyContext ctx;
          CFMutableArrayRef array;
          CFRange range;

          array = CFArrayCreateMutable (alloc, cnt, &kCFTypeArrayCallBacks);
          ctx.opts = opts;
          ctx.alloc = alloc;
          ctx.container = (CFTypeRef) array;
          range = CFRangeMake (0, cnt);
          CFArrayApplyFunction (array, range, CFArrayCopyFunction, &ctx);

          copy = array;
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

          keys = CFAllocatorAllocate (alloc, 2 * cnt * sizeof (CFTypeRef), 0);
          values = keys + cnt;
          CFDictionaryGetKeysAndValues (plist, keys, values);
          for (i = 0; i < cnt; ++i)
            values[i] = CFPropertyListCreateDeepCopy (alloc, values[i], opts);
          dict = CFDictionaryCreate (alloc, keys, values, cnt,
                                     &kCFCopyStringDictionaryKeyCallBacks,
                                     &kCFTypeDictionaryValueCallBacks);
          for (i = 0; i < cnt; ++i)
            CFRelease (values[i]);

          copy = dict;
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
          ctx.container = (CFTypeRef) dict;
          CFDictionaryApplyFunction (dict, CFDictionaryCopyFunction, &ctx);

          copy = dict;
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
  else if (typeID == _kCFDateTypeID)
    {
      copy = CFDateCreate (alloc, CFDateGetAbsoluteTime (plist));
    }
  else if (typeID == _kCFNumberTypeID)
    {
      UInt8 number[16];         /* Largest type size */
      CFNumberType type;

      type = CFNumberGetType (plist);
      CFNumberGetValue (plist, type, number);
      copy = CFNumberCreate (alloc, type, number);
    }
  else if (typeID == _kCFBooleanTypeID)
    {
      /* CFBoolean instances are singletons */
      copy = plist;
    }
  else
    {
      copy = NULL;
    }

  return copy;
}

/* Binary property list object ref */
#define PL_BOOL 0x0
#define PL_INT  0x1
#define PL_REAL 0x2
#define PL_DATE 0x3
#define PL_DATA 0x4
#define PL_ASTR 0x5
#define PL_USTR 0x6
#define PL_UID  0x8
#define PL_ARRY 0xA
#define PL_SET  0xC
#define PL_DICT 0xD

static void
CFBinaryPListAppendObject (CFPropertyListRef plist, CFMutableDataRef data,
                           CFIndex level, CFOptionFlags options)
{
  /* HEADER:
   *  'bplist00'
   *
   * OBJECTS:
   *  
   *
   * OBJECT TABLE:
   *  
   *
   * METADATA:
   *  
   */
  CFTypeID typeID;

  typeID = CFGetTypeID (plist);
  if (typeID == CFArrayGetTypeID ())
    {

    }
  else if (typeID == CFBooleanGetTypeID ())
    {

    }
  else if (typeID == CFDataGetTypeID ())
    {

    }
  else if (typeID == CFDateGetTypeID ())
    {

    }
  else if (typeID == CFDictionaryGetTypeID ())
    {

    }
  else if (typeID == CFNumberGetTypeID ())
    {

    }
  else if (typeID == CFStringGetTypeID ())
    {

    }
  else
    {
      return;
    }
}

static CFPropertyListRef
CFBinaryPListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                             CFOptionFlags opts, CFErrorRef * err)
{
  return NULL;
}

static void
CFBinaryPListWriteToData (CFPropertyListRef plist, CFMutableDataRef data,
                          CFOptionFlags opts, CFErrorRef * err)
{
  //CFIndex level;

}

static const char *indentString = "\t\t\t\t\t\t\t\t";
#define MAX_DEPTH 8

static void
CFPListAppendIndentation (CFIndex level, CFMutableDataRef data,
                          CFOptionFlags options)
{
  while (level > MAX_DEPTH)
    {
      CFDataAppendBytes (data, (const UInt8 *) indentString, MAX_DEPTH);
      level -= MAX_DEPTH;
    }
  CFDataAppendBytes (data, (const UInt8 *) indentString, level);
}


static void
CFPListAppendBase16Data (CFDataRef obj, CFDataRef data, CFOptionFlags options)
{

}

static void
CFPListAppendBase64Data (CFDataRef obj, CFDataRef data, CFOptionFlags options)
{

}

#define BUFFER_SIZE 256

static void
CFPListAppendString (CFStringRef str, CFMutableDataRef data,
                     CFOptionFlags options)
{
  UInt8 buffer[BUFFER_SIZE];
  CFIndex length;
  CFIndex read;
  CFIndex used;

  length = CFStringGetLength (str);
  read = 0;
  do
    {
      read = CFStringGetBytes (str, CFRangeMake (read, length),
                               kCFStringEncodingUTF8, 0, false, buffer,
                               BUFFER_SIZE, &used);
      CFDataAppendBytes (data, (const UInt8 *) buffer, used);
      read += used;
      length -= used;
    }
  while (length > 0);
}

static void
CFXMLPListAppendObject (CFPropertyListRef plist, CFMutableDataRef data,
                        CFIndex level, CFOptionFlags options)
{
  CFTypeID typeID;

  CFPListAppendIndentation (level, data, options);
  typeID = CFGetTypeID (plist);
  if (typeID == CFArrayGetTypeID ())
    {
      /* <array>
       *   Object 1
       *   Object 2
       *   ...
       * </array>
       */
      CFIndex idx;
      CFIndex count;

      CFDataAppendBytes (data, (const UInt8 *) "<array>\n", 8);

      count = CFArrayGetCount ((CFArrayRef)plist);
      for (idx = 0; idx < count; ++idx)
        {
          CFPropertyListRef obj;
          obj = CFArrayGetValueAtIndex ((CFArrayRef) plist, idx);
          CFXMLPListAppendObject (obj, data, level + 1, options);
        }
      CFPListAppendIndentation (level, data, options);
      CFDataAppendBytes (data, (const UInt8 *) "</array>", 8);
    }
  else if (typeID == CFBooleanGetTypeID ())
    {
      /* <true/> or <false/>
       */
      if (plist == kCFBooleanTrue)
        CFDataAppendBytes (data, (const UInt8 *) "<true/>", 7);
      else if (plist == kCFBooleanFalse)
        CFDataAppendBytes (data, (const UInt8 *) "<false/>", 8);
    }
  else if (typeID == CFDataGetTypeID ())
    {
      /* <data>Base64 Data</data>
       */
      CFDataAppendBytes (data, (const UInt8 *) "<data>", 6);
      CFPListAppendBase64Data ((CFDataRef) plist, data, options);
      CFDataAppendBytes (data, (const UInt8 *) "</data>", 7);
    }
  else if (typeID == CFDateGetTypeID ())
    {
      /* <date>%04d-%02d-%02dT%02d:%02d:%02dZ</date>
       */
      CFAbsoluteTime at;
      CFGregorianDate gdate;
      int printed;
      char buffer[21];          /* Size of "%04d-%02d-%02dT%02d:%02d:%02dZ" + '\0' */

      at = CFDateGetAbsoluteTime ((CFDateRef) plist);
      gdate = CFAbsoluteTimeGetGregorianDate (at, NULL);
      printed = sprintf (buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ", gdate.year,
                         gdate.month, gdate.day, gdate.hour, gdate.minute,
                         (SInt32) gdate.second);
      if (printed < 20)
        return;                 /* Do something with the error? */
      CFDataAppendBytes (data, (const UInt8 *) "<date>", 6);
      CFDataAppendBytes (data, (const UInt8 *) buffer, 20);
      CFDataAppendBytes (data, (const UInt8 *) "</date>", 7);
    }
  else if (typeID == CFDictionaryGetTypeID ())
    {

    }
  else if (typeID == CFNumberGetTypeID ())
    {
      if (CFNumberIsFloatType ((CFNumberRef) plist))
        {
          CFDataAppendBytes (data, (const UInt8 *) "<real>", 6);

          CFDataAppendBytes (data, (const UInt8 *) "</real>", 7);
        }
      else
        {
          CFDataAppendBytes (data, (const UInt8 *) "<integer>", 9);

          CFDataAppendBytes (data, (const UInt8 *) "</integer>", 10);
        }
    }
  else if (typeID == CFStringGetTypeID ())
    {
      /* <string>String</string>
       */
      CFDataAppendBytes (data, (const UInt8 *) "<string>", 8);
      CFPListAppendString ((CFStringRef) plist, data, options);
      CFDataAppendBytes (data, (const UInt8 *) "</string>", 9);
    }
  else
    {
      return;
    }

  CFDataAppendBytes (data, (const UInt8 *) "\n", 1);
}

static CFPropertyListRef
CFXMLPListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                          CFOptionFlags opts, CFErrorRef * err)
{
  return NULL;
}

static void
CFXMLPListWriteToData (CFPropertyListRef plist, CFMutableDataRef data,
                       CFOptionFlags opts, CFErrorRef * err)
{

}

static void
CFOpenStepPListAppendObject (CFPropertyListRef plist, CFMutableDataRef data,
                             CFIndex level, CFOptionFlags options)
{
  CFTypeID typeID;

  CFPListAppendIndentation (level, data, options);
  typeID = CFGetTypeID (plist);
  if (typeID == CFArrayGetTypeID ())
    {
      /* (
       *   Object 1
       *   Object 2
       *   ...
       * )
       */
      CFIndex idx;
      CFIndex count;

      CFDataAppendBytes (data, (const UInt8 *) "(\n", 2);

      count = CFArrayGetCount ((CFArrayRef)plist);
      for (idx = 0; idx < count; ++idx)
        {
          CFPropertyListRef obj;
          obj = CFArrayGetValueAtIndex ((CFArrayRef) plist, idx);
          CFOpenStepPListAppendObject (obj, data, level + 1, options);
        }

      CFPListAppendIndentation (level, data, options);
      CFDataAppendBytes (data, (const UInt8 *) ")", 1);
    }
  else if (typeID == CFDataGetTypeID ())
    {
      /* <Hexadecimal Data>
       */
      CFDataAppendBytes (data, (const UInt8 *) "<", 1);
      CFPListAppendBase16Data ((CFDataRef) plist, data, options);
      CFDataAppendBytes (data, (const UInt8 *) ">", 1);
    }
  else if (typeID == CFDictionaryGetTypeID ())
    {

    }
  else if (typeID == CFStringGetTypeID ())
    {
      /* String or "String"
       */
      CFDataAppendBytes (data, (const UInt8 *) "\"", 1);
      CFPListAppendString ((CFStringRef) plist, data, options);
      CFDataAppendBytes (data, (const UInt8 *) "\"", 1);
    }
  else
    {
      return;
    }

  CFDataAppendBytes (data, (const UInt8 *) "\n", 1);
}

static CFPropertyListRef
CFOpenStepPListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                               CFOptionFlags opts, CFErrorRef * err)
{
  return NULL;
}

static void
CFOpenStepPListWriteToData (CFPropertyListRef plist, CFMutableDataRef data,
                            CFOptionFlags opts, CFErrorRef * err)
{

}

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                              CFOptionFlags opts, CFPropertyListFormat * fmt,
                              CFErrorRef * error)
{
  /* FIXME */
  return NULL;
}

CFDataRef
CFPropertyListCreateData (CFAllocatorRef alloc, CFPropertyListRef plist,
                          CFPropertyListFormat fmt, CFOptionFlags opts,
                          CFErrorRef * error)
{
  CFMutableDataRef tmp;
  CFDataRef data;

  tmp = CFDataCreateMutable (alloc, 0);
  if (fmt == kCFPropertyListOpenStepFormat)
    CFOpenStepPListWriteToData (plist, tmp, opts, error);
  else if (fmt == kCFPropertyListXMLFormat_v1_0)
    CFXMLPListWriteToData (plist, tmp, opts, error);
  else if (fmt == kCFPropertyListBinaryFormat_v1_0)
    CFBinaryPListWriteToData (plist, tmp, opts, error);
  data = CFDataCreateCopy (alloc, tmp);
  CFRelease (tmp);

  return data;
}

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat * fmt, CFErrorRef * error)
{
  return NULL;
}

CFIndex
CFPropertyListWrite (CFPropertyListRef plist, CFWriteStreamRef stream,
                     CFPropertyListFormat fmt, CFOptionFlags opts,
                     CFErrorRef * error)
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
                             CFPropertyListFormat fmt, CFStringRef * errStr)
{
  CFIndex ret;
  CFErrorRef err = NULL;

  ret = CFPropertyListWrite (plist, stream, fmt, 0, &err);
  if (ret == 0)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }

  return ret;
}

CFPropertyListRef
CFPropertyListCreateFromXMLData (CFAllocatorRef alloc, CFDataRef data,
                                 CFOptionFlags opts, CFStringRef * errStr)
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
                                CFPropertyListFormat * fmt,
                                CFStringRef * errStr)
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
