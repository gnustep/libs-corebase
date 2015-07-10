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
#include "CoreFoundation/GSCharacter.h"
#include "CoreFoundation/GSUnicode.h"

#include "GSPrivate.h"
#include "GSCArray.h"
#include "GSMemory.h"

static CFTypeID _kCFArrayTypeID = 0;
static CFTypeID _kCFBooleanTypeID = 0;
static CFTypeID _kCFDataTypeID = 0;
static CFTypeID _kCFDateTypeID = 0;
static CFTypeID _kCFDictionaryTypeID = 0;
static CFTypeID _kCFNumberTypeID = 0;
static CFTypeID _kCFStringTypeID = 0;
static Boolean _kCFPropertyListTypeIDsInitialized = false;

static const UInt32 _kCFOpenStepPlistQuotables[4] =
  { 0xFFFFFFFF, 0xFC00FFFF, 0xF8000001, 0xF8000001 };
#define CFOpenStepPlistCharacterIsQuotable(c) \
  (((c) > 128) \
   || ((_kCFOpenStepPlistQuotables[(c) >> 5] & (1 << ((c) & 0x1F))) > 0))

#if 0
static const UInt8 *_kCFPlistXMLArrayTag = (const UInt8 *) "array";
static const UInt8 *_kCFPlistXMLDictTag = (const UInt8 *) "dict";
static const UInt8 *_kCFPlistXMLKeyTag = (const UInt8 *) "key";
static const UInt8 *_kCFPlistXMLValueTag = (const UInt8 *) "value";
static const UInt8 *_kCFPlistXMLDataTag = (const UInt8 *) "data";
static const UInt8 *_kCFPlistXMLDateTag = (const UInt8 *) "date";
static const UInt8 *_kCFPlistXMLIntegerTag = (const UInt8 *) "integer";
static const UInt8 *_kCFPlistXMLRealTag = (const UInt8 *) "real";
static const UInt8 *_kCFPlistXMLTrueTag = (const UInt8 *) "true";
static const UInt8 *_kCFPlistXMLFalseTag = (const UInt8 *) "false";

static const CFIndex _kCFPlistXMLArrayTagLength = 5;
static const CFIndex _kCFPlistXMLDictTagLength = 4;
static const CFIndex _kCFPlistXMLKeyTagLength = 3;
static const CFIndex _kCFPlistXMLValueTagLength = 5;
static const CFIndex _kCFPlistXMLDataTagLength = 4;
static const CFIndex _kCFPlistXMLDateTagLength = 4;
static const CFIndex _kCFPlistXMLIntegerTagLength = 7;
static const CFIndex _kCFPlistXMLRealTagLength = 4;
static const CFIndex _kCFPlistXMLTrueTagLength = 4;
static const CFIndex _kCFPlistXMLFalseTagLength = 5;
#endif

#define _kCFPlistBufferSize 1024

typedef struct
{
  CFWriteStreamRef stream;
  CFOptionFlags options;
  CFErrorRef error;
  CFIndex written;
  UInt8 *cursor;
  const UInt8 *limit;
  UInt8 buffer[_kCFPlistBufferSize];
} CFPlistWriteStream;

typedef struct
{
  UniChar *buffer;
  CFOptionFlags options;
  CFErrorRef error;
  CFMutableSetRef unique;
  UniChar *cursor;
  const UniChar *limit;
} CFPlistString;

struct CFPlistIsValidContext
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

  _kCFArrayTypeID = CFArrayGetTypeID ();
  _kCFBooleanTypeID = CFBooleanGetTypeID ();
  _kCFDataTypeID = CFDataGetTypeID ();
  _kCFDateTypeID = CFDateGetTypeID ();
  _kCFDictionaryTypeID = CFDictionaryGetTypeID ();
  _kCFNumberTypeID = CFNumberGetTypeID ();
  _kCFStringTypeID = CFStringGetTypeID ();
  _kCFPropertyListTypeIDsInitialized = true;
}

static Boolean
CFPlistTypeIsValid (CFPropertyListRef plist, CFPropertyListFormat fmt,
                    CFMutableSetRef set);

static void
CFArrayIsValidFunction (const void *value, void *context)
{
  struct CFPlistIsValidContext *ctx =
    (struct CFPlistIsValidContext *) context;

  if (ctx->isValid)
    ctx->isValid = value && CFPlistTypeIsValid (value, ctx->fmt, ctx->set);
}

static void
CFDictionaryIsValidFunction (const void *key, const void *value,
                             void *context)
{
  struct CFPlistIsValidContext *ctx =
    (struct CFPlistIsValidContext *) context;

  if (ctx->isValid)
    {
      ctx->isValid = key
        && (CFGetTypeID (key) == _kCFStringTypeID)
        && value && CFPlistTypeIsValid (value, ctx->fmt, ctx->set);
    }
}

static Boolean
CFPlistTypeIsValid (CFPropertyListRef plist, CFPropertyListFormat fmt,
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
      struct CFPlistIsValidContext ctx;

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
      struct CFPlistIsValidContext ctx;

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
  ret = CFPlistTypeIsValid (plist, fmt, set);
  CFRelease (set);
  return ret;
}

struct CFPlistCopyContext
{
  CFOptionFlags opts;
  CFAllocatorRef alloc;
  CFTypeRef container;
};

static void
CFArrayCopyFunction (const void *value, void *context)
{
  CFPropertyListRef newValue;
  struct CFPlistCopyContext *ctx = (struct CFPlistCopyContext *) context;
  CFMutableArrayRef array = (CFMutableArrayRef) ctx->container;

  newValue = CFPropertyListCreateDeepCopy (ctx->alloc, value, ctx->opts);
  CFArrayAppendValue (array, newValue);
  CFRelease (newValue);
}

static void
CFDictionaryCopyFunction (const void *key, const void *value, void *context)
{
  CFPropertyListRef newValue;
  struct CFPlistCopyContext *ctx = (struct CFPlistCopyContext *) context;
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
          CFAllocatorDeallocate (alloc, values);

          copy = array;
        }
      else
        {
          struct CFPlistCopyContext ctx;
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
            {
              CFRelease (keys[i]);
              CFRelease (values[i]);
            }
          CFAllocatorDeallocate (alloc, keys);

          copy = dict;
        }
      else
        {
          struct CFPlistCopyContext ctx;
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

#if 0
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
CFBinaryPlistWriteObject (CFPropertyListRef plist, CFPlistWriteStream stream,
                          CFIndex lev, CFOptionFlags opts)
{
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
#endif

static CFErrorRef
CFPlistCreateError (CFIndex code, CFStringRef message)
{
  const void *key[1];
  const void *value[1];
  CFErrorRef error;

  key[0] = kCFErrorDescriptionKey;
  value[0] = message;
  error = CFErrorCreateWithUserInfoKeysAndValues (kCFAllocatorSystemDefault,
                                                  kCFErrorDomainCocoa,
                                                  code, key, value, 1);

  return error;
}

static Boolean
CFPlistStringSkipWhitespace (CFPlistString * string)
{
  UniChar ch;

  while (string->cursor < string->limit)
    {
      ch = *string->cursor;
      while (string->cursor < string->limit && GSCharacterIsWhitespace (ch))
        {
          string->cursor++;
          ch = *string->cursor;
        }
      if (ch == '/')
        {
          string->cursor++;
          ch = *string->cursor;
          if (ch == '/')
            {
              do
                {
                  string->cursor++;
                  ch = *string->cursor;
                  if (ch == '\n')
                    break;
                }
              while (ch);
            }
          else if (ch == '*')
            {
              do
                {
                  string->cursor++;
                  ch = *string->cursor;
                  if (ch == '*')
                    {
                      string->cursor++;
                      ch = *string->cursor;
                      if (ch == '/')
                        break;
                    }
                }
              while (ch);
            }
          else
            {
              string->cursor--;
              break;
            }
        }
      else
        {
          return true;
        }
    }

  return false;
}

CF_INLINE UInt8
getNibble (UniChar c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'z')
    return (c | 0x20) - 'a';

  return 0xFF; /* This function return 0xFF on error. */
}

static CFDataRef
CFOpenStepPlistParseData (CFAllocatorRef alloc, CFPlistString * string)
{
  UInt8 bytes[_kCFPlistBufferSize];
  UInt8 nibble1;
  UInt8 nibble2;
  CFMutableDataRef obj;
  UniChar ch;
  CFIndex i;

  if (!CFPlistStringSkipWhitespace (string))
    return NULL;

  obj = CFDataCreateMutable (alloc, 0);
  i = 0;
  ch = *string->cursor++;
  nibble1 = getNibble (ch);

  while (nibble1 < 0x10 && string->cursor < string->limit)
    {
      ch = *string->cursor++;
      nibble2 = getNibble (ch);
      if (nibble2 > 0x0F)
        {
          string->error = CFPlistCreateError (kCFPropertyListReadCorruptError,
              CFSTR ("Unexpected character while reading property list data."));
          break;
        }
      bytes[i++] = (nibble1 << 4) | nibble2;

      if (i == _kCFPlistBufferSize)
        {
          CFDataAppendBytes (obj, bytes, _kCFPlistBufferSize);
          i = 0;
        }
      if (!CFPlistStringSkipWhitespace (string)
          && string->cursor < string->limit)
        break;

      ch = *string->cursor++;
      nibble1 = getNibble (ch);
    }

  if (!CFPlistStringSkipWhitespace (string))
    {
      string->error = CFPlistCreateError (kCFPropertyListReadCorruptError,
          CFSTR (""));
    }
  if (ch == '>')
    {
      CFDataAppendBytes (obj, bytes, i);
    }
  else
    {
      CFRelease (obj);
      obj = NULL;
    }

  return obj;
}

static CFStringRef
CFOpenStepPlistParseString (CFAllocatorRef alloc, CFPlistString * string)
{
  UniChar ch;
  CFStringRef obj;

  if (string->error || !CFPlistStringSkipWhitespace (string))
    return NULL;

  obj = NULL;
  ch = *string->cursor;

  if (ch == '\"')
    {
      const UniChar *mark;
      CFIndex len;
      CFMutableStringRef tmp;

      string->cursor++;
      mark = string->cursor;
      tmp = NULL;

      while (string->cursor < string->limit)
        {
          ch = *string->cursor++;
          if (ch == '\"')
            {
              break;
            }
          else if (ch == '\\')
            {
              if (tmp == NULL)
                tmp = CFStringCreateMutable (alloc, 0);

              CFStringAppendCharacters (tmp, mark, string->cursor - mark);

              ch = *string->cursor++;
              /* FIXME */
              if (ch >= '0' && ch <= '9')
                {
                }
              else if (ch == 'u' || ch == 'U')
                {
                }
              else
                {
                }
            }
        }

      len = string->cursor - mark;
      if (tmp == NULL)
        {
          if (string->options == kCFPropertyListMutableContainersAndLeaves)
            {
              obj = CFStringCreateMutable (alloc, len);
              CFStringAppendCharacters ((CFMutableStringRef) obj,
                                        (const UniChar *) mark, len);
            }
          else
            {
              obj = CFStringCreateWithCharacters (alloc, mark, len);
            }
        }
      else
        {
          CFStringAppendCharacters (tmp, mark, len);

          if (string->options == kCFPropertyListMutableContainersAndLeaves)
            {
              obj = (CFStringRef) tmp;
            }
          else
            {
              obj = CFStringCreateCopy (alloc, tmp);
              CFRelease (tmp);
            }
        }
    }
  else if (!CFOpenStepPlistCharacterIsQuotable (ch))
    {
      UniChar *mark;

      mark = string->cursor;
      while (string->cursor < string->limit)
        {
          if (CFOpenStepPlistCharacterIsQuotable (ch))
            break;
          string->cursor++;
          ch = *string->cursor;
        }

      if (mark != string->cursor)
        {
          CFIndex len;

          len = string->cursor - mark;

          if (string->options == kCFPropertyListMutableContainersAndLeaves)
            {
              obj = CFStringCreateMutable (alloc, len);
              CFStringAppendCharacters ((CFMutableStringRef) obj,
                                        (const UniChar *) mark, len);
            }
          else
            {
              obj = CFStringCreateWithCharacters (alloc, mark, len);
            }
        }
    }

  return obj;
}

static CFPropertyListRef
CFOpenStepPlistParseObject (CFAllocatorRef alloc, CFPlistString * string)
{
  UniChar ch;
  CFPropertyListRef obj;

  /* If we have an error, return immediately. */
  if (string->error)
    return NULL;

  if (!CFPlistStringSkipWhitespace (string))
    return NULL;
  ch = *string->cursor++;

  if (ch == '{')
    {
      CFMutableDictionaryRef dict;
      CFStringRef key;
      CFPropertyListRef value;

      dict =
        CFDictionaryCreateMutable (alloc, 0,
                                   &kCFCopyStringDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);
      key = CFOpenStepPlistParseString (alloc, string);

      while (key)
        {
          if (!CFPlistStringSkipWhitespace (string) || *string->cursor != '=')
            {
              CFRelease (key);
              break;
            }

          string->cursor++;
          value = CFOpenStepPlistParseObject (alloc, string);
          if (value == NULL)
            {
              CFRelease (key);
              break;
            }

          CFDictionaryAddValue (dict, key, value);
          CFRelease (key);
          CFRelease (value);

          if (!CFPlistStringSkipWhitespace (string) || *string->cursor != ';')
            {
              key = NULL;
            }
          else
            {
              string->cursor++;
              key = CFOpenStepPlistParseString (alloc, string);
            }
        }

      if (*string->cursor == '}')
        {
          string->cursor++;
          obj = dict;
        }
      else
        {
          CFRelease (dict);
          obj = NULL;
        }
    }
  else if (ch == '(')
    {
      CFMutableArrayRef array;
      CFPropertyListRef value;

      array = CFArrayCreateMutable (alloc, 0, &kCFTypeArrayCallBacks);
      value = CFOpenStepPlistParseObject (alloc, string);

      while (value)
        {
          CFArrayAppendValue (array, value);
          CFRelease (value);

          if (!CFPlistStringSkipWhitespace (string) || *string->cursor != ',')
            {
              value = NULL;
            }
          else
            {
              string->cursor++;
              value = CFOpenStepPlistParseObject (alloc, string);
            }
        }

      if (*string->cursor == ')')
        {
          string->cursor++;
          obj = array;
        }
      else
        {
          CFRelease (array);
          obj = NULL;
        }
    }
  else if (ch == '<')
    {
      obj = CFOpenStepPlistParseData (alloc, string);
    }
  else
    {
      string->cursor--;
      obj = CFOpenStepPlistParseString (alloc, string);
    }

  return obj;
}

static void
CFPlistWriteStreamFlush (CFPlistWriteStream * stream)
{
  CFIndex ret;
  UInt8 *buf;

  /* If an error has already been set, return immediately.
   */
  if (stream->error)
    return;

  buf = stream->buffer;
  ret = CFWriteStreamWrite (stream->stream, buf, stream->cursor - buf);
  if (ret < 0)
    {
      CFErrorRef error;

      error = CFWriteStreamCopyError (stream->stream);
      if (error)
        {
          stream->error = error;
        }
      else
        {
          stream->error = CFPlistCreateError (kCFPropertyListWriteStreamError,
                                              CFSTR ("Unknown stream error encountered while trying to write property list."));
        }
    }
  else if (ret == 0)
    {
      stream->error = CFPlistCreateError (kCFPropertyListWriteStreamError,
                                          CFSTR ("Property list write could not be completed. Stream is full."));
    }

  stream->written += ret;
  stream->cursor = stream->buffer;
}

static void
CFPlistWriteStreamWrite (CFPlistWriteStream * stream, const UInt8 * buf,
                         CFIndex len)
{
  do
    {
      CFIndex writeLen;

      /* Flush buffer if needed */
      if (stream->cursor == stream->buffer + _kCFPlistBufferSize)
        CFPlistWriteStreamFlush (stream);

      /* If an error was set, return immediately.
       */
      if (stream->error != NULL)
        return;

      if (len > _kCFPlistBufferSize - (stream->cursor - stream->buffer))
        writeLen = _kCFPlistBufferSize - (stream->cursor - stream->buffer);
      else
        writeLen = len;
      GSMemoryCopy (stream->cursor, buf, writeLen);
      stream->cursor += writeLen;
      buf += writeLen;
      len -= writeLen;
    }
  while (len > 0);
}

static const UInt8 *indentString = (const UInt8 *) "\t\t\t\t\t\t\t\t";
#define _kCFPlistIndentStringLength 8

static void
CFPlistIndent (CFPlistWriteStream * stream, CFIndex lev)
{
  while (lev > _kCFPlistIndentStringLength)
    {
      CFPlistWriteStreamWrite (stream, indentString,
                               _kCFPlistIndentStringLength);
      lev -= _kCFPlistIndentStringLength;
    }
  CFPlistWriteStreamWrite (stream, indentString, lev);
}

static const UInt8 *base16 = (const UInt8 *) "0123456789ABCDEF";

static void
CFPlistWriteDataBase16 (CFDataRef data, CFPlistWriteStream * stream)
{
  CFIndex len;
  CFIndex i;
  const UInt8 *d;
  const UInt8 *dLimit;
  UInt8 buffer[_kCFPlistBufferSize];

  CFPlistWriteStreamFlush (stream);

  len = CFDataGetLength (data);
  if (len == 0)
    return;

  d = CFDataGetBytePtr (data);
  dLimit = d + len;
  i = 0;

  while (d < dLimit)
    {
      buffer[i++] = base16[((*d) >> 4)];
      buffer[i++] = base16[(*d) & 0xF];
      d++;

      if (i == _kCFPlistBufferSize)
        {
          CFPlistWriteStreamWrite (stream, buffer, _kCFPlistBufferSize);
          i = 0;
        }
    }
  CFPlistWriteStreamWrite (stream, buffer, i);
}

static void
CFPlistWriteDataBase64 (CFDataRef data, CFPlistWriteStream * stream)
{

}

static void
CFPlistWriteXMLString (CFStringRef str, CFPlistWriteStream * stream)
{
}

static void
CFXMLPlistWriteObject (CFPropertyListRef plist, CFPlistWriteStream * stream,
                       CFIndex lev)
{
  CFTypeID typeID;

  CFPlistIndent (stream, lev);
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

      CFPlistWriteStreamWrite (stream, (const UInt8 *) "<array>", 7);

      count = CFArrayGetCount ((CFArrayRef) plist);
      for (idx = 0; idx < count; ++idx)
        {
          CFPropertyListRef obj;

          obj = CFArrayGetValueAtIndex ((CFArrayRef) plist, idx);
          CFXMLPlistWriteObject (obj, stream, lev + 1);
        }
      CFPlistIndent (stream, lev);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "</array>", 8);
    }
  else if (typeID == CFBooleanGetTypeID ())
    {
      /* <true/> or <false/> */
      if (plist == kCFBooleanTrue)
        CFPlistWriteStreamWrite (stream, (const UInt8 *) "<true/>", 7);
      else if (plist == kCFBooleanFalse)
        CFPlistWriteStreamWrite (stream, (const UInt8 *) "<false/>", 8);
    }
  else if (typeID == CFDataGetTypeID ())
    {
      /* <data>Base64 Data</data> */
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "<data>", 6);
      CFPlistWriteDataBase64 ((CFDataRef) plist, stream);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "</data>", 7);
    }
  else if (typeID == CFDateGetTypeID ())
    {
      /* <date>%04d-%02d-%02dT%02d:%02d:%02dZ</date> */
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
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "<date>", 6);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) buffer, 20);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "</date>", 7);
    }
  else if (typeID == CFDictionaryGetTypeID ())
    {

    }
  else if (typeID == CFNumberGetTypeID ())
    {
      if (CFNumberIsFloatType ((CFNumberRef) plist))
        {
          CFPlistWriteStreamWrite (stream, (const UInt8 *) "<real>", 6);

          CFPlistWriteStreamWrite (stream, (const UInt8 *) "</real>", 7);
        }
      else
        {
          CFPlistWriteStreamWrite (stream, (const UInt8 *) "<integer>", 9);

          CFPlistWriteStreamWrite (stream, (const UInt8 *) "</integer>", 10);
        }
    }
  else if (typeID == CFStringGetTypeID ())
    {
      /* <string>String</string> */
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "<string>", 8);
      CFPlistWriteXMLString ((CFStringRef) plist, stream);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "</string>", 9);
    }
  else
    {
      return;
    }

  CFPlistWriteStreamWrite (stream, (const UInt8 *) "\n", 1);
}

static Boolean
CFOpenStepPlistStringHasQuotables (CFStringRef str)
{
  UniChar ch;
  UniChar *cursor;
  UniChar buffer[_kCFPlistBufferSize];
  CFIndex loc;
  CFIndex len;
  CFIndex read;

  loc = 0;
  len = CFStringGetLength (str);
  do
    {
      read = len > _kCFPlistBufferSize ? _kCFPlistBufferSize : len;
      CFStringGetCharacters (str, CFRangeMake (loc, read), buffer);
      cursor = buffer;
      do
        {
          ch = *cursor++;
          if (CFOpenStepPlistCharacterIsQuotable (ch))
            return true;
        }
      while (cursor < (buffer + read));

      loc += read;
      len -= read;
    }
  while (len > 0);

  return false;
}

static void
CFPlistWriteOpenStepString (CFStringRef str, CFPlistWriteStream * stream)
{
  /* An encoding parameter could be added to this function, but we write
     every string on human readable property lists (OpenStep and XML) in
     UTF-8 encoding, so this is not necessary.
   */
  UInt8 buffer[_kCFPlistBufferSize];
  CFIndex length;
  CFIndex loc;
  CFIndex read;
  CFIndex used;

  length = CFStringGetLength (str);
  loc = 0;
  if (length == 0)
    {
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "\"\"", 2);
    }
  else if (CFOpenStepPlistStringHasQuotables (str))
    {
      const UInt8 *mark;
      const UInt8 *cursor;
      UInt8 ch;

      CFPlistWriteStreamWrite (stream, (const UInt8 *) "\"", 1);

      do
        {
          read = CFStringGetBytes (str, CFRangeMake (loc, length),
                                   kCFStringEncodingUTF8, 0, false, buffer,
                                   _kCFPlistBufferSize, &used);
          mark = (const UInt8 *) buffer;
          cursor = (const UInt8 *) buffer;
          while ((cursor - mark) < read)
            {
              ch = *cursor++;
              if (ch == '\a' || ch == '\b' || ch == '\v' || ch == '\f'
                  || ch == '\"')
                {
                  CFPlistWriteStreamWrite (stream, mark, cursor - mark);
                  CFPlistWriteStreamWrite (stream, (const UInt8 *) "\\", 1);
                  if (ch == '\a')
                    CFPlistWriteStreamWrite (stream, (const UInt8 *) "a", 1);
                  else if (ch == '\b')
                    CFPlistWriteStreamWrite (stream, (const UInt8 *) "b", 1);
                  else if (ch == '\v')
                    CFPlistWriteStreamWrite (stream, (const UInt8 *) "v", 1);
                  else if (ch == '\f')
                    CFPlistWriteStreamWrite (stream, (const UInt8 *) "f", 1);
                  else if (ch == '\"')
                    CFPlistWriteStreamWrite (stream, (const UInt8 *) "\"", 1);
                }
            }
          CFPlistWriteStreamWrite (stream, mark, cursor - mark);
          loc += read;
          length -= read;
        }
      while (length > 0);

      CFPlistWriteStreamWrite (stream, (const UInt8 *) "\"", 1);
    }
  else
    {
      do
        {
          read = CFStringGetBytes (str, CFRangeMake (loc, length),
                                   kCFStringEncodingUTF8, 0, false, buffer,
                                   _kCFPlistBufferSize, &used);
          CFPlistWriteStreamWrite (stream, (const UInt8 *) buffer, used);
          loc += read;
          length -= read;
        }
      while (length > 0);
    }
}

static void
CFOpenStepPlistWriteObject (CFPropertyListRef plist,
                            CFPlistWriteStream * stream, CFIndex lev)
{
  CFTypeID typeID;

  if (stream->error)
    return;

  typeID = CFGetTypeID (plist);
  if (typeID == CFArrayGetTypeID ())
    {
      /* (
           Object1,
           Object2,
           ...
         )
       */
      CFIndex i;
      CFIndex count;

      CFPlistWriteStreamWrite (stream, (const UInt8 *) "(\n", 2);

      i = 0;
      count = CFArrayGetCount ((CFArrayRef) plist);
      while (i < count)
        {
          CFPropertyListRef obj;

          obj = CFArrayGetValueAtIndex ((CFArrayRef) plist, i++);
          CFPlistIndent (stream, lev + 1);
          CFOpenStepPlistWriteObject (obj, stream, 0);
          if (i < count)
            CFPlistWriteStreamWrite (stream, (const UInt8 *) ",\n", 2);
        }

      CFPlistWriteStreamWrite (stream, (const UInt8 *) "\n", 1);
      CFPlistIndent (stream, lev);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) ")", 1);
    }
  else if (typeID == CFDataGetTypeID ())
    {
      /* <Hexadecimal Data> */
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "<", 1);
      CFPlistWriteDataBase16 ((CFDataRef) plist, stream);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) ">", 1);
    }
  else if (typeID == CFDictionaryGetTypeID ())
    {
      /* {
           Key1 = Value1;
           Key2 = Value2;
           ...
         }
       */
      CFIndex count;
      CFIndex i;
      const void **keys;

      CFPlistWriteStreamWrite (stream, (const UInt8 *) "{\n", 2);

      count = CFDictionaryGetCount ((CFDictionaryRef) plist);
      keys = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                  sizeof (void *) * count, 0);
      CFDictionaryGetKeysAndValues ((CFDictionaryRef) plist, keys, NULL);
      GSCArrayQuickSort (keys, count, (CFComparatorFunction) CFStringCompare,
                         NULL);
      for (i = 0; i < count; ++i)
        {
          CFPropertyListRef obj;

          CFPlistIndent (stream, lev + 1);
          CFPlistWriteOpenStepString ((CFStringRef) keys[i], stream);

          CFPlistWriteStreamWrite (stream, (const UInt8 *) " = ", 3);

          obj = CFDictionaryGetValue ((CFDictionaryRef) plist, keys[i]);
          CFOpenStepPlistWriteObject (obj, stream, lev + 1);

          CFPlistWriteStreamWrite (stream, (const UInt8 *) ";\n", 2);
        }

      CFPlistIndent (stream, lev);
      CFPlistWriteStreamWrite (stream, (const UInt8 *) "}\n", 2);
    }
  else if (typeID == CFStringGetTypeID ())
    {
      /* String or "Quotable String" */
      CFPlistWriteOpenStepString ((CFStringRef) plist, stream);
    }
  else
    {
      stream->error = CFPlistCreateError (0, CFSTR ("Invalid property list item encountered while writing OpenStep format."));
    }
}

#if 0
static CFPropertyListRef
CFBinaryPlistCreate (CFAllocatorRef alloc, CFDataRef data,
                     CFOptionFlags opts, CFErrorRef * err)
{
  return NULL;
}
#endif

static CFPropertyListRef
CFXMLPlistCreate (CFAllocatorRef alloc, CFPlistString * stream)
{
  return NULL;
}

static const UInt8 *_kCFBinaryPlistHeader = (const UInt8 *) "bplist00";
static const CFIndex _kCFBinaryPlistHeaderLength =
  sizeof (_kCFBinaryPlistHeader) - 1;

static void
CFBinaryPlistWrite (CFPropertyListRef plist, CFPlistWriteStream * stream)
{
}

static const UInt8 *_kCFXMLPlistHeader = (const UInt8 *)
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE plist PUBLIC \"-//GNUstep//DTD plist 0.9//EN\" \"http://www.gnustep.org/plist-0_9.dtd\">\n"
  "<plist version=\"0.9\">\n";
static const CFIndex _kCFXMLPlistHeaderLength =
  sizeof (_kCFXMLPlistHeader) - 1;

static void
CFXMLPlistWrite (CFPropertyListRef plist, CFPlistWriteStream * stream)
{
  CFPlistWriteStreamWrite (stream, _kCFXMLPlistHeader,
                           _kCFXMLPlistHeaderLength);

  CFXMLPlistWriteObject (plist, stream, 0);

  CFPlistWriteStreamWrite (stream, (const UInt8 *) "</plist>\n", 9);
  CFPlistWriteStreamFlush (stream);
}

static const UInt8 UTF8BOM[3] = { 0xEF, 0xBB, 0xBF };

static void
CFOpenStepPlistWrite (CFPropertyListRef plist, CFPlistWriteStream * stream)
{
  /* We'll include a UTF-8 BOM to OpenStep formatted property lists.
   */
  CFPlistWriteStreamWrite (stream, UTF8BOM, 3);
  CFOpenStepPlistWriteObject (plist, stream, 0);
  CFPlistWriteStreamFlush (stream);
}

static CFStringEncoding
CFPlistGetEncoding (CFDataRef data)
{
  return kCFStringEncodingUTF8; /* FIXME */
}

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                              CFOptionFlags opts, CFPropertyListFormat * fmt,
                              CFErrorRef * err)
{
  CFPropertyListRef result;
  CFStringEncoding enc;
  CFIndex count;
  CFIndex dataLen;
  UniChar *start;
  UniChar *tmp;
  UniChar buffer[_kCFPlistBufferSize];
  const UInt8 *bytes;
  CFPlistString string;

  enc = CFPlistGetEncoding (data);
  if (enc == kCFStringEncodingInvalidId)
    {
      if (err)
        *err = NULL;
#if 0
      *err = CFPlistCreateError (kCFPropertyListReadCorruptError,
                                 CFSTR
                                 ("Property list is in an unknown string encoding."));
#endif
      return NULL;
    }


  bytes = CFDataGetBytePtr (data);
  dataLen = CFDataGetLength (data);
  start = buffer;
  tmp = buffer;

  count = GSUnicodeFromEncoding (&tmp, start + _kCFPlistBufferSize, enc,
                                 &bytes, bytes + dataLen, 0);
  if (count < 0)
    return NULL;
  
  if (count > _kCFPlistBufferSize)
    {
      start = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                   count * sizeof (UniChar), 0);
      if (start == NULL)
        return NULL;
      tmp = start;
      bytes = CFDataGetBytePtr (data);
      GSUnicodeFromEncoding (&tmp, start + count, enc, &bytes,
                             bytes + dataLen, 0);
    }

  string.buffer = start;
  string.options = opts;
  string.error = NULL;
  string.unique = NULL;
  string.cursor = string.buffer;
  string.limit = string.buffer + count;

  result = CFXMLPlistCreate (alloc, &string);
  if (result)
    {
      if (fmt)
        *fmt = kCFPropertyListXMLFormat_v1_0;
      return result;
    }

  result = CFOpenStepPlistParseObject (alloc, &string);
  if (result)
    {
      if (fmt)
        *fmt = kCFPropertyListOpenStepFormat;
      return result;
    }

  if (start != buffer)
    CFAllocatorDeallocate (kCFAllocatorSystemDefault, tmp);

  return NULL;
}

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat * fmt, CFErrorRef * err)
{
  CFMutableDataRef data;
  UInt8 buffer[_kCFPlistBufferSize];
  CFIndex read;
  CFPropertyListRef result;

  /* We'll read the stream completely into a CFDataRef object for processing.
   */
  data = CFDataCreateMutable (kCFAllocatorSystemDefault, len);
  do
    {
      CFIndex toRead;

      if (len == 0 || len > _kCFPlistBufferSize)
        toRead = _kCFPlistBufferSize;
      else
        toRead = len;

      read = CFReadStreamRead (stream, buffer, toRead);
      if (read > 0)
        CFDataAppendBytes (data, buffer, read);
      len -= read;
    }
  while (read > 0);

  if (read < 0)
    {
      CFErrorRef streamError;

      streamError = CFReadStreamCopyError (stream);
      if (streamError && err)
        *err = streamError;

      return NULL;
    }

  result = CFPropertyListCreateWithData (alloc, data, opts, fmt, err);
  CFRelease (data);

  return result;
}

CFDataRef
CFPropertyListCreateData (CFAllocatorRef alloc, CFPropertyListRef plist,
                          CFPropertyListFormat fmt, CFOptionFlags opts,
                          CFErrorRef * err)
{
  CFWriteStreamRef stream;
  CFIndex written;
  CFDataRef data;

  stream = CFWriteStreamCreateWithAllocatedBuffers (alloc, alloc);
  CFWriteStreamOpen (stream);
  written = CFPropertyListWrite (plist, stream, fmt, opts, err);
  data = NULL;
  if (written > 0)
    data = CFWriteStreamCopyProperty (stream, kCFStreamPropertyDataWritten);
  CFWriteStreamClose (stream);
  CFRelease (stream);

  return data;
}

CFIndex
CFPropertyListWrite (CFPropertyListRef plist, CFWriteStreamRef stream,
                     CFPropertyListFormat fmt, CFOptionFlags opts,
                     CFErrorRef * err)
{
  CFPlistWriteStream plStream;

  plStream.stream = stream;
  plStream.options = opts;
  plStream.error = NULL;
  plStream.written = 0;
  plStream.cursor = plStream.buffer;

  if (fmt == kCFPropertyListOpenStepFormat)
    CFOpenStepPlistWrite (plist, &plStream);
  else if (fmt == kCFPropertyListXMLFormat_v1_0)
    CFXMLPlistWrite (plist, &plStream);
  else if (fmt == kCFPropertyListBinaryFormat_v1_0)
    CFBinaryPlistWrite (plist, &plStream);

  if (plStream.error)
    {
      if (err)
        *err = plStream.error;
      else
        CFRelease (plStream.error);
      return 0;
    }

  return plStream.written;
}

/* The following functions are marked as obsolete as of Mac OS X 10.6.  They
 * will be implemented as wrappers around the new functions.
 */
CFDataRef
CFPropertyListCreateXMLData (CFAllocatorRef alloc, CFPropertyListRef plist)
{
  return CFPropertyListCreateData (alloc, plist,
                                   kCFPropertyListXMLFormat_v1_0, 0, NULL);
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

  plist =
    CFPropertyListCreateWithStream (alloc, stream, len, opts, fmt, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }

  return plist;
}
