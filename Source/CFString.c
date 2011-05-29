/* CFString.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: May, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redisibute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is disibuted in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <unicode/unorm.h>
#include <unicode/urep.h>
#include <unicode/ustring.h>
#include <unicode/utrans.h>
#include <unicode/ustdio.h>

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"

#include "CoreFoundation/ForFoundationOnly.h"

/* CFString has two possible internal encodings:
     * UTF-16 (preferable)
     * ASCII
   If the encoding is not one of the two listed above, it will be converted
   to UTF-16 any character is not ASCII.
*/

struct __CFString
{
  CFRuntimeBase  _parent;
  void          *_contents;
  CFIndex        _count;
  CFHashCode     _hash;
  CFAllocatorRef _deallocator;
};

struct __CFMutableString
{
  CFRuntimeBase  _parent;
  UniChar       *_contents;
  CFIndex        _count;
  CFHashCode     _hash;
  CFAllocatorRef _allocator;
  CFIndex        _capacity;
};

static CFTypeID _kCFStringTypeID;
static UFILE *_ustdout;

/* These are some masks to access the data in CFRuntimeBase's _flags.info
   field. */
enum
{
  _kCFStringIsMutable = (1<<0),
  _kCFStringIsWide    = (1<<1),
  _kCFStringIsOwned   = (1<<2),
  _kCFStringIsInline  = (1<<3),
  _kCFStringHasLengthByte = (1<<4), // This is used for Pascal strings
  _kCFStringHasNullByte = (1<<5)
};

static inline Boolean
CFStringIsMutable (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsMutable ? true : false;
}

static inline Boolean
CFStringIsWide (CFStringRef str)
{
  return ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsWide ? true : false;
}

static inline Boolean
CFStringIsOwned (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsOwned ? true : false;
}

static inline Boolean
CFStringIsInline (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsInline ? true : false;
}

static inline Boolean
CFStringHasLengthByte (CFStringRef str)
{
  return ((CFRuntimeBase *)str)->_flags.info & _kCFStringHasLengthByte ?
    true : false;
}

static inline Boolean
CFStringHasNullByte (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringHasNullByte ? true : false;
}

static inline void
CFStringSetMutable (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsMutable;
}

static inline void
CFStringSetWide (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsWide;
}

static inline void
CFStringSetOwned (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsOwned;
}

static inline void
CFStringSetInline (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsInline;
}

static inline void
CFStringSetHasLengthByte (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringHasLengthByte;
}

static inline void
CFStringSetHasNullByte (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringHasNullByte;
}



static void CFStringFinalize (CFTypeRef cf)
{
  CFStringRef str = (CFStringRef)cf;
  
  if (CFStringIsOwned(str) && !CFStringIsInline(str))
    CFAllocatorDeallocate (str->_deallocator, str->_contents);
}

static Boolean CFStringEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return CFStringCompare (cf1, cf2, 0) == 0 ? true : false;
}

static CFHashCode CFStringHash (CFTypeRef cf)
{
  CFHashCode ret;
  CFIndex len;
  
  CFStringRef str = (CFStringRef)cf;
  if (str->_hash)
    return str->_hash;
  
  /* This must match the NSString hash algorithm. */
  ret = 0;
  len = str->_count;
  if (len > 0)
    {
      register CFIndex idx = 0;
      register const UniChar *p = str->_contents;
      register const CFIndex len = str->_count;
      UChar32 c;
      
      /* We'll use the _UNSAFE variant of the U*_NEXT macros because
         all bytes stored by CFString are guaranteed to be valid. */
      if (CFStringIsWide(str)) // UTF-16
        {
          U16_NEXT_UNSAFE(p, idx, c);
          while (idx < len)
            {
              ret = (ret << 5) + ret + c;
              U16_NEXT_UNSAFE(p, idx, c);
            }
        }
      else // UTF-8
        {
          U8_NEXT_UNSAFE(p, idx, c);
          while (idx < len)
            {
              ret = (ret << 5) + ret + c;
              U8_NEXT_UNSAFE(p, idx, c);
            }
        }
      
      ret &= 0x0fffffff;
      if (ret == 0)
        {
          ret = 0x0fffffff;
        }
    }
  else
    {
      ret = 0x0ffffffe;
    }
  ((struct __CFString *)str)->_hash = ret;

  return str->_hash;
}

static CFStringRef
CFStringCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFStringCreateCopy(CFGetAllocator(cf), cf);
}

static const CFRuntimeClass CFStringClass =
{
  0,
  "CFString",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFStringCreateCopy,
  CFStringFinalize,
  CFStringEqual,
  CFStringHash,
  CFStringCopyFormattingDesc,
  NULL
};

void CFStringInitialize (void)
{
  _kCFStringTypeID = _CFRuntimeRegisterClass (&CFStringClass);
  _ustdout = u_finit (stdout, NULL, NULL);
}



void
CFShow (CFTypeRef obj)
{
  CFStringRef str = CFCopyDescription (obj);
  char *out;
  
  out = CFStringIsWide(str) ? "%S" : "%s";
  
  u_fprintf (_ustdout, out, str->_contents);
}

void
CFShowStr (CFStringRef s)
{
  fprintf (stdout, "Length %d\n", (int)s->_count);
  fprintf (stdout, "IsWide %d\n", CFStringIsWide(s));
  fprintf (stdout, "HasLengthByte %d\n", CFStringHasLengthByte(s));
  fprintf (stdout, "HasNullByte %d\n", CFStringHasNullByte(s));
  fprintf (stdout, "InlineContents %d\n", CFStringIsInline(s));
  fprintf (stdout, "Allocator %p\n", CFGetAllocator(s)); // FIXME: allocator name
  fprintf (stdout, "Mutable %d\n", CFStringIsMutable(s));
  u_fprintf (_ustdout, CFStringIsWide(s) ? "%S\n" : "%s\n", s->_contents);
}

CFTypeID
CFStringGetTypeID (void)
{
  return _kCFStringTypeID;
}



/* The CFString create function will return an inlined string whenever
   possible.  The contents may or may not be inlined if a NoCopy function
   is called. With this in mind, CFStringCreateWithBytes will return a
   string with the content inlined and CFStringCreateWithBytesNoCopy will not
   have inlined content if, and only if, the input bytes are in one of the
   internal encodings. */

CFStringRef
CFStringCreateWithBytes (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding,
  Boolean isExternalRepresentation)
{
  struct __CFString *new;
  CFIndex strSize;
  CFIndex size;
  CFVarWidthCharBuffer buffer;
  
  buffer.allocator = alloc;
  if (!__CFStringDecodeByteStream3 (bytes, numBytes, encoding, false, &buffer,
      NULL, 0))
    return NULL;
  
  /* We'll inline the string buffer if __CFStringDecodeByteStream3() has not
     already allocated a buffer for us. */
  if (buffer.shouldFreeChars)
    strSize = 0;
  else
    strSize =
      (buffer.numChars + 1) * (buffer.isASCII ? sizeof(char) : sizeof(UniChar));
  
  size = sizeof(struct __CFString) + strSize - sizeof(struct __CFRuntimeBase);
  new = (struct __CFString *)
    _CFRuntimeCreateInstance (alloc, _kCFStringTypeID, size, NULL);
  
  if (buffer.isASCII)
    {
      memcpy (&(new[1]), buffer.chars.c, buffer.numChars);
      new->_contents = &(new[1]);
      CFStringSetInline((CFStringRef)new);
      
      if (buffer.shouldFreeChars == true)
        CFAllocatorDeallocate (buffer.allocator, buffer.chars.c);
    }
  else
    {
      u_memcpy ((UChar*)&(new[1]), buffer.chars.u, buffer.numChars);
      new->_contents = &(new[1]);
      CFStringSetInline((CFStringRef)new);
      CFStringSetWide((CFStringRef)new);
      
      if (buffer.shouldFreeChars == true)
        CFAllocatorDeallocate (buffer.allocator, buffer.chars.c);
    }
  
  CFStringSetHasNullByte((CFStringRef)new);
  new->_count = buffer.numChars;
  new->_deallocator = alloc;
  
  return (CFStringRef)new;
}

CFStringRef
CFStringCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding,
  Boolean isExternalRepresentation, CFAllocatorRef contentsDeallocator)
{
  return NULL;
}

CFStringRef
CFStringCreateByCombiningStrings (CFAllocatorRef alloc, CFArrayRef theArray,
  CFStringRef separatorString)
{
  return NULL;
}

CFStringRef
CFStringCreateCopy (CFAllocatorRef alloc, CFStringRef str)
{
  CFStringRef new;
  CFStringEncoding enc;
  
  if (alloc == NULL)
    alloc = CFAllocatorGetDefault ();
  
  if (CFGetAllocator(str) == alloc && !CFStringIsMutable(str))
    return CFRetain (str);
  
  enc = CFStringIsWide(str) ? kCFStringEncodingUTF16 : kCFStringEncodingASCII;
  new =
    CFStringCreateWithBytes (alloc, str->_contents, str->_count, enc, false);
  
  return new;
}

CFStringRef
CFStringCreateWithFileSystemRepresentation (CFAllocatorRef alloc,
  const char *buffer)
{
  return NULL;
}

CFStringRef
CFStringCreateFromExternalRepresentation (CFAllocatorRef alloc, CFDataRef data,
  CFStringEncoding encoding)
{
  const UInt8 *bytes = CFDataGetBytePtr (data);
  CFIndex numBytes = CFDataGetLength (data);
  return CFStringCreateWithBytes (alloc, bytes, numBytes, encoding, true);
}

CFStringRef
CFStringCreateWithCharacters (CFAllocatorRef alloc, const UniChar *chars,
  CFIndex numChars)
{
  return CFStringCreateWithBytes (alloc, (const UInt8*)chars,
    numChars * sizeof(UniChar), kCFStringEncodingUnicode, false);
}

CFStringRef
CFStringCreateWithCharactersNoCopy (CFAllocatorRef alloc, const UniChar *chars,
  CFIndex numChars, CFAllocatorRef contentsDeallocator)
{
  return CFStringCreateWithBytesNoCopy (alloc, (const UInt8*)chars,
    numChars * sizeof(UniChar), kCFStringEncodingUnicode, false,
    contentsDeallocator);
}

CFStringRef
CFStringCreateWithCString (CFAllocatorRef alloc, const char *cStr,
  CFStringEncoding encoding)
{
  CFIndex len = strlen(cStr);
  return CFStringCreateWithBytes (alloc, (const UInt8*)cStr, len, encoding,
    false);
}

CFStringRef
CFStringCreateWithCStringNoCopy (CFAllocatorRef alloc, const char *cStr,
  CFStringEncoding encoding, CFAllocatorRef contentsDeallocator)
{
  CFIndex len = strlen(cStr);
  return CFStringCreateWithBytesNoCopy (alloc, (const UInt8*)cStr, len,
    encoding, false, contentsDeallocator);
}

CFStringRef
CFStringCreateWithFormat (CFAllocatorRef alloc, CFDictionaryRef formatOptions,
  CFStringRef format, ...)
{
  CFStringRef result;
  va_list args;
  
  va_start (args, format);
  result =
    CFStringCreateWithFormatAndArguments (alloc, formatOptions, format, args);
  va_end (args);
  
  return result;
}

CFStringRef
CFStringCreateWithFormatAndArguments (CFAllocatorRef alloc,
  CFDictionaryRef formatOptions, CFStringRef format, va_list arguments)
{
  return NULL;
}

CFStringRef
CFStringCreateWithSubstring (CFAllocatorRef alloc, CFStringRef str, CFRange range)
{
  return NULL;
}

CFDataRef
CFStringCreateExternalRepresentation (CFAllocatorRef alloc,
  CFStringRef str, CFStringEncoding encoding, UInt8 lossByte)
{
  UInt8 *buffer;
  CFRange range;
  CFIndex numBytes;
  CFIndex strLen;
  CFIndex len;
  CFIndex usedLen = 0;
  
  strLen = CFStringGetLength (str);
  range = CFRangeMake (0, strLen);
  len = strLen + 1; // include space for a NULL byte.
  
  buffer = CFAllocatorAllocate (alloc, len, 0);
  
  numBytes = CFStringGetBytes (str, range, encoding, lossByte,
    true, buffer, len, &usedLen);
  
  if (numBytes == 0)
    return NULL;
  
  return CFDataCreateWithBytesNoCopy (alloc, buffer, usedLen, alloc);
}

CFArrayRef
CFStringCreateArrayBySeparatingStrings (CFAllocatorRef alloc,
  CFStringRef str, CFStringRef separatorString)
{
  return NULL;
}

const UniChar *
CFStringGetCharactersPtr (CFStringRef str)
{
  return CFStringIsWide(str) ? str->_contents : NULL;
}

const char *
CFStringGetCStringPtr (CFStringRef str, CFStringEncoding enc)
{
  return (!CFStringIsWide(str) && __CFStringEncodingIsSupersetOfASCII(enc))
    ? str->_contents : NULL;
}

CFIndex
CFStringGetBytes (CFStringRef str, CFRange range,
  CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation,
  UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen)
{
  return __CFStringEncodeByteStream (str, range.location, range.length,
    isExternalRepresentation, encoding, (char)lossByte, buffer, maxBufLen,
    usedBufLen);
}

void
CFStringGetCharacters (CFStringRef str, CFRange range, UniChar *buffer)
{
  __CFStringEncodeByteStream (str, range.location, range.length,
    false, kCFStringEncodingUTF16, '?', (UInt8*)buffer, range.length, NULL);
}

Boolean
CFStringGetCString (CFStringRef str, char *buffer, CFIndex bufferSize,
  CFStringEncoding encoding)
{
  CFIndex len = CFStringGetLength (str);
  CFIndex used;
  
  if (__CFStringEncodeByteStream (str, 0, len, false, encoding, '?',
      (UInt8*)buffer, bufferSize, &used) == len)
    {
      buffer[used] = '\0';
      return true;
    }
  
  return false;
}

Boolean
CFStringGetFileSystemRepresentation (CFStringRef string, char *buffer,
  CFIndex maxBufLen)
{
  return false;
}


UniChar
CFStringGetCharacterAtIndex (CFStringRef str, CFIndex idx)
{
  return CFStringIsWide(str) ? ((UniChar*)str->_contents)[idx] :
    ((char*)str->_contents)[idx];
}

CFIndex
CFStringGetLength (CFStringRef str)
{
  return str->_count;
}

CFRange
CFStringGetRangeOfComposedCharactersAtIndex (CFStringRef str,
  CFIndex theIndex)
{
  return CFRangeMake (0, 0);
}

UTF32Char
CFStringGetLongCharacterForSurrogatePair (UniChar surrogateHigh,
  UniChar surrogateLow)
{
  return (UTF32Char)U16_GET_SUPPLEMENTARY(surrogateHigh, surrogateLow);
}

Boolean
CFStringGetSurrogatePairForLongCharacter (UTF32Char character,
  UniChar *surrogates)
{
  if (character > 0x10000)
    return false;
  
  surrogates[0] = U16_LEAD(character);
  surrogates[1] = U16_TRAIL(character);
  
  return true;
}

Boolean
CFStringIsSurrogateHighCharacter (UniChar character)
{
  return (Boolean)U16_IS_LEAD(character);
}

Boolean
CFStringIsSurrogateLowCharacter (UniChar character)
{
  return (Boolean)U16_IS_TRAIL(character);
}



/* CFMutableString functions start here. 
   
   All mutable string instances are Unicode.  This makes it
   easier to use the ICU functions. */

/* This function is used to grow the size of a CFMutableString buffer.
   The string's buffer will grow to (newCapacity * sizeof(UniChar)).
   On return, oldContentBuffer will point to the old data.  If this value
   is not provided, the old content buffer is freed and data will be lost. */
static Boolean
CFStringCheckCapacityAndGrow (CFMutableStringRef str, CFIndex newCapacity,
  void **oldContentBuffer)
{
  struct __CFMutableString *mStr = (struct __CFMutableString *)str;
  
  if (mStr->_capacity >= newCapacity)
    return false;
  
  if (oldContentBuffer)
    *oldContentBuffer = mStr->_contents;
  else
    CFAllocatorDeallocate (mStr->_allocator, mStr->_contents);
  
  mStr->_contents = CFAllocatorAllocate (mStr->_allocator,
    (newCapacity * sizeof(UniChar)), 0);
  if (mStr->_contents == NULL)
    {
      mStr->_capacity = 0;
      return false;
    }
  mStr->_capacity = newCapacity;
  mStr->_hash = 0;
  
  return true;
}

/* This function will replace characters in a specified range with the
   characters in the text parameter of textLen length.  If range.length is
   0, this function will insert the text at the specified location.  If
   text is NULL, this function will delete the text in range (textLen is
   disregarded). */
static inline void
CFStringReplace_internal (CFMutableStringRef str, CFRange range,
  UniChar *text, CFIndex textLen)
{
  
}

#define DEFAULT_STRING_CAPACITY 16

#define CFSTRING_INIT_MUTABLE(str) do \
{ \
  ((CFRuntimeBase *)str)->_flags.info = \
    0xFF & (_kCFStringIsMutable | _kCFStringIsWide | _kCFStringHasNullByte); \
} while(0)

CFMutableStringRef
CFStringCreateMutable (CFAllocatorRef alloc, CFIndex maxLength)
{
  struct __CFMutableString *new;
  
  new = (struct __CFMutableString *)_CFRuntimeCreateInstance (alloc,
    _kCFStringTypeID,
    sizeof(struct __CFMutableString) - sizeof (CFRuntimeBase),
    NULL);
  
  new->_capacity =
    maxLength < DEFAULT_STRING_CAPACITY ? DEFAULT_STRING_CAPACITY : maxLength;
  new->_allocator = alloc;
  new->_contents = CFAllocatorAllocate (new->_allocator,
    new->_capacity * sizeof(UniChar), 0);
  
  CFSTRING_INIT_MUTABLE(new);
  
  return (CFMutableStringRef)new;
}

CFMutableStringRef
CFStringCreateMutableCopy (CFAllocatorRef alloc, CFIndex maxLength,
  CFStringRef str)
{
  struct __CFMutableString *new;
  UniChar *text;
  CFIndex textLen;
  
  new = (struct __CFMutableString *)CFStringCreateMutable (alloc, maxLength);
  
  textLen = CFStringGetLength (str);
  text = (UniChar*)CFStringGetCharactersPtr (str);
  if (text == NULL)
    {
      text = alloca ((textLen + 1) * sizeof(UniChar));
      CFStringGetCharacters (str, CFRangeMake(0, textLen), text);
      text[textLen] = 0;
    }
  CFStringReplace_internal ((CFMutableStringRef)new, CFRangeMake(0, 0),
    text, textLen);
  
  CFSTRING_INIT_MUTABLE(new);
  
  return (CFMutableStringRef)new;
}

CFMutableStringRef
CFStringCreateMutableWithExternalCharactersNoCopy (CFAllocatorRef alloc,
  UniChar *chars, CFIndex numChars, CFIndex capacity,
  CFAllocatorRef externalCharactersAllocator)
{
  return NULL;
}

void
CFStringSetExternalCharactersNoCopy (CFMutableStringRef str, UniChar *chars,
  CFIndex length, CFIndex capacity)
{
  return;
}

CFIndex
CFStringFindAndReplace (CFMutableStringRef str, CFStringRef stringToFind,
  CFStringRef replacementString, CFRange rangeToSearch,
  CFOptionFlags compareOptions)
{
  return 0;
}

void
CFStringAppend (CFMutableStringRef str, CFStringRef appendedString)
{
  return;
}

void
CFStringAppendCharacters (CFMutableStringRef str,
  const UniChar *chars, CFIndex numChars)
{
  return;
}

void
CFStringAppendCString (CFMutableStringRef str, const char *cStr,
  CFStringEncoding encoding)
{
  return;
}

void
CFStringAppendFormat (CFMutableStringRef str,
  CFDictionaryRef formatOptions, CFStringRef format, ...)
{
  va_list args;
  va_start (args, format);
  CFStringAppendFormatAndArguments (str, formatOptions, format, args);
  va_end (args);
}

void
CFStringAppendFormatAndArguments (CFMutableStringRef str,
  CFDictionaryRef formatOptions, CFStringRef format, va_list arguments)
{
  return;
}

void
CFStringDelete (CFMutableStringRef str, CFRange range)
{
  CFStringReplace_internal (str, range, NULL, 0);
}

void
CFStringInsert (CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr)
{
  UniChar *text;
  CFIndex textLen;
  
  textLen = CFStringGetLength (insertedStr);
  text = (UniChar*)CFStringGetCharactersPtr (insertedStr);
  if (text == NULL)
    {
      text = alloca (textLen * sizeof(UniChar));
      CFStringGetCharacters (str, CFRangeMake(0, textLen), text);
    }
  CFStringReplace_internal (str, CFRangeMake(idx, 0), text, textLen);
}

void
CFStringPad (CFMutableStringRef str, CFStringRef padString,
  CFIndex length, CFIndex indexIntoPad)
{
  return;
}

void
CFStringReplace (CFMutableStringRef str, CFRange range,
  CFStringRef replacement)
{
  UniChar *text;
  CFIndex textLen;
  
  textLen = CFStringGetLength (replacement);
  text = (UniChar*)CFStringGetCharactersPtr (replacement);
  if (text == NULL)
    {
      text = alloca (textLen * sizeof(UniChar));
      CFStringGetCharacters (str, CFRangeMake(0, textLen), text);
    }
  CFStringReplace_internal (str, range, text, textLen);
}

void
CFStringReplaceAll (CFMutableStringRef theString, CFStringRef replacement)
{
}

void
CFStringTrim (CFMutableStringRef str, CFStringRef trimString)
{
  return;
}

void
CFStringTrimWhitespace (CFMutableStringRef str)
{
  return;
}

enum
{
  _kCFStringCapitalize,
  _kCFStringLowercase,
  _kCFStringUppercase,
  _kCFStringFold
};

static inline void
CFStringCaseMap (CFMutableStringRef str, CFLocaleRef locale,
  CFOptionFlags flags, CFIndex op)
{
  char *localeID = NULL; // FIXME
  const UniChar *oldContents;
  CFIndex oldContentsLength;
  CFIndex newLength;
  int32_t optFlags;
  UErrorCode err = U_ZERO_ERROR;
  struct __CFMutableString *mStr = (struct __CFMutableString *)str;
  
  oldContents = CFStringGetCharactersPtr (str);
  oldContentsLength = CFStringGetLength (str);
  
  /* Loops a maximum of 2 times, and should never loop more than that.  If
     it does have to go through the loop a 3rd time something is wrong
     and this whole thing will blow up. */
  do
    {
      switch (op)
        {
          case _kCFStringCapitalize:
            newLength = u_strToTitle (mStr->_contents, mStr->_capacity,
              oldContents, oldContentsLength, NULL, localeID, &err);
            break;
          case _kCFStringLowercase:
            newLength = u_strToLower (mStr->_contents, mStr->_capacity,
              oldContents, oldContentsLength, localeID, &err);
          case _kCFStringUppercase:
            newLength = u_strToUpper (mStr->_contents, mStr->_capacity,
              oldContents, oldContentsLength, localeID, &err);
          case _kCFStringFold:
            optFlags = 0; // FIXME
            newLength = u_strFoldCase (mStr->_contents, mStr->_capacity,
              oldContents, oldContentsLength, optFlags, &err);
          default:
            return;
        }
    } while (err == U_BUFFER_OVERFLOW_ERROR
        && CFStringCheckCapacityAndGrow(str, newLength, (void**)&oldContents));
  
  mStr->_count = newLength;
  
  if (oldContents != mStr->_contents)
    CFAllocatorDeallocate (mStr->_allocator, (void*)oldContents);
}

void
CFStringCapitalize (CFMutableStringRef str, CFLocaleRef locale)
{
  CFStringCaseMap (str, locale, 0, _kCFStringCapitalize);
}

void
CFStringLowercase (CFMutableStringRef str, CFLocaleRef locale)
{
  CFStringCaseMap (str, locale, 0, _kCFStringCapitalize);
}

void
CFStringUppercase (CFMutableStringRef str, CFLocaleRef locale)
{
  CFStringCaseMap (str, locale, 0, _kCFStringUppercase);
}

void
CFStringFold (CFMutableStringRef str, CFOptionFlags flags, CFLocaleRef locale)
{
  CFStringCaseMap (str, locale, flags, _kCFStringFold);
}

static inline UNormalizationMode
CFToICUNormalization (CFStringNormalizationForm form)
{
  switch (form)
    {
      case kCFStringNormalizationFormD:
        return UNORM_NFD;
      case kCFStringNormalizationFormKD:
        return UNORM_NFKD;
      case kCFStringNormalizationFormC:
        return UNORM_NFC;
      case kCFStringNormalizationFormKC:
        return UNORM_NFKC;
      default:
        return 1;
    }
}

void
CFStringNormalize (CFMutableStringRef str,
  CFStringNormalizationForm theForm)
{
/*  UChar *ubuffer;
  int32_t len;
  int32_t newLen;
  UErrorCode err = U_ZERO_ERROR;
  UNormalizationMode mode = CFToICUNormalization (theForm);
  UNormalizationCheckResult checkResult;
  
   Make sure string isn't already normalized.  Use the quick check for speed.
     We still go through the normalization if the quick check does not
     return UNORM_YES.
  len = (int32_t)CFStringGetLength (str);
  checkResult = unorm_quickCheck (str->_contents, len, mode, &err);
  if (U_FAILURE(err) || checkResult == UNORM_YES)
    return;
  
  ubuffer = alloca (len * sizeof(UChar));
  u_memcpy (ubuffer, (UChar*)str->_contents, len);
  
   Assume the resulting buffer is the same size.  We only reallocate
     str's contents if the normalized string is larger than the 
     unnormalized string. 
  do
    {
      err = U_ZERO_ERROR;
      newLen = unorm_normalize (ubuffer, len, mode, 0,
        (UChar*)str->_contents, len, &err);
    } while (err == U_BUFFER_OVERFLOW_ERROR
             && CFStringCheckCapacityAndGrow(str, newLen, NULL)); */
  return;
}

Boolean
CFStringTransform (CFMutableStringRef str, CFRange *range,
  CFStringRef transform, Boolean reverse)
{
/*#define UTRANS_LENGTH 128
  struct __CFMutableString *mStr;
  UTransliterator *utrans;
  UChar transID[UTRANS_LENGTH];
  int32_t idLen;
  int32_t limit;
  UTransDirection dir;
  UErrorCode err = U_ZERO_ERROR;
  
  mStr = (struct __CFMutableString *)str;
  
  dir = reverse ? UTRANS_REVERSE : UTRANS_FORWARD;
  
  idLen = (int32_t)CFStringGetLength (str);
  if (idLen > UTRANS_LENGTH)
    idLen = UTRANS_LENGTH;
  CFStringGetCharacters (str, CFRangeMake(0, idLen), transID);
  utrans = utrans_openU (transID, idLen, dir, NULL, 0, NULL, &err);
  if (U_FAILURE(err))
    return false;
  
  do
    {
      newLen = utrans_transUChars (utrans, mStr->_contents, 
  if (U_FAILURE(err))
    return false;
  */
  return true;
}




/* All the Pascal string functions will go here.  None of them are currently
   implemented. */
CFStringRef
CFStringCreateWithPascalString (CFAllocatorRef alloc, ConstStr255Param pStr,
  CFStringEncoding encoding)
{
  return NULL;
}

CFStringRef
CFStringCreateWithPascalStringNoCopy (CFAllocatorRef alloc,
  ConstStr255Param pStr, CFStringEncoding encoding,
  CFAllocatorRef contentsDeallocate)
{
  return NULL;
}

Boolean
CFStringGetPascalString (CFStringRef str, StringPtr buffer,
  CFIndex bufferSize, CFStringEncoding encoding)
{
  return false;
}

ConstStringPtr
CFStringGetPascalStringPtr (CFStringRef str, CFStringEncoding encoding)
{
  if (CFStringHasLengthByte(str))
    return str->_contents;
  
  return NULL;
}

void
CFStringAppendPascalString (CFMutableStringRef str,
  ConstStr255Param pStr, CFStringEncoding encoding)
{
  return;
}



static pthread_mutex_t static_strings_lock = PTHREAD_MUTEX_INITIALIZER;
static CFMutableDictionaryRef static_strings;
/**
 * Hack to allocated CFStrings uniquely without compiler support.
 */
CFStringRef __CFStringMakeConstantString (const char *str)
{
  /* FIXME: Use CFMutableSet as David originally did whenever that type
     gets implemented. */
  CFStringRef new =
    CFStringCreateWithCString (NULL, str, kCFStringEncodingASCII);
  CFStringRef old =
    (CFStringRef)CFDictionaryGetValue (static_strings, (const void *)new);
  // Return the existing string pointer if we have one.
  if (NULL != old)
    {
      CFRelease (new);
      return old;
    }
  pthread_mutex_lock(&static_strings_lock);
  if (NULL == static_strings)
    {
      static_strings = CFDictionaryCreateMutable (NULL, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
  // Check again in case another thread added this string to the table while
  // we were waiting on the mutex.
  old = (CFStringRef)CFDictionaryGetValue (static_strings, (const void *)new);
  if (NULL == old)
    {
      // Note: In theory, for proper retain count tracking, we should release
      // new here.  We're not going to, because it is expected to persist for
      // the lifetime of the program
      CFDictionaryAddValue (static_strings, (const void *)new,
        (const void *)new);
      old = new;
    }
  else
    {
      CFRelease (new);
    }
  pthread_mutex_unlock(&static_strings_lock);
  return old;
}
