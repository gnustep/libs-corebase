/* CFString.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: May, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redisibute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is disibuted in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"
#include "CoreFoundation/GSCharacter.h"
#include "CoreFoundation/GSUnicode.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"
#include "GSMemory.h"

/* Need the next few includes so we can initialize the string constants */
#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFDateFormatter.h"
#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFStream.h"
#include "CoreFoundation/CFURLAccess.h"
#include "CoreFoundation/CFXMLParser.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if HAVE_UNICODE_UNORM_H
#include <unicode/unorm.h>
#endif
#if HAVE_UNICODE_USTRING_H
#include <unicode/ustring.h>
#endif
#if HAVE_UNICODE_UTRANS_H
#include <unicode/utrans.h>
#endif

#define BUFFER_SIZE 512

CONST_STRING_DECL (kCFStringTransformStripCombiningMarks,
                   "NFD; [:nonspacing mark:]Remove; NFC");
CONST_STRING_DECL (kCFStringTransformToLatin, "Any-Latin");
CONST_STRING_DECL (kCFStringTransformFullwidthHalfwidth,
                   "Fullwidth-Halfwidth");
CONST_STRING_DECL (kCFStringTransformLatinKatakana, "Latin-Katakana");
CONST_STRING_DECL (kCFStringTransformLatinHiragana, "Latin-Hiragana");
CONST_STRING_DECL (kCFStringTransformHiraganaKatakana, "Hiragana-Katakana");
CONST_STRING_DECL (kCFStringTransformMandarinLatin, "Mandarin-Latin");
CONST_STRING_DECL (kCFStringTransformLatinHangul, "Latin-Hangul");
CONST_STRING_DECL (kCFStringTransformLatinArabic, "Latin-Arabic");
CONST_STRING_DECL (kCFStringTransformLatinHebrew, "Latin-Hebrew");
CONST_STRING_DECL (kCFStringTransformLatinThai, "Latin-Thai");
CONST_STRING_DECL (kCFStringTransformLatinCyrillic, "Latin-Cyrillic");
CONST_STRING_DECL (kCFStringTransformLatinGreek, "Latin-Greek");
CONST_STRING_DECL (kCFStringTransformToXMLHex, "Any-Hex/XML");
CONST_STRING_DECL (kCFStringTransformToUnicodeName, "Any-Name");
CONST_STRING_DECL (kCFStringTransformStripDiacritics,
                   "NFD; [:nonspacing marks:]Remove; NFC");

/* CFString has two possible internal encodings:
     * UTF-16 (preferable)
     * ASCII
   If the encoding is not one of the two listed above, it will be converted
   to UTF-16 any character is not ASCII.
*/

struct __CFString
{
  CFRuntimeBase _parent;
  void *_contents;
  CFIndex _count;
  CFHashCode _hash;
  CFAllocatorRef _deallocator;
};

struct __CFMutableString
{
  CFRuntimeBase _parent;
  UniChar *_contents;
  CFIndex _count;
  CFHashCode _hash;
  CFAllocatorRef _allocator;
  CFIndex _capacity;
};

static CFTypeID _kCFStringTypeID;

/* These are some masks to access the data in CFRuntimeBase's _flags.info
   field. */
enum
{
  _kCFStringIsMutable = (1 << 0),
  _kCFStringIsInline = (1 << 1),
  _kCFStringIsUnicode = (1 << 2),
  _kCFStringHasLengthByte = (1 << 3),   /* This is used for Pascal strings */
  _kCFStringHasNullByte = (1 << 4)
};

// TODO: dispatch to ObjC in all of these methods OR check its use

CF_INLINE Boolean
CFStringIsMutable (CFStringRef str)
{
  return
    ((CFRuntimeBase *) str)->_flags.info & _kCFStringIsMutable ? true : false;
}

CF_INLINE Boolean
CFStringIsUnicode (CFStringRef str)
{
  return ((CFRuntimeBase *) str)->
    _flags.info & _kCFStringIsUnicode ? true : false;
}

CF_INLINE Boolean
CFStringIsInline (CFStringRef str)
{
  return
    ((CFRuntimeBase *) str)->_flags.info & _kCFStringIsInline ? true : false;
}

CF_INLINE Boolean
CFStringHasNullByte (CFStringRef str)
{
  return
    ((CFRuntimeBase *) str)->
    _flags.info & _kCFStringHasNullByte ? true : false;
}

CF_INLINE void
CFStringSetMutable (CFStringRef str)
{
  ((CFRuntimeBase *) str)->_flags.info |= _kCFStringIsMutable;
}

CF_INLINE void
CFStringSetUnicode (CFStringRef str)
{
  ((CFRuntimeBase *) str)->_flags.info |= _kCFStringIsUnicode;
}

CF_INLINE void
CFStringSetInline (CFStringRef str)
{
  ((CFRuntimeBase *) str)->_flags.info |= _kCFStringIsInline;
}

CF_INLINE void
CFStringSetNullByte (CFStringRef str)
{
  ((CFRuntimeBase *) str)->_flags.info |= _kCFStringHasNullByte;
}



static void
CFStringFinalize (CFTypeRef cf)
{
  CFStringRef str = (CFStringRef) cf;

  if (!CFStringIsInline (str))
    CFAllocatorDeallocate (str->_deallocator, str->_contents);
}

static Boolean
CFStringEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return CFStringCompare (cf1, cf2, 0) == 0 ? true : false;
}

static CFHashCode
CFStringHash (CFTypeRef cf)
{
  CFStringRef str = (CFStringRef) cf;
  UniChar *buf;
  CFIndex len;
  Boolean isObjc;
  CFHashCode hash;

  isObjc = CF_IS_OBJC (_kCFStringTypeID, str);

  if (!isObjc)
    {
      if (str->_hash == 0)
        {
          if (CFStringIsUnicode (str))
            {
              len = CFStringGetLength (str) * sizeof (UniChar);
              ((struct __CFString *) str)->_hash =
                GSHashBytes (str->_contents, len);
              return str->_hash;
            }
        }
      else
        return str->_hash;
    }

  len = CFStringGetLength (str) * sizeof (UniChar);

  buf = CFAllocatorAllocate (kCFAllocatorSystemDefault, len, 0);
  CFStringGetCharacters (str, CFRangeMake (0, len / 2), buf);

  hash = GSHashBytes (buf, len);
  if (!isObjc)
    ((struct __CFString *) str)->_hash = hash;

  CFAllocatorDeallocate (kCFAllocatorSystemDefault, buf);
  return hash;
}

static CFStringRef
CFStringCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFStringCreateCopy (kCFAllocatorSystemDefault, cf);
}

static const CFRuntimeClass CFStringClass = {
  0,
  "CFString",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) CFStringCreateCopy,
  CFStringFinalize,
  CFStringEqual,
  CFStringHash,
  CFStringCopyFormattingDesc,
  NULL
};

static GSMutex static_strings_lock;
static CFMutableDictionaryRef static_strings;
/*
 * Hack to allocated CFStrings uniquely without compiler support.
 */
CFStringRef
__CFStringMakeConstantString (const char *str)
{
  CFStringRef old;

  if (static_strings == NULL)
    {
      /* The 170 capacity is really arbitrary.  I just wanted to make the
       * number large enough so that the hash table size comes out to 257,
       * a table large enough to fit most needs before needing to expand.
       */
      GSMutexLock (&static_strings_lock);
      if (static_strings == NULL)
        static_strings = CFDictionaryCreateMutable (NULL, 170, NULL, NULL);
      GSMutexUnlock (&static_strings_lock);
    }

  old = (CFStringRef) CFDictionaryGetValue (static_strings, str);

  /* Return the existing string pointer if we have one. */
  if (NULL != old)
    return old;

  GSMutexLock (&static_strings_lock);
  /* Check again in case another thread added this string to the table while
   * we were waiting on the mutex. */
  old = (CFStringRef) CFDictionaryGetValue (static_strings, str);
  if (NULL == old)
    {
      struct __CFString *new_const_str;

      new_const_str =
        CFAllocatorAllocate (NULL, sizeof (struct __CFString), 0);
      assert (new_const_str);

      /* Using _CFRuntimeInitStaticInstance() guarantees that any CFRetain or
       * CFRelease calls on object will be a no-op.
       */
      _CFRuntimeInitStaticInstance (new_const_str, _kCFStringTypeID);
      new_const_str->_contents = (void *) str;
      new_const_str->_count = strlen (str);
      new_const_str->_hash = 0;
      new_const_str->_deallocator = NULL;

      CFDictionaryAddValue (static_strings, str,
                            (const void *) new_const_str);
      old = new_const_str;
    }
  GSMutexUnlock (&static_strings_lock);

  return old;
}

void
CFStringInitialize (void)
{
  _kCFStringTypeID = _CFRuntimeRegisterClass (&CFStringClass);
  GSMutexInitialize (&static_strings_lock);

  GSRuntimeConstantInit (kCFErrorDomainPOSIX, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDomainOSStatus, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDomainMach, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDomainCocoa, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorLocalizedDescriptionKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorLocalizedFailureReasonKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorLocalizedRecoverySuggestionKey,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDescriptionKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorUnderlyingErrorKey, _kCFStringTypeID);

  GSRuntimeConstantInit (kCFStringTransformStripCombiningMarks,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToLatin, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformFullwidthHalfwidth,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinKatakana, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHiragana, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformHiraganaKatakana,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformMandarinLatin, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHangul, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinArabic, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHebrew, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinThai, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinCyrillic, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinGreek, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToXMLHex, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToUnicodeName, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformStripDiacritics, _kCFStringTypeID);

  GSRuntimeConstantInit (kCFLocaleMeasurementSystem, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleDecimalSeparator, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleGroupingSeparator, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCurrencySymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCurrencyCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleLanguageCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCountryCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleScriptCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleVariantCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleExemplarCharacterSet, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCalendarIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCollationIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleUsesMetricSystem, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCollatorIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleQuotationBeginDelimiterKey,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleQuotationEndDelimiterKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleAlternateQuotationBeginDelimiterKey,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleAlternateQuotationEndDelimiterKey,
                         _kCFStringTypeID);

  GSRuntimeConstantInit (kCFGregorianCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFBuddhistCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFChineseCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFHebrewCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFIslamicCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFIslamicCivilCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFJapaneseCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFRepublicOfChinaCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFPersianCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFIndianCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFISO8601Calendar, _kCFStringTypeID);

  GSRuntimeConstantInit (kCFDateFormatterIsLenient, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterTimeZone, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterCalendarName, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterDefaultFormat, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterTwoDigitStartDate, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterDefaultDate, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterEraSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterMonthSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortMonthSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterWeekdaySymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterAMSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterPMSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterLongEraSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortStandaloneMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortStandaloneWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterQuarterSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortQuarterSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneQuarterSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneQuarterSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterGregorianStartDate,
                         _kCFStringTypeID);

  GSRuntimeConstantInit (kCFNumberFormatterCurrencyCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterDecimalSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencyDecimalSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterAlwaysShowDecimalSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterGroupingSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterUseGroupingSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPercentSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterZeroSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterNaNSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterInfinitySymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinusSign, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPlusSign, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencySymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterExponentSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinIntegerDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxIntegerDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinFractionDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxFractionDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterGroupingSize, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterSecondaryGroupingSize,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterRoundingMode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterRoundingIncrement,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterFormatWidth, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPaddingPosition, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPaddingCharacter,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterDefaultFormat, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMultiplier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPositivePrefix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPositiveSuffix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterNegativePrefix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterNegativeSuffix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPerMillSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterInternationalCurrencySymbol,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencyGroupingSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterIsLenient, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterUseSignificantDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinSignificantDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxSignificantDigits,
                         _kCFStringTypeID);

  GSRuntimeConstantInit (kCFRunLoopCommonModes, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFRunLoopDefaultMode, _kCFStringTypeID);

  GSRuntimeConstantInit (kCFStreamPropertyDataWritten, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertySocketNativeHandle,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertySocketRemoteHostName,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertySocketRemotePortNumber,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertyAppendToFile, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertyFileCurrentOffset,
                         _kCFStringTypeID);

  GSRuntimeConstantInit (kCFXMLTreeErrorDescription, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFXMLTreeErrorLineNumber, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFXMLTreeErrorLocation, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFXMLTreeErrorStatusCode, _kCFStringTypeID);

  GSRuntimeConstantInit (kCFURLFileExists, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileDirectoryContents, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileLength, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileLastModificationTime, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFilePOSIXMode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileOwnerID, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLHTTPStatusCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLHTTPStatusLine, _kCFStringTypeID);
}



void
CFShow (CFTypeRef obj)
{
  CFStringRef str;
  const char *out;
  char buffer[256];

  str = CFCopyDescription (obj);
  if (str)
    {
      out = CFStringGetCStringPtr (str, kCFStringEncodingASCII);
      if (out == NULL)
        {
          if (CFStringGetCString (str, buffer, 256, kCFStringEncodingASCII))
            out = buffer;
        }
      CFRelease (str);
    }
  else
    {
      out = NULL;
    }
  fprintf (stderr, "%s\n", out);
}

void
CFShowStr (CFStringRef s)
{
  fprintf (stderr, "Length %ld\n", (long) CFStringGetLength (s));
  fprintf (stderr, "IsWide %d\n", CFStringIsUnicode (s));
  fprintf (stderr, "InlineContents %d\n", CFStringIsInline (s));
  fprintf (stderr, "Allocator %p\n", CFGetAllocator (s));
  fprintf (stderr, "Mutable %d\n", CFStringIsMutable (s));
  fprintf (stderr, "Contents ");
  CFShow (s);
}

CFTypeID
CFStringGetTypeID (void)
{
  return _kCFStringTypeID;
}

/* CFStringCreateImmutable function will return an inlined string whenever
   possible.  The contents may or may not be inlined if a NoCopy function
   is called. With this in mind, CFStringCreateWithBytes will return a
   string with the content inlined and CFStringCreateWithBytesNoCopy will not
   have inlined content if, and only if, the input bytes are in one of the
   internal encodings (ASCII or UTF-16).
 */
#define CFSTRING_SIZE \
  sizeof(struct __CFString) - sizeof(struct __CFRuntimeBase)

static CFStringRef
CFStringCreateImmutable (CFAllocatorRef alloc, const UInt8 * bytes,
                         CFIndex numBytes, CFStringEncoding encoding,
                         Boolean isExtRep, CFAllocatorRef contentsDealloc,
                         Boolean copy)
{
  struct __CFString *new;
  UniChar b[BUFFER_SIZE];
  UniChar *buffer;
  UniChar *bufferStart;
  CFIndex need;
  CFIndex extra;

  buffer = b;
  bufferStart = b;

  /* Check if we can store this string as ASCII */
  if (__CFStringEncodingIsSupersetOfASCII (encoding))
    {
      const UInt8 *s;
      const UInt8 *limit;

      s = bytes;
      limit = s + numBytes;
      while (s < limit)
        {
          if (*s++ > 0x7F)
            break;
        }
      if (s >= limit)
        encoding = kCFStringEncodingASCII;
    }

  if (encoding == kCFStringEncodingASCII)
    {
      need = numBytes;
      extra = numBytes + 1;
    }
  else
    {
      const UInt8 *src;

      src = bytes;
      /* We'll include a loss character for this conversion in case
         there is an error in the input stream. We dont' want to fail
         just because someone included an invalid code point/unit.
         The loss character will be the Unicode replacement character U+FFFD.
       */
      need = GSUnicodeFromEncoding (&buffer, buffer + BUFFER_SIZE, encoding,
                                    &src, src + numBytes, 0xFFFD);
      if (need < 0)             /* There is something seriously wrong! */
        return NULL;
      extra = (need + 1) * sizeof (UniChar);
    }
  if (!copy)
    {
      if (encoding == kCFStringEncodingUTF16)
        {
          UniChar c;

          c = *((const UniChar *) bytes);
          if (c == kGSUTF16CharacterSwappedByteOrderMark)
            copy = true;
        }
      else if (encoding != kCFStringEncodingASCII)
        {
          copy = true;
        }
#if __BIG_ENDIAN__
      else if (encoding == kCFStringEncodingUTF16LE)
        {
          copy = true;
        }
#else
      else if (encoding == kCFStringEncodingUTF16BE)
        {
          copy = true;
        }
#endif
    }

  new = (struct __CFString *) _CFRuntimeCreateInstance (alloc,
                                                        _kCFStringTypeID,
                                                        CFSTRING_SIZE +
                                                        extra, NULL);
  if (new)
    {
      if (contentsDealloc == NULL)
        contentsDealloc = CFAllocatorGetDefault ();
      new->_deallocator = CFRetain (contentsDealloc);

      if (copy)
        {
          new->_contents = &(new[1]);

          if (encoding == kCFStringEncodingASCII)
            {
              GSMemoryCopy (new->_contents, bytes, numBytes);
            }
          else if (extra <= BUFFER_SIZE)
            {
              GSMemoryCopy (new->_contents, bufferStart,
                            need * sizeof (UniChar));
              CFStringSetUnicode (new);
            }
          else
            {
              UniChar *contents;

              contents = new->_contents;
              GSUnicodeFromEncoding (&contents, contents + need, encoding,
                                     &bytes, bytes + numBytes, 0xFFFD);
              CFStringSetUnicode (new);
            }

          new->_count = need;
          CFStringSetInline (new);
          CFStringHasNullByte (new);
        }
      else
        {
          new->_contents = (void *) bytes;
          new->_count = encoding == kCFStringEncodingASCII
            ? numBytes : numBytes / sizeof (UniChar);
          if (encoding != kCFStringEncodingASCII)
            CFStringSetUnicode (new);
        }
    }

  return (CFStringRef) new;
}

CFStringRef
CFStringCreateWithBytes (CFAllocatorRef alloc, const UInt8 * bytes,
                         CFIndex numBytes, CFStringEncoding encoding,
                         Boolean isExternalRepresentation)
{
  return CFStringCreateImmutable (alloc, bytes, numBytes, encoding,
                                  isExternalRepresentation, NULL, true);
}

CFStringRef
CFStringCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 * bytes,
                               CFIndex numBytes, CFStringEncoding encoding,
                               Boolean isExternalRepresentation,
                               CFAllocatorRef contentsDeallocator)
{
  return CFStringCreateImmutable (alloc, bytes, numBytes, encoding,
                                  isExternalRepresentation,
                                  contentsDeallocator, false);
}

CFStringRef
CFStringCreateCopy (CFAllocatorRef alloc, CFStringRef str)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      CFIndex length = CFStringGetLength (str);
      UniChar *buf =
        (UniChar *) CFAllocatorAllocate (alloc, sizeof (UniChar) * length, 0);

      CFStringGetCharacters (str, CFRangeMake (0, length), buf);
      return CFStringCreateWithCharactersNoCopy (alloc, buf, length, alloc);
    }

  CFIndex length;
  CFStringRef new;
  CFStringEncoding enc;

  if (alloc == NULL)
    alloc = CFAllocatorGetDefault ();

  if (CFGetAllocator (str) == alloc && !CFStringIsMutable (str))
    return CFRetain (str);

  length =
    CFStringIsUnicode (str) ? str->_count * sizeof (UniChar) : str->_count;
  enc =
    CFStringIsUnicode (str) ? kCFStringEncodingUTF16 : kCFStringEncodingASCII;
  new = CFStringCreateWithBytes (alloc, str->_contents, length, enc, false);

  return new;
}

CFStringRef
CFStringCreateWithFileSystemRepresentation (CFAllocatorRef alloc,
                                            const char *buffer)
{
  /* FIXME: Need to make sure the system encoding will work here. */
  return CFStringCreateWithCString (alloc, buffer,
                                    CFStringGetSystemEncoding ());
}

CFStringRef
CFStringCreateFromExternalRepresentation (CFAllocatorRef alloc,
                                          CFDataRef data,
                                          CFStringEncoding encoding)
{
  const UInt8 *bytes = CFDataGetBytePtr (data);
  CFIndex numBytes = CFDataGetLength (data);
  return CFStringCreateWithBytes (alloc, bytes, numBytes, encoding, true);
}

CFStringRef
CFStringCreateWithCharacters (CFAllocatorRef alloc, const UniChar * chars,
                              CFIndex numChars)
{
  return CFStringCreateWithBytes (alloc, (const UInt8 *) chars,
                                  numChars * sizeof (UniChar),
                                  kCFStringEncodingUnicode, false);
}

CFStringRef
CFStringCreateWithCharactersNoCopy (CFAllocatorRef alloc,
                                    const UniChar * chars, CFIndex numChars,
                                    CFAllocatorRef contentsDeallocator)
{
  return CFStringCreateWithBytesNoCopy (alloc, (const UInt8 *) chars,
                                        numChars * sizeof (UniChar),
                                        kCFStringEncodingUnicode, false,
                                        contentsDeallocator);
}

CFStringRef
CFStringCreateWithCString (CFAllocatorRef alloc, const char *cStr,
                           CFStringEncoding encoding)
{
  CFIndex len = strlen (cStr);
  return CFStringCreateWithBytes (alloc, (const UInt8 *) cStr, len, encoding,
                                  false);
}

CFStringRef
CFStringCreateWithCStringNoCopy (CFAllocatorRef alloc, const char *cStr,
                                 CFStringEncoding encoding,
                                 CFAllocatorRef contentsDeallocator)
{
  CFIndex len = strlen (cStr);
  return CFStringCreateWithBytesNoCopy (alloc, (const UInt8 *) cStr, len,
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
                                      CFDictionaryRef formatOptions,
                                      CFStringRef format, va_list arguments)
{
  Boolean free_fmt_str = false;
  CFIndex str_len;
  CFIndex fmt_str_len;
  UniChar buf[BUFFER_SIZE];
  UniChar *str = buf;
  const UniChar *fmt_str;
  CFStringRef fmted_str;

  fmt_str_len = CFStringGetLength (format);
  fmt_str = CFStringGetCharactersPtr (format);
  if (fmt_str == 0)
    {
      CFRange range;

      free_fmt_str = true;
      fmt_str = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                     fmt_str_len * sizeof (UniChar), 0);
      range = CFRangeMake (0, fmt_str_len);
      CFStringGetCharacters (format, range, (UniChar *) fmt_str);
    }
  str_len = GSUnicodeFormatWithArguments (str, BUFFER_SIZE, formatOptions,
                                          fmt_str, fmt_str_len, arguments);
  if (str_len > BUFFER_SIZE)
    {
      /* The buffer was not long enough for the formatted string, so we will
       * need to allocate a longer buffer and try again.
       */
      str = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                 str_len * sizeof (UniChar), 0);
      str_len = GSUnicodeFormatWithArguments (str, str_len, formatOptions,
                                              fmt_str, fmt_str_len,
                                              arguments);

    }
  if (str_len < 0)
    return NULL;

  fmted_str = CFStringCreateWithCharacters (alloc, (const UniChar *) str,
                                            str_len);
  if (free_fmt_str)
    CFAllocatorDeallocate (kCFAllocatorSystemDefault, (void *) fmt_str);
  if (str != buf)
    CFAllocatorDeallocate (kCFAllocatorSystemDefault, str);

  return fmted_str;
}

CFStringRef
CFStringCreateWithSubstring (CFAllocatorRef alloc, CFStringRef str,
                             CFRange range)
{
  void *contents;
  CFIndex len;
  CFStringEncoding enc;

  if (CFStringIsUnicode (str))
    {
      enc = kCFStringEncodingUnicode;
      len = range.length * sizeof (UniChar);
      contents = ((UniChar *) str->_contents) + range.location;
    }
  else
    {
      enc = kCFStringEncodingASCII;
      len = range.length;
      contents = ((char *) str->_contents) + range.location;
    }

  return CFStringCreateWithBytes (alloc, (const UInt8 *) contents, len, enc,
                                  false);
}

CFDataRef
CFStringCreateExternalRepresentation (CFAllocatorRef alloc,
                                      CFStringRef str,
                                      CFStringEncoding encoding,
                                      UInt8 lossByte)
{
  UInt8 *buffer;
  CFRange range;
  CFIndex numBytes;
  CFIndex strLen;
  CFIndex len;
  CFIndex usedLen = 0;

  strLen = CFStringGetLength (str);
  range = CFRangeMake (0, strLen);
  len = strLen + 1;             /* include space for a NULL byte. */

  buffer = CFAllocatorAllocate (alloc, len, 0);

  numBytes = CFStringGetBytes (str, range, encoding, lossByte,
                               true, buffer, len, &usedLen);

  if (numBytes == 0)
    return NULL;

  return CFDataCreateWithBytesNoCopy (alloc, buffer, usedLen, alloc);
}

const UniChar *
CFStringGetCharactersPtr (CFStringRef str)
{
  if (!CF_IS_OBJC (_kCFStringTypeID, str) && CFStringIsUnicode (str))
    return str->_contents;

  return NULL;
}

const char *
CFStringGetCStringPtr (CFStringRef str, CFStringEncoding enc)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, const char *, str,
                         "cStringUsingEncoding:",
                         CFStringConvertEncodingToNSStringEncoding (enc));

  return (!CFStringIsUnicode (str)
          && __CFStringEncodingIsSupersetOfASCII (enc)) ? str->
    _contents : NULL;
}

CFIndex
CFStringGetBytes (CFStringRef str, CFRange range, CFStringEncoding enc,
                  UInt8 lossByte, Boolean isExtRep, UInt8 * buffer,
                  CFIndex maxBufLen, CFIndex * usedBufLen)
{
  UInt8 *bufferStart;
  const UniChar *sUnicode;
  CFIndex converted;

  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      /* Boolean b; */
      uintptr_t opts = 0;

      if (lossByte != 0)
        opts |= 1;              /* NSStringEncodingConversionAllowLossy */
      if (isExtRep != 0)
        opts |= 2;              /*NSStringEncodingConversionExternalRepresentation */

      // FIXME: Base's NSString doesn't implement getBytes:... yet!
      *usedBufLen = 0;
      return 0;

      /*
         CF_OBJC_CALLV (Boolean, b, str,
         "getBytes:maxLength:usedLength:encoding:options:range:remainingRange:",
         buffer, maxBufLen, usedBufLen,
         CFStringConvertEncodingToNSStringEncoding (enc), opts,
         range, NULL);

         return b ? *usedBufLen : 0;
       */
    }

  bufferStart = buffer;
  sUnicode = CFStringGetCharactersPtr (str);

  if (sUnicode)
    {
      const UniChar *sUnicodeStart;
      const UniChar *sLimit;

      sUnicodeStart = sUnicode + range.location;
      sUnicode = sUnicodeStart;
      sLimit = sUnicodeStart + range.length;
      /* It is safe to discard the return value in this case. */
      GSUnicodeToEncoding (&buffer, buffer + maxBufLen, enc, &sUnicode,
                           sLimit, lossByte, isExtRep);
      converted = sUnicode - sUnicodeStart;
    }
  else if (__CFStringEncodingIsSupersetOfASCII (enc))
    {
      const char *sCString;

      sCString = CFStringGetCStringPtr (str, kCFStringEncodingASCII);
      if (sCString)
        {
          sCString += range.location;
          converted = range.length > maxBufLen ? maxBufLen : range.length;
          GSMemoryCopy (buffer, sCString, converted);
          buffer += converted;
        }
      else
        {
          converted = 0;
        }
    }
  else if (enc == kCFStringEncodingUnicode)
    {
      range.length =
        range.length >
        (maxBufLen / sizeof (UniChar)) ? maxBufLen /
        sizeof (UniChar) : range.length;
      CFStringGetCharacters (str, range, (UniChar *) buffer);
      buffer += range.length * sizeof (UniChar);
      converted = range.length;
    }
  if (usedBufLen)
    *usedBufLen = buffer - bufferStart;

  return converted;             /* FIXME */
}

void
CFStringGetCharacters (CFStringRef str, CFRange range, UniChar * buffer)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, void, str,
                         "getCharacters:range:", buffer, range);

  if (CFStringIsUnicode (str))
    {
      memcpy (buffer, ((UniChar *) str->_contents) + range.location,
              range.length * sizeof (UniChar));
    }
  else
    {
      UInt8 *c;
      UInt8 *stop;

      c = str->_contents + range.location;
      stop = c + range.length;
      while (c < stop)
        *buffer++ = *c++;
    }
}

Boolean
CFStringGetCString (CFStringRef str, char *buffer, CFIndex bufferSize,
                    CFStringEncoding encoding)
{
  CFIndex len = CFStringGetLength (str);
  CFIndex used;

  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, Boolean, str,
                         "getCString:maxLength:encoding:", buffer, bufferSize,
                         CFStringConvertEncodingToNSStringEncoding
                         (encoding));

  if (CFStringGetBytes (str, CFRangeMake (0, len), encoding, 0, false,
                        (UInt8 *) buffer, bufferSize, &used) == len
      && used <= len)
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
  /* FIXME */
  return CFStringGetCString (string, buffer, maxBufLen,
                             CFStringGetSystemEncoding ());
}


UniChar
CFStringGetCharacterAtIndex (CFStringRef str, CFIndex idx)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, UniChar, str,
                         "characterAtIndex:", idx);
  return CFStringIsUnicode (str) ? ((UniChar *) str->_contents)[idx] :
    ((UInt8 *) str->_contents)[idx];
}

CFIndex
CFStringGetLength (CFStringRef str)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, CFIndex, str, "length");
  return str->_count;
}

CFRange
CFStringGetRangeOfComposedCharactersAtIndex (CFStringRef str,
                                             CFIndex theIndex)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, CFRange, str,
                         "rangeOfComposedCharacterSequenceAtIndex:",
                         theIndex);

  if (CFStringIsUnicode (str))
    {
      CFIndex len = 1;
      UniChar *characters = ((UniChar *) str->_contents) + theIndex;
      if (U16_IS_SURROGATE (*characters))
        {
          len = 2;
          if (U16_IS_SURROGATE_TRAIL (*characters))
            theIndex -= 1;
        }

      return CFRangeMake (theIndex, len);
    }

  return CFRangeMake (theIndex, 1);
}

UTF32Char
CFStringGetLongCharacterForSurrogatePair (UniChar surrogateHigh,
                                          UniChar surrogateLow)
{
  return (UTF32Char) U16_GET_SUPPLEMENTARY (surrogateHigh, surrogateLow);
}

Boolean
CFStringGetSurrogatePairForLongCharacter (UTF32Char character,
                                          UniChar * surrogates)
{
  if (character > 0x10000)
    return false;

  surrogates[0] = U16_LEAD (character);
  surrogates[1] = U16_TRAIL (character);

  return true;
}

Boolean
CFStringIsSurrogateHighCharacter (UniChar character)
{
  return (Boolean) U16_IS_LEAD (character);
}

Boolean
CFStringIsSurrogateLowCharacter (UniChar character)
{
  return (Boolean) U16_IS_TRAIL (character);
}

double
CFStringGetDoubleValue (CFStringRef str)
{
  double d;
  Boolean success;
  CFNumberFormatterRef fmt;

  fmt = CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterDecimalStyle);
  if (fmt == NULL)
    return 0.0;

  success = CFNumberFormatterGetValueFromString (fmt, str, NULL,
                                                 kCFNumberDoubleType,
                                                 (void *) &d);

  CFRelease (fmt);
  return success ? d : 0.0;
}

SInt32
CFStringGetIntValue (CFStringRef str)
{
  SInt32 i;
  Boolean success;
  CFNumberFormatterRef fmt;

  fmt = CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterNoStyle);
  if (fmt == NULL)
    return 0;

  success = CFNumberFormatterGetValueFromString (fmt, str, NULL,
                                                 kCFNumberSInt32Type,
                                                 (void *) &i);

  CFRelease (fmt);
  return success ? i : 0;
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
  void *currentContents;
  void *newContents;
  struct __CFMutableString *mStr = (struct __CFMutableString *) str;

  if (mStr->_capacity >= newCapacity)
    {
      if (oldContentBuffer)
        *oldContentBuffer = str->_contents;
      return true;
    }

  currentContents = mStr->_contents;

  newContents = CFAllocatorAllocate (mStr->_allocator,
                                     (newCapacity * sizeof (UniChar)), 0);
  if (newContents == NULL)
    return false;
  mStr->_contents = newContents;
  mStr->_capacity = newCapacity;

  if (oldContentBuffer)
    *oldContentBuffer = currentContents;
  else
    CFAllocatorDeallocate (mStr->_allocator, currentContents);

  return true;
}

#define DEFAULT_STRING_CAPACITY 16

#define CFSTRING_INIT_MUTABLE(str) do \
{ \
  ((CFRuntimeBase *)str)->_flags.info = 0xFF \
    & (_kCFStringIsMutable | _kCFStringIsUnicode | _kCFStringHasNullByte); \
} while(0)

CFMutableStringRef
CFStringCreateMutable (CFAllocatorRef alloc, CFIndex maxLength)
{
  struct __CFMutableString *new;

  new = (struct __CFMutableString *) _CFRuntimeCreateInstance (alloc,
                                                               _kCFStringTypeID,
                                                               sizeof (struct
                                                                       __CFMutableString)
                                                               -
                                                               sizeof
                                                               (CFRuntimeBase),
                                                               NULL);

  new->_capacity =
    maxLength < DEFAULT_STRING_CAPACITY ? DEFAULT_STRING_CAPACITY : maxLength;
  new->_allocator = alloc ? alloc : CFAllocatorGetDefault ();
  new->_contents = CFAllocatorAllocate (new->_allocator,
                                        new->_capacity * sizeof (UniChar), 0);

  CFSTRING_INIT_MUTABLE (new);

  return (CFMutableStringRef) new;
}

CFMutableStringRef
CFStringCreateMutableCopy (CFAllocatorRef alloc, CFIndex maxLength,
                           CFStringRef str)
{
  CFMutableStringRef new;
  CFStringInlineBuffer buffer;
  UniChar *contents;
  CFIndex textLen;
  CFIndex capacity;
  CFIndex idx;

  textLen = CFStringGetLength (str);
  capacity = textLen;
  if (maxLength > capacity)
    capacity = maxLength;
  new = (CFMutableStringRef) CFStringCreateMutable (alloc, capacity);

  /* An inline buffer is going to work well here... */
  CFStringInitInlineBuffer (str, &buffer, CFRangeMake (0, textLen));
  contents = (UniChar *) new->_contents;
  idx = 0;
  while (idx < textLen)
    {
      UniChar c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
      *(contents++) = c;
    }
  new->_count = textLen;

  CFSTRING_INIT_MUTABLE (new);

  return new;
}

CFMutableStringRef
CFStringCreateMutableWithExternalCharactersNoCopy (CFAllocatorRef alloc,
                                                   UniChar * chars,
                                                   CFIndex numChars,
                                                   CFIndex capacity,
                                                   CFAllocatorRef
                                                   externalCharactersAllocator)
{
  return NULL;                  /* FIXME */
}

void
CFStringSetExternalCharactersNoCopy (CFMutableStringRef str, UniChar * chars,
                                     CFIndex length, CFIndex capacity)
{
  return;                       /* FIXME */
}

CFIndex
CFStringFindAndReplace (CFMutableStringRef str, CFStringRef stringToFind,
                        CFStringRef replacementString, CFRange rangeToSearch,
                        CFOptionFlags compareOptions)
{
  return 0;                     /* FIXME */
}

void
CFStringAppend (CFMutableStringRef str, CFStringRef appendString)
{
  CFStringReplace (str, CFRangeMake (CFStringGetLength (str), 0),
                   appendString);
}

void
CFStringAppendCharacters (CFMutableStringRef str,
                          const UniChar * chars, CFIndex numChars)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      // TODO: add a test for this branch
      CFStringRef wrapped;

      wrapped = CFStringCreateWithCharactersNoCopy (NULL, chars, numChars,
                                                    kCFAllocatorNull);

      CF_OBJC_VOIDCALLV (str, "appendString:", wrapped);

      CFRelease (wrapped);
      return;
    }

  CFIndex length;
  void *contents;

  length = str->_count;

  if (CFStringCheckCapacityAndGrow (str, (length + numChars), &contents)
      && contents != str->_contents)
    {
      memcpy (str->_contents, contents, length * sizeof (UniChar));
      CFAllocatorDeallocate (str->_deallocator, contents);
    }

  memcpy ((UniChar *) str->_contents + length, chars,
          numChars * sizeof (UniChar));
  str->_count = length + numChars;
}

void
CFStringAppendCString (CFMutableStringRef str, const char *cStr,
                       CFStringEncoding encoding)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      // TODO: add a test for this branch
      CFStringRef wrapped;

      wrapped = CFStringCreateWithCStringNoCopy (NULL, cStr, encoding,
                                                 kCFAllocatorNull);

      CF_OBJC_VOIDCALLV (str, "appendString:", wrapped);

      CFRelease (wrapped);
      return;
    }

  UniChar b[BUFFER_SIZE];
  UniChar *buffer;
  UniChar *bufferStart;
  const UInt8 *cStrStart;
  const UInt8 *cStrLimit;
  CFIndex numChars;

  buffer = b;
  bufferStart = b;
  cStrStart = (const UInt8 *) cStr;
  cStrLimit = (const UInt8 *) cStr + strlen (cStr);

  numChars = GSUnicodeFromEncoding (&buffer, buffer + BUFFER_SIZE, encoding,
                                    (const UInt8 **) &cStr, cStrLimit, 0);
  if (numChars <= BUFFER_SIZE)
    {
      CFStringAppendCharacters (str, bufferStart, numChars);
    }
  else
    {
      UniChar *tmp;
      UniChar *tmpStart;

      tmp = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                 numChars * sizeof (UniChar), 0);
      tmpStart = tmp;
      GSUnicodeFromEncoding (&tmp, tmp + numChars, encoding,
                             (const UInt8 **) &cStrStart, cStrLimit, 0);
      CFStringAppendCharacters (str, tmpStart, numChars);
    }
}

void
CFStringAppendFormat (CFMutableStringRef str, CFDictionaryRef options,
                      CFStringRef format, ...)
{
  va_list args;
  va_start (args, format);
  CFStringAppendFormatAndArguments (str, options, format, args);
  va_end (args);
}

void
CFStringAppendFormatAndArguments (CFMutableStringRef str,
                                  CFDictionaryRef options, CFStringRef format,
                                  va_list args)
{
  Boolean free_fmt_str = false;
  CFIndex str_len;
  CFIndex fmt_str_len;
  UniChar buf[BUFFER_SIZE];
  UniChar *fmted_str = buf;
  const UniChar *fmt_str;

  fmt_str_len = CFStringGetLength (format);
  fmt_str = CFStringGetCharactersPtr (format);
  if (fmt_str == 0)
    {
      CFRange range;

      free_fmt_str = true;
      fmt_str = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                     fmt_str_len * sizeof (UniChar), 0);
      range = CFRangeMake (0, fmt_str_len);
      CFStringGetCharacters (format, range, (UniChar *) fmt_str);
    }
  str_len = GSUnicodeFormatWithArguments (fmted_str, BUFFER_SIZE, options,
                                          fmt_str, fmt_str_len, args);
  if (str_len > BUFFER_SIZE)
    {
      /* The buffer was not long enough for the formatted string, so we will
       * need to allocate a longer buffer and try again.
       */
      fmted_str = CFAllocatorAllocate (kCFAllocatorSystemDefault,
                                       str_len * sizeof (UniChar), 0);
      str_len = GSUnicodeFormatWithArguments (fmted_str, str_len, options,
                                              fmt_str, fmt_str_len, args);

    }
  if (str_len < 0)
    return;

  CFStringAppendCharacters (str, (const UniChar *) fmted_str, str_len);
  if (free_fmt_str)
    CFAllocatorDeallocate (kCFAllocatorSystemDefault, (void *) fmt_str);
  if (fmted_str != buf)
    CFAllocatorDeallocate (kCFAllocatorSystemDefault, str);
}

void
CFStringDelete (CFMutableStringRef str, CFRange range)
{
  CFStringReplace (str, range, CFSTR (""));
}

void
CFStringInsert (CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr)
{
  CFStringReplace (str, CFRangeMake (idx, 0), insertedStr);
}

void
CFStringPad (CFMutableStringRef str, CFStringRef padString,
             CFIndex length, CFIndex indexIntoPad)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      // TODO: add a test for this branch
      CFStringRef padded;

      CF_OBJC_CALLV (CFStringRef, padded, str,
                     "stringByPaddingToLength:withString:startingAtIndex:",
                     length, padString, indexIntoPad);

      CF_OBJC_VOIDCALLV (str, "setString:", padded);

      return;
    }

  if (padString == NULL && length < CFStringGetLength (str))    /* truncating */
    {
      ((UniChar *) str->_contents)[length] = 0x0000;
      str->_count = length;
      str->_hash = 0;
    }
  else
    {
      CFIndex padLength;
      UniChar *padContents;
      UniChar *contents;
      UniChar *end;

      if (!CFStringCheckCapacityAndGrow (str, length, (void **) &contents))
        return;
      if (contents != str->_contents)
        {
          memcpy (str->_contents, contents, length * sizeof (UniChar));
          CFAllocatorDeallocate (str->_deallocator, contents);
        }

      contents = ((UniChar *) str->_contents) + CFStringGetLength (str);
      end = ((UniChar *) str->_contents) + length;

      padLength = CFStringGetLength (padString);
      padContents =
        CFAllocatorAllocate (NULL, padLength * sizeof (UniChar), 0);
      CFStringGetCharacters (padString, CFRangeMake (0, padLength),
                             padContents);

      while (contents < end)
        {
          *contents++ = padContents[indexIntoPad++];
          if (indexIntoPad == padLength)
            indexIntoPad = 0;
        }

      CFAllocatorDeallocate (NULL, padContents);

      str->_count = length;
      str->_hash = 0;
    }
}

void
CFStringReplace (CFMutableStringRef str, CFRange range,
                 CFStringRef replacement)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, void, str,
                         "replaceCharactersInRange:withString:", range,
                         replacement);

  CFStringInlineBuffer buffer;
  CFIndex textLength;
  CFIndex repLength;
  CFIndex idx;
  UniChar *contents;

  textLength = CFStringGetLength (str);
  repLength = CFStringGetLength (replacement);

  if (repLength != range.length)
    {
      UniChar *moveFrom;
      UniChar *moveTo;
      UniChar *oldContents;
      CFIndex newLength;
      CFIndex moveLength;

      newLength = textLength - range.length + repLength;
      if (!CFStringCheckCapacityAndGrow
          (str, newLength, (void **) &oldContents))
        return;
      if (oldContents != str->_contents)
        {
          memcpy (str->_contents, oldContents,
                  range.location * sizeof (UniChar));
        }
      moveFrom = (oldContents + range.location + range.length);
      moveTo = (((UniChar *) str->_contents) + range.location + repLength);
      moveLength = textLength - (range.location + range.length);
      memmove (moveTo, moveFrom, moveLength * sizeof (UniChar));

      if (oldContents != str->_contents)
        CFAllocatorDeallocate (str->_deallocator, (void *) oldContents);

      textLength = newLength;
    }

  CFStringInitInlineBuffer (replacement, &buffer, CFRangeMake (0, repLength));
  contents = (((UniChar *) str->_contents) + range.location);
  idx = 0;
  while (idx < repLength)
    {
      UniChar c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
      *(contents++) = c;
    }
  str->_count = textLength;
  str->_hash = 0;
}

void
CFStringReplaceAll (CFMutableStringRef theString, CFStringRef replacement)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, void, theString, "setString:",
                         replacement);

  CFStringInlineBuffer buffer;
  CFIndex textLength;
  CFIndex idx;
  UniChar *contents;

  /* This function is very similar to CFStringReplace() but takes a few
     shortcuts and should be a little fast if all you need to do is replace
     the whole string. */
  textLength = CFStringGetLength (replacement);
  if (!CFStringCheckCapacityAndGrow (theString, textLength + 1, NULL))
    return;

  CFStringInitInlineBuffer (replacement, &buffer,
                            CFRangeMake (0, textLength));
  contents = (UniChar *) theString->_contents;
  idx = 0;
  while (idx < textLength)
    {
      UniChar c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
      *(contents++) = c;
    }
  theString->_count = textLength;
  theString->_hash = 0;
}

void
CFStringTrim (CFMutableStringRef str, CFStringRef trimString)
{
  CFStringFindAndReplace (str, trimString, NULL,
                          CFRangeMake (0, CFStringGetLength (str)),
                          kCFCompareAnchored);
  CFStringFindAndReplace (str, trimString, NULL,
                          CFRangeMake (0, CFStringGetLength (str)),
                          kCFCompareBackwards | kCFCompareAnchored);
}

void
CFStringTrimWhitespace (CFMutableStringRef str)
{
  CF_OBJC_FUNCDISPATCHV (_kCFStringTypeID, void, str, "_cfTrimWhitespace");

  CFStringInlineBuffer buffer;
  CFIndex start;
  CFIndex end;
  CFIndex textLength;
  CFIndex newLength;
  CFIndex idx;
  UniChar c;
  UniChar *contents;
  UniChar *contentsStart;

  /* I assume that the resulting string will be shorter than the original
     so no bounds checking is done. */
  textLength = CFStringGetLength (str);
  CFStringInitInlineBuffer (str, &buffer, CFRangeMake (0, textLength));

  idx = 0;
  c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
  while (GSCharacterIsWhitespace (c) && idx < textLength)
    c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
  start = idx - 1;
  end = start;
  while (idx < textLength)
    {
      c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
      /* reset the end point */
      if (!GSCharacterIsWhitespace (c))
        end = idx;
    }

  newLength = end - start;
  contents = (UniChar *) str->_contents;
  contentsStart = (UniChar *) (contents + start);
  memmove (contents, contentsStart, newLength * sizeof (UniChar));

  str->_count = newLength;
  str->_hash = 0;
}

enum
{
  _kCFStringCapitalize,
  _kCFStringLowercase,
  _kCFStringUppercase,
  _kCFStringFold
};

static void
CFStringCaseMap (CFMutableStringRef str, CFLocaleRef locale,
                 CFOptionFlags flags, CFIndex op)
{
#if HAVE_UNICODE_USTRING_H
  char *localeID = NULL;        /* FIXME */
  const UniChar *oldContents;
  CFIndex oldContentsLength;
  CFIndex newLength;
  int32_t optFlags;
  UErrorCode err = U_ZERO_ERROR;
  struct __CFMutableString *mStr = (struct __CFMutableString *) str;

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
                                    oldContents, oldContentsLength, NULL,
                                    localeID, &err);
          break;
        case _kCFStringLowercase:
          newLength = u_strToLower (mStr->_contents, mStr->_capacity,
                                    oldContents, oldContentsLength, localeID,
                                    &err);
          break;
        case _kCFStringUppercase:
          newLength = u_strToUpper (mStr->_contents, mStr->_capacity,
                                    oldContents, oldContentsLength, localeID,
                                    &err);
          break;
        case _kCFStringFold:
          optFlags = 0;         /* FIXME */
          newLength = u_strFoldCase (mStr->_contents, mStr->_capacity,
                                     oldContents, oldContentsLength, optFlags,
                                     &err);
          break;
        default:
          return;
        }
    }
  while (err == U_BUFFER_OVERFLOW_ERROR
         && CFStringCheckCapacityAndGrow (str, newLength,
                                          (void **) &oldContents));
  if (U_FAILURE (err))
    return;

  mStr->_count = newLength;
  mStr->_hash = 0;

  if (oldContents != mStr->_contents)
    CFAllocatorDeallocate (mStr->_allocator, (void *) oldContents);
#else
  /* FIXME */
#endif
}

void
CFStringCapitalize (CFMutableStringRef str, CFLocaleRef locale)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      CFStringRef mod;
      CF_OBJC_CALLV (CFStringRef, mod, str, "capitalizedString");
      CF_OBJC_VOIDCALLV (str, "setString:", mod);

      CFRelease (mod);
    }
  else
    CFStringCaseMap (str, locale, 0, _kCFStringCapitalize);
}

void
CFStringLowercase (CFMutableStringRef str, CFLocaleRef locale)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      CFStringRef mod;
      CF_OBJC_CALLV (CFStringRef, mod, str, "lowercaseString");
      CF_OBJC_VOIDCALLV (str, "setString:", mod);

      CFRelease (mod);
    }
  else
    CFStringCaseMap (str, locale, 0, _kCFStringLowercase);
}

void
CFStringUppercase (CFMutableStringRef str, CFLocaleRef locale)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      CFStringRef mod;
      CF_OBJC_CALLV (CFStringRef, mod, str, "uppercaseString");
      CF_OBJC_VOIDCALLV (str, "setString:", mod);

      CFRelease (mod);
    }
  else
    CFStringCaseMap (str, locale, 0, _kCFStringUppercase);
}

void
CFStringFold (CFMutableStringRef str, CFOptionFlags flags, CFLocaleRef locale)
{
  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      /* CFOptionFlags have the same insensitivity values
       * as NSStringCompareOptions.
       * CFLocaleRef is toll-free bridged*/

      CFStringRef mod;
      CF_OBJC_CALLV (CFStringRef, mod, str,
                     "stringByFoldingWithOptions:locale:", flags, locale);

      CF_OBJC_VOIDCALLV (str, "setString:", mod);
      CFRelease (mod);
    }
  else
    CFStringCaseMap (str, locale, flags, _kCFStringFold);
}

CF_INLINE UNormalizationMode
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
CFStringNormalize (CFMutableStringRef str, CFStringNormalizationForm theForm)
{
#if !UCONFIG_NO_NORMALIZATION
  /* FIXME: The unorm API has been officially deprecated on ICU 4.8, however,
     the new unorm2 API was only introduced on ICU 4.4.  The current plan is
     to provide compatibility down to ICU 4.0, so unorm is used here.  In
     the future, when we no longer support building with older versions of
     the library this code can be updated for the new API. */
  UniChar *oldContents;
  CFIndex oldContentsLength;
  CFIndex newLength;
  UNormalizationCheckResult checkResult;
  UErrorCode err = U_ZERO_ERROR;
  UNormalizationMode mode = CFToICUNormalization (theForm);
  struct __CFMutableString *mStr;
  CFMutableStringRef objc = NULL;

  /* Make sure string isn't already normalized.  Use the quick check for
     speed. We still go through the normalization if the quick check does not
     return UNORM_YES. */
  oldContents = (UniChar *) CFStringGetCharactersPtr (str);
  oldContentsLength = CFStringGetLength (str);

  if (oldContents != NULL)
    {
      checkResult =
        unorm_quickCheck (oldContents, oldContentsLength, mode, &err);
      if (U_FAILURE (err) || checkResult == UNORM_YES)
        return;
    }

  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      objc = str;
      str = CFStringCreateMutableCopy (kCFAllocatorDefault, 0, objc);
    }

  /* unorm_normalize requires that source/dest buffers don't overlap */
  mStr = (struct __CFMutableString *) str;
  oldContents = CFAllocatorAllocate (mStr->_allocator,
                                     oldContentsLength * sizeof (UniChar), 0);
  CFStringGetCharacters (str, CFRangeMake (0, oldContentsLength),
                         oldContents);

  /* Works just like CFStringCaseMap() above... */
  do
    {
      newLength = unorm_normalize (oldContents, oldContentsLength, mode,
                                   0, mStr->_contents, mStr->_capacity, &err);
    }
  while (err == U_BUFFER_OVERFLOW_ERROR
         && CFStringCheckCapacityAndGrow (str, newLength, NULL));

  if (U_FAILURE (err))
    return;

  mStr->_count = newLength;

  if (oldContents != mStr->_contents)
    CFAllocatorDeallocate (mStr->_allocator, (void *) oldContents);

  if (objc != NULL)
    {
      CF_OBJC_VOIDCALLV (objc, "setString:", str);
      CFRelease (str);
    }
#else
  /* FIXME */
#endif
}

Boolean
CFStringTransform (CFMutableStringRef str, CFRange * range,
                   CFStringRef transform, Boolean reverse)
{
#if !UCONFIG_NO_TRANSLITERATION
#define UTRANS_LENGTH 128
  struct __CFMutableString *mStr;
  UniChar transID[UTRANS_LENGTH];
  CFIndex idLength;
  CFIndex newLength;
  CFIndex start;
  CFIndex limit;
  UTransliterator *utrans;
  UTransDirection dir;
  UErrorCode err = U_ZERO_ERROR;

  dir = reverse ? UTRANS_REVERSE : UTRANS_FORWARD;

  idLength = CFStringGetLength (transform);
  if (idLength > UTRANS_LENGTH)
    idLength = UTRANS_LENGTH;
  CFStringGetCharacters (transform, CFRangeMake (0, idLength), transID);
  utrans = utrans_openU (transID, idLength, dir, NULL, 0, NULL, &err);
  if (U_FAILURE (err))
    return false;

  newLength = CFStringGetLength (str);
  if (range)
    {
      start = range->location;
      limit = range->length + start;
    }
  else
    {
      start = 0;
      limit = newLength;
    }

  if (CF_IS_OBJC (_kCFStringTypeID, str))
    {
      mStr = (struct __CFMutableString *)
        CFStringCreateMutableCopy (kCFAllocatorDefault, 0, str);
    }
  else
    {
      mStr = (struct __CFMutableString *) str;
    }

  utrans_transUChars (utrans, mStr->_contents, (int32_t *) & mStr->_count,
                      mStr->_capacity, start, (int32_t *) & limit, &err);
  utrans_close (utrans);

  if (((CFMutableStringRef) mStr) != str)       /* ObjC case */
    {
      CF_OBJC_VOIDCALLV (str, "setString:", mStr);
      CFRelease (mStr);
    }

  if (U_FAILURE (err))
    return false;

  if (range)
    range->length = limit;

  return true;
#else
  return false;
#endif
}
