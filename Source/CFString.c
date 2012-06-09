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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"
#include "GSPrivate.h"

/* Need the next few includes so we can initialize the string constants */
#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFDateFormatter.h"
#include "CoreFoundation/CFURLAccess.h"
#include "CoreFoundation/CFXMLParser.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unicode/uchar.h>
#include <unicode/unorm.h>
#include <unicode/ustring.h>
#include <unicode/utrans.h>

#define BUFFER_SIZE 512
#define CFRANGE_CHECK(len, range) \
  ((range.location + range.length) <= len)

CONST_STRING_DECL(kCFStringTransformStripCombiningMarks,
  "NFD; [:nonspacing mark:]Remove; NFC");
CONST_STRING_DECL(kCFStringTransformToLatin,
  "Any-Latin");
CONST_STRING_DECL(kCFStringTransformFullwidthHalfwidth,
  "Fullwidth-Halfwidth");
CONST_STRING_DECL(kCFStringTransformLatinKatakana,
  "Latin-Katakana");
CONST_STRING_DECL(kCFStringTransformLatinHiragana,
  "Latin-Hiragana");
CONST_STRING_DECL(kCFStringTransformHiraganaKatakana ,
  "Hiragana-Katakana");
CONST_STRING_DECL(kCFStringTransformMandarinLatin,
  "Mandarin-Latin");
CONST_STRING_DECL(kCFStringTransformLatinHangul,
  "Latin-Hangul");
CONST_STRING_DECL(kCFStringTransformLatinArabic,
  "Latin-Arabic");
CONST_STRING_DECL(kCFStringTransformLatinHebrew,
  "Latin-Hebrew");
CONST_STRING_DECL(kCFStringTransformLatinThai,
  "Latin-Thai");
CONST_STRING_DECL(kCFStringTransformLatinCyrillic,
  "Latin-Cyrillic");
CONST_STRING_DECL(kCFStringTransformLatinGreek,
  "Latin-Greek");
CONST_STRING_DECL(kCFStringTransformToXMLHex,
  "Any-Hex/XML");
CONST_STRING_DECL(kCFStringTransformToUnicodeName,
  "Any-Name");
CONST_STRING_DECL(kCFStringTransformStripDiacritics,
  "NFD; [:nonspacing marks:]Remove; NFC");

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

/* These are some masks to access the data in CFRuntimeBase's _flags.info
   field. */
enum
{
  _kCFStringIsMutable = (1<<0),
  _kCFStringIsInline  = (1<<1),
  _kCFStringIsUnicode = (1<<2),
  _kCFStringHasLengthByte = (1<<3), /* This is used for Pascal strings */
  _kCFStringHasNullByte = (1<<4)
};

CF_INLINE Boolean
CFStringIsMutable (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsMutable ? true : false;
}

CF_INLINE Boolean
CFStringIsUnicode (CFStringRef str)
{
  return ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsUnicode ? true : false;
}

CF_INLINE Boolean
CFStringIsInline (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringIsInline ? true : false;
}

CF_INLINE Boolean
CFStringHasNullByte (CFStringRef str)
{
  return
    ((CFRuntimeBase *)str)->_flags.info & _kCFStringHasNullByte ? true : false;
}

CF_INLINE void
CFStringSetMutable (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsMutable;
}

CF_INLINE void
CFStringSetUnicode (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsUnicode;
}

CF_INLINE void
CFStringSetInline (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringIsInline;
}

CF_INLINE void
CFStringSetNullByte (CFStringRef str)
{
  ((CFRuntimeBase *)str)->_flags.info |= _kCFStringHasNullByte;
}



static void CFStringFinalize (CFTypeRef cf)
{
  CFStringRef str = (CFStringRef)cf;
  
  if (!CFStringIsInline(str))
    CFAllocatorDeallocate (str->_deallocator, str->_contents);
}

static Boolean CFStringEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return CFStringCompare (cf1, cf2, 0) == 0 ? true : false;
}

static CFHashCode CFStringHash (CFTypeRef cf)
{
  CFStringRef str = (CFStringRef)cf;
  if (str->_hash == 0)
    {
      CFIndex len = CFStringGetLength (str) *
        (CFStringIsUnicode(str) ? sizeof(UniChar) : sizeof(char));
      ((struct __CFString *)str)->_hash = GSHashBytes (str->_contents, len);
    }

  return str->_hash;
}

static CFStringRef
CFStringCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFStringCreateCopy(kCFAllocatorSystemDefault, cf);
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

static GSMutex static_strings_lock;
static CFMutableDictionaryRef static_strings;
/*
 * Hack to allocated CFStrings uniquely without compiler support.
 */
CFStringRef __CFStringMakeConstantString (const char *str)
{
  CFStringRef old;
  
  if (static_strings == NULL)
    {
      /* The 170 capacity is really arbitrary.  I just wanted to make the
       * number large enough so that the hash table size comes out to 257,
       * a table large enough to fit most needs before needing to expand.
       */
      GSMutexLock(&static_strings_lock);
      if (static_strings == NULL)
        static_strings = CFDictionaryCreateMutable (NULL, 170, NULL, NULL);
      GSMutexUnlock(&static_strings_lock);
    }
  
  old = (CFStringRef)CFDictionaryGetValue (static_strings, str);
  
  /* Return the existing string pointer if we have one. */
  if (NULL != old)
    return old;
  
  GSMutexLock(&static_strings_lock);
  /* Check again in case another thread added this string to the table while
   * we were waiting on the mutex. */
  old = (CFStringRef)CFDictionaryGetValue (static_strings, str);
  if (NULL == old)
    {
      struct __CFString *new_const_str;
      
      new_const_str = CFAllocatorAllocate (NULL, sizeof(struct __CFString), 0);
      assert (new_const_str);
      
      /* Using _CFRuntimeInitStaticInstance() guarantees that any CFRetain or
       * CFRelease calls on object will be a no-op.
       */
      _CFRuntimeInitStaticInstance (new_const_str, _kCFStringTypeID);
      new_const_str->_contents = (void*)str;
      new_const_str->_count = strlen (str);
      new_const_str->_hash = 0;
      new_const_str->_deallocator = NULL;
      
      CFDictionaryAddValue (static_strings, str, (const void *)new_const_str);
      old = new_const_str;
    }
  GSMutexUnlock(&static_strings_lock);
  
  return old;
}

void CFStringInitialize (void)
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
  
  GSRuntimeConstantInit (kCFStringTransformStripCombiningMarks, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToLatin, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformFullwidthHalfwidth, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinKatakana, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHiragana, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformHiraganaKatakana, _kCFStringTypeID);
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
  GSRuntimeConstantInit (kCFLocaleQuotationBeginDelimiterKey, _kCFStringTypeID);
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
  GSRuntimeConstantInit (kCFDateFormatterShortWeekdaySymbols, _kCFStringTypeID);
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
  GSRuntimeConstantInit (kCFDateFormatterShortQuarterSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneQuarterSymbols,
    _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneQuarterSymbols,
    _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterGregorianStartDate, _kCFStringTypeID);

  GSRuntimeConstantInit (kCFNumberFormatterCurrencyCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterDecimalSeparator, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencyDecimalSeparator,
    _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterAlwaysShowDecimalSeparator,
    _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterGroupingSeparator, _kCFStringTypeID);
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
  GSRuntimeConstantInit (kCFNumberFormatterMinIntegerDigits, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxIntegerDigits, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinFractionDigits, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxFractionDigits, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterGroupingSize, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterSecondaryGroupingSize,
    _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterRoundingMode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterRoundingIncrement, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterFormatWidth, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPaddingPosition, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPaddingCharacter, _kCFStringTypeID);
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
  fprintf (stderr, "Length %d\n", (int)s->_count);
  fprintf (stderr, "IsWide %d\n", CFStringIsUnicode(s));
  fprintf (stderr, "InlineContents %d\n", CFStringIsInline(s));
  fprintf (stderr, "Allocator %p\n", CFGetAllocator(s));
  fprintf (stderr, "Mutable %d\n", CFStringIsMutable(s));
  fprintf (stderr, "Contents ");
  CFShow (s);
}

CFTypeID
CFStringGetTypeID (void)
{
  return _kCFStringTypeID;
}

CF_INLINE CFIndex
CFStringGetExtraBytes (CFStringEncoding *encoding, const UInt8 *bytes,
  CFIndex numBytes, Boolean isExtRep)
{
  CFIndex needed;
  
  if (__CFStringEncodingIsSupersetOfASCII(*encoding))
    {
      const UInt8 *start;
      const UInt8 *end;
      Boolean isASCII;
      
      start = bytes;
      end = start + numBytes;
      isASCII = true;
      while (start < end)
        {
          if (*start++ > 127)
            {
              isASCII = false;
              break;
            }
        }
      
      if (isASCII)
        {
          *encoding = kCFStringEncodingASCII;
          return numBytes;
        }
    }
  
  GSStringEncodingToUnicode(*encoding, NULL, 0, (const char**)&bytes,
    numBytes, isExtRep, &needed);
  
  return needed;
}

/* The CFString create function will return an inlined string whenever
   possible.  The contents may or may not be inlined if a NoCopy function
   is called. With this in mind, CFStringCreateWithBytes will return a
   string with the content inlined and CFStringCreateWithBytesNoCopy will not
   have inlined content if, and only if, the input bytes are in one of the
   internal encodings. */

#define CFSTRING_SIZE \
  sizeof(struct __CFString) - sizeof(struct __CFRuntimeBase)

static CFStringRef
CFStringCreateImmutable (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding, Boolean isExtRep,
  CFAllocatorRef contentsDealloc, Boolean copy)
{
  struct __CFString *new;
  CFIndex extra;
  
  /* We'll need to convert anything that isn't one of these encodings. */
  if (!(encoding == kCFStringEncodingASCII
      || encoding == kCFStringEncodingUTF16
      || encoding == UTF16_ENCODING))
    {
      CFStringEncoding origEncoding = encoding;
      extra = CFStringGetExtraBytes (&encoding, bytes, numBytes, isExtRep);
      
      /* We force a copy of the data if we can't represent it in one
       * of the internal encodings.
       */
      if (encoding == origEncoding && extra != numBytes)
        copy = true;
      if (copy) /* Add space for '\0' */
        extra += encoding == kCFStringEncodingASCII? 1 : sizeof(UniChar);
    }
  else
    {
      extra = copy ? numBytes : 0;
    }
  
  new = (struct __CFString *)_CFRuntimeCreateInstance (alloc,
    _kCFStringTypeID, CFSTRING_SIZE + extra, NULL);
  if (new)
    {
      if (contentsDealloc == NULL)
        contentsDealloc = CFAllocatorGetDefault ();
      
      if (copy)
        {
          new->_contents = (void*)&(new[1]);
          
          if (encoding == kCFStringEncodingASCII)
            {
              memcpy (new->_contents, bytes, numBytes);
              new->_count = numBytes;
            }
          else if (encoding == kCFStringEncodingUTF16
              || encoding == UTF16_ENCODING)
            {
              memcpy (new->_contents, bytes, numBytes);
              new->_count = numBytes / sizeof(UniChar);
              CFStringSetUnicode (new);
            }
          else
            {
              CFIndex count;
              
              count = GSStringEncodingToUnicode (encoding, new->_contents,
                extra / sizeof(UniChar), (const char**)&bytes, numBytes,
                isExtRep, NULL);
              
              new->_count = count;
              CFStringSetUnicode (new);
            }
          new->_deallocator = contentsDealloc;
          CFStringSetInline (new);
          CFStringHasNullByte (new);
        }
      else
        {
          new->_contents = (void*)bytes;
          new->_deallocator = contentsDealloc;
          new->_count = encoding == kCFStringEncodingASCII
            ? numBytes : numBytes / sizeof(UniChar);
          if (encoding != kCFStringEncodingASCII)
            CFStringSetUnicode (new);
        }
    }
  
  return (CFStringRef)new;
}

CFStringRef
CFStringCreateWithBytes (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding,
  Boolean isExternalRepresentation)
{
  return CFStringCreateImmutable (alloc, bytes, numBytes, encoding,
    isExternalRepresentation, NULL, true);
}

CFStringRef
CFStringCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding,
  Boolean isExternalRepresentation, CFAllocatorRef contentsDeallocator)
{
  return CFStringCreateImmutable (alloc, bytes, numBytes, encoding,
    isExternalRepresentation, contentsDeallocator, false);
}

CFStringRef
CFStringCreateCopy (CFAllocatorRef alloc, CFStringRef str)
{
  CFIndex length;
  CFStringRef new;
  CFStringEncoding enc;
  
  if (alloc == NULL)
    alloc = CFAllocatorGetDefault ();
  
  if (CFGetAllocator(str) == alloc && !CFStringIsMutable(str))
    return CFRetain (str);
  
  length = CFStringIsUnicode(str) ? str->_count * sizeof(UniChar) : str->_count;
  enc = CFStringIsUnicode(str) ? kCFStringEncodingUTF16 : kCFStringEncodingASCII;
  new = CFStringCreateWithBytes (alloc, str->_contents, length, enc, false);
  
  return new;
}

CFStringRef
CFStringCreateWithFileSystemRepresentation (CFAllocatorRef alloc,
  const char *buffer)
{
  /* FIXME: Need to make sure the system encoding will work here. */
  return CFStringCreateWithCString(alloc, buffer, CFStringGetSystemEncoding());
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
    numChars * sizeof(UniChar), UTF16_ENCODING, false);
}

CFStringRef
CFStringCreateWithCharactersNoCopy (CFAllocatorRef alloc, const UniChar *chars,
  CFIndex numChars, CFAllocatorRef contentsDeallocator)
{
  return CFStringCreateWithBytesNoCopy (alloc, (const UInt8*)chars,
    numChars * sizeof(UniChar), UTF16_ENCODING, false, contentsDeallocator);
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
  return _CFStringCreateWithFormatAndArgumentsAux (alloc, NULL,
    formatOptions, format, arguments);
}

CFStringRef
CFStringCreateWithSubstring (CFAllocatorRef alloc, CFStringRef str,
  CFRange range)
{
  void *contents;
  CFIndex len;
  CFStringEncoding enc;
  
  if (CFStringIsUnicode(str))
    {
      enc = UTF16_ENCODING;
      len = range.length * sizeof(UniChar);
      contents = ((UniChar*)str->_contents) + range.location;
    }
  else
    {
      enc = kCFStringEncodingASCII;
      len = range.length;
      contents = ((char*)str->_contents) + range.location;
    }
  
  return CFStringCreateWithBytes (alloc, (const UInt8*)contents, len, enc,
    false);
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
  len = strLen + 1; /* include space for a NULL byte. */
  
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
  return CFStringIsUnicode(str) ? str->_contents : NULL;
}

const char *
CFStringGetCStringPtr (CFStringRef str, CFStringEncoding enc)
{
  return (!CFStringIsUnicode(str) && __CFStringEncodingIsSupersetOfASCII(enc))
    ? str->_contents : NULL;
}

CFIndex
CFStringGetBytes (CFStringRef str, CFRange range,
  CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation,
  UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen)
{
  CFIndex used;
  CFIndex converted;
  const UniChar *characters;
  
  /* FIXME: Need ObjC bridge.
   * Use -getBytes:maxLength:usedLength:encoding:options:range:remainingRange:
   */
  
  characters = CFStringGetCharactersPtr (str);
  if (characters)
    {
      const UniChar *src;
      CFIndex bomLen = 0;
      
      characters += range.location;
      src = characters;
      used = GSStringEncodingFromUnicode (encoding, (char*)buffer, maxBufLen,
        &src, range.length, lossByte, isExternalRepresentation, NULL) + bomLen;
      converted = src - characters;
    }
  else
    {
      /* We can simply copy the ASCII characters into an ASCII superset
       * encoding.  We don't do this for UTF-8 because it may require a BOM.
       */
      if (__CFStringEncodingIsSupersetOfASCII(encoding)
          && encoding != kCFStringEncodingUTF8)
        {
          memcpy (buffer, ((char*)str->_contents) + range.location,
            range.length);
          used = range.length;
          converted = range.length;
        }
      else if (encoding == kCFStringEncodingUTF16
          || encoding == UTF16_ENCODING)
        {
          UniChar *dst;
          const char *bytes;
          const char *end;
          
          dst = (UniChar*)buffer;
          bytes = (char*)str->_contents + range.location;
          end = bytes + range.length;
          
          while (bytes < end)
            *dst++ = *bytes++;
          used = buffer - (UInt8*)dst;
          converted = bytes - (char*)str->_contents;
        }
      else
        {
          /* We'll use a pivot to first convert to Unicode and then to
           * whatever encoding was specified.
           */
          const char *bytes;
          const char *end;
          UniChar *chars;
          UniChar pivot[BUFFER_SIZE];
          
          converted = 0;
          used = 0;
          bytes = (char*)str->_contents + range.location;
          end = bytes + range.length;
          while (bytes < end)
            {
              chars = pivot;
              
              /* No need to worry about lossByte since ASCII can be converted
               * to any encoding.
               */
              converted += GSStringEncodingToUnicode (kCFStringEncodingASCII,
                chars, BUFFER_SIZE, &bytes, range.length, false, NULL);
              used += GSStringEncodingFromUnicode (encoding, (char*)buffer,
                maxBufLen, (const UniChar**)&chars, converted, 0,
                isExternalRepresentation, NULL);
              
              maxBufLen -= used;
              buffer += used;
            }
        }
    }
  
  if (usedBufLen)
    *usedBufLen = used;
  
  return converted;
}

void
CFStringGetCharacters (CFStringRef str, CFRange range, UniChar *buffer)
{
  CF_OBJC_FUNCDISPATCH2(_kCFStringTypeID, void, str,
    "getCharacters:range:", buffer, range);
  
  CFStringGetBytes (str, range, UTF16_ENCODING, 0, false, (UInt8*)buffer,
    range.length * sizeof(UniChar), NULL);
}

Boolean
CFStringGetCString (CFStringRef str, char *buffer, CFIndex bufferSize,
  CFStringEncoding encoding)
{
  CFIndex len = CFStringGetLength (str);
  CFIndex used;
  
  CF_OBJC_FUNCDISPATCH3(_kCFStringTypeID, Boolean, str,
    "getCString:maxLength:encoding:", buffer, bufferSize,
    CFStringConvertEncodingToNSStringEncoding(encoding));
  
  if (CFStringGetBytes (str, CFRangeMake(0, len), encoding, 0, false,
      (UInt8*)buffer, bufferSize, &used) == len && used <= len)
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
    CFStringGetSystemEncoding());
}


UniChar
CFStringGetCharacterAtIndex (CFStringRef str, CFIndex idx)
{
  CF_OBJC_FUNCDISPATCH1(_kCFStringTypeID, UniChar, str,
    "characterAtIndex:", idx);
  return CFStringIsUnicode(str) ? ((UniChar*)str->_contents)[idx] :
    ((char*)str->_contents)[idx];
}

CFIndex
CFStringGetLength (CFStringRef str)
{
  CF_OBJC_FUNCDISPATCH0(_kCFStringTypeID, CFIndex, str, "length");
  return str->_count;
}

CFRange
CFStringGetRangeOfComposedCharactersAtIndex (CFStringRef str,
  CFIndex theIndex)
{
  if (CFStringIsUnicode(str))
    {
      CFIndex len = 1;
      UniChar *characters = ((UniChar*)str->_contents) + theIndex;
      if (U16_IS_SURROGATE(*characters))
        {
          len = 2;
          if (U16_IS_SURROGATE_TRAIL(*characters))
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

double
CFStringGetDoubleValue (CFStringRef str)
{
  double d;
  Boolean success;
  CFNumberFormatterRef fmt;
  
  fmt = CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterDecimalStyle);
  if (fmt == NULL)
    return 0.0;
  
  success = CFNumberFormatterGetValueFromString(fmt, str, NULL,
    kCFNumberDoubleType, (void*)&d);
  
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
  
  success = CFNumberFormatterGetValueFromString(fmt, str, NULL,
    kCFNumberSInt32Type, (void*)&i);
  
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
  struct __CFMutableString *mStr = (struct __CFMutableString *)str;
  
  if (mStr->_capacity >= newCapacity)
    {
      if (oldContentBuffer)
        *oldContentBuffer = str->_contents;
      return true;
    }
  
  currentContents = mStr->_contents;
  
  newContents = CFAllocatorAllocate (mStr->_allocator,
    (newCapacity * sizeof(UniChar)), 0);
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
  
  new = (struct __CFMutableString *)_CFRuntimeCreateInstance (alloc,
    _kCFStringTypeID,
    sizeof(struct __CFMutableString) - sizeof (CFRuntimeBase),
    NULL);
  
  new->_capacity =
    maxLength < DEFAULT_STRING_CAPACITY ? DEFAULT_STRING_CAPACITY : maxLength;
  new->_allocator = alloc ? alloc : CFAllocatorGetDefault();
  new->_contents = CFAllocatorAllocate (new->_allocator,
    new->_capacity * sizeof(UniChar), 0);
  
  CFSTRING_INIT_MUTABLE(new);
  
  return (CFMutableStringRef)new;
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
  
  textLen = CFStringGetLength(str);
  capacity = textLen;
  if (maxLength > capacity)
    capacity = maxLength;
  new = (CFMutableStringRef)CFStringCreateMutable (alloc, capacity);
  
  /* An inline buffer is going to work well here... */
  CFStringInitInlineBuffer (str, &buffer, CFRangeMake(0, textLen));
  contents = (UniChar*)new->_contents;
  idx = 0;
  while (idx < textLen)
    {
      UniChar c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
      *(contents++) = c;
    }
  new->_count = textLen;
  
  CFSTRING_INIT_MUTABLE(new);
  
  return new;
}

CFMutableStringRef
CFStringCreateMutableWithExternalCharactersNoCopy (CFAllocatorRef alloc,
  UniChar *chars, CFIndex numChars, CFIndex capacity,
  CFAllocatorRef externalCharactersAllocator)
{
  return NULL; /* FIXME */
}

void
CFStringSetExternalCharactersNoCopy (CFMutableStringRef str, UniChar *chars,
  CFIndex length, CFIndex capacity)
{
  return; /* FIXME */
}

CFIndex
CFStringFindAndReplace (CFMutableStringRef str, CFStringRef stringToFind,
  CFStringRef replacementString, CFRange rangeToSearch,
  CFOptionFlags compareOptions)
{
  return 0; /* FIXME */
}

void
CFStringAppend (CFMutableStringRef str, CFStringRef appendString)
{
  CFStringReplace (str, CFRangeMake(CFStringGetLength(str), 0), appendString);
}

void
CFStringAppendCharacters (CFMutableStringRef str,
  const UniChar *chars, CFIndex numChars)
{
  CFIndex length;
  void *contents;
  
  length = str->_count;
  
  if (CFStringCheckCapacityAndGrow(str, (length + numChars), &contents)
      && contents != str->_contents)
    {
      memcpy (str->_contents, contents, length * sizeof(UniChar));
      CFAllocatorDeallocate (str->_deallocator, contents);
    }
  
  memcpy ((UniChar*)str->_contents + length, chars,
    numChars * sizeof(UniChar));
  str->_count = length + numChars;
}

void
CFStringAppendCString (CFMutableStringRef str, const char *cStr,
  CFStringEncoding encoding)
{
  UniChar uBuffer[BUFFER_SIZE];
  CFIndex cStrLen;
  CFIndex numChars;
  CFIndex neededChars;
  
  cStrLen = strlen (cStr);
  numChars = BUFFER_SIZE;
  do
    {
      numChars = GSStringEncodingToUnicode (encoding, uBuffer, numChars,
        &cStr, cStrLen, false, &neededChars);
      
      CFStringAppendCharacters (str, uBuffer, numChars);
    } while (numChars < neededChars);
}

void
CFStringAppendFormat (CFMutableStringRef str, CFDictionaryRef options,
  CFStringRef format, ...)
{
  va_list args;
  va_start (args, format);
  _CFStringAppendFormatAndArgumentsAux (str, NULL, options, format, args);
  va_end (args);
}

void
CFStringAppendFormatAndArguments (CFMutableStringRef str,
  CFDictionaryRef options, CFStringRef format, va_list args)
{
  _CFStringAppendFormatAndArgumentsAux (str, NULL, options, format, args);
}

void
CFStringDelete (CFMutableStringRef str, CFRange range)
{
  CFStringReplace (str, range, CFSTR(""));
}

void
CFStringInsert (CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr)
{
  CFStringReplace (str, CFRangeMake(idx, 0), insertedStr);
}

void
CFStringPad (CFMutableStringRef str, CFStringRef padString,
  CFIndex length, CFIndex indexIntoPad)
{
  if (padString == NULL && length < CFStringGetLength(str)) /* truncating */
    {
      ((UniChar*)str->_contents)[length] = 0x0000;
      str->_count = length;
      str->_hash = 0;
    }
  else
    {
      CFIndex padLength;
      UniChar *padContents;
      UniChar *contents;
      UniChar *end;
      
      if (!CFStringCheckCapacityAndGrow(str, length, (void**)&contents))
        return;
      if (contents != str->_contents)
        {
          memcpy (str->_contents, contents, length * sizeof(UniChar));
          CFAllocatorDeallocate (str->_deallocator, contents);
        }
      
      contents = ((UniChar*)str->_contents) + CFStringGetLength (str);
      end = ((UniChar*)str->_contents) + length;
      
      padLength = CFStringGetLength (padString);
      padContents = CFAllocatorAllocate (NULL, padLength * sizeof(UniChar), 0);
      CFStringGetCharacters (padString, CFRangeMake(0, padLength),
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
  CFStringInlineBuffer buffer;
  CFIndex textLength;
  CFIndex repLength;
  CFIndex idx;
  UniChar *contents;
  
  textLength = CFStringGetLength (str);
  repLength = CFStringGetLength (replacement);
  if (!CFRANGE_CHECK(textLength, range))
    return; /* out of range */
  
  if (repLength != range.length)
    {
      UniChar *moveFrom;
      UniChar *moveTo;
      UniChar *oldContents;
      CFIndex newLength;
      CFIndex moveLength;
      
      newLength = textLength - range.length + repLength;
      if (!CFStringCheckCapacityAndGrow(str, newLength, (void**)&oldContents))
        return;
      if (oldContents != str->_contents)
        {
          memcpy (str->_contents, oldContents,
            range.location * sizeof(UniChar));
        }
      moveFrom = (oldContents + range.location + range.length);
      moveTo = (((UniChar*)str->_contents) + range.location + repLength);
      moveLength = textLength - (range.location + range.length);
      memmove (moveTo, moveFrom, moveLength * sizeof(UniChar));
      
      if (oldContents != str->_contents)
        CFAllocatorDeallocate (str->_deallocator, (void*)oldContents);
      
      textLength = newLength;
    }
  
  CFStringInitInlineBuffer (replacement, &buffer, CFRangeMake(0, repLength));
  contents = (((UniChar*)str->_contents) + range.location);
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
  CFStringInlineBuffer buffer;
  CFIndex textLength;
  CFIndex idx;
  UniChar *contents;
  
  /* This function is very similar to CFStringReplace() but takes a few
     shortcuts and should be a little fast if all you need to do is replace
     the whole string. */
  textLength = CFStringGetLength (replacement);
  if (!CFStringCheckCapacityAndGrow(theString, textLength + 1, NULL))
    return;
  
  CFStringInitInlineBuffer (replacement, &buffer, CFRangeMake(0, textLength));
  contents = (UniChar*)theString->_contents;
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
    CFRangeMake(0, CFStringGetLength(str)), kCFCompareAnchored);
  CFStringFindAndReplace (str, trimString, NULL,
    CFRangeMake(0, CFStringGetLength(str)),
    kCFCompareBackwards | kCFCompareAnchored);
}

void
CFStringTrimWhitespace (CFMutableStringRef str)
{
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
  CFStringInitInlineBuffer (str, &buffer, CFRangeMake(0, textLength));
  
  idx = 0;
  c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
  while (u_isUWhiteSpace((UChar32)c) && idx < textLength)
    c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
  start = idx - 1;
  end = start;
  while (idx < textLength)
    {
      c = CFStringGetCharacterFromInlineBuffer (&buffer, idx++);
      /* reset the end point */
      if (!u_isUWhiteSpace((UChar32)c))
        end = idx;
    }
  
  newLength = end - start;
  contents = (UniChar*)str->_contents;
  contentsStart = (UniChar*)(contents + start);
  memmove (contents, contentsStart, newLength * sizeof(UniChar));
  
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
  char *localeID = NULL; /* FIXME */
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
            break;
          case _kCFStringUppercase:
            newLength = u_strToUpper (mStr->_contents, mStr->_capacity,
              oldContents, oldContentsLength, localeID, &err);
            break;
          case _kCFStringFold:
            optFlags = 0; /* FIXME */
            newLength = u_strFoldCase (mStr->_contents, mStr->_capacity,
              oldContents, oldContentsLength, optFlags, &err);
            break;
          default:
            return;
        }
    } while (err == U_BUFFER_OVERFLOW_ERROR
        && CFStringCheckCapacityAndGrow(str, newLength, (void**)&oldContents));
  if (U_FAILURE(err))
    return;
  
  mStr->_count = newLength;
  mStr->_hash = 0;
  
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
  CFStringCaseMap (str, locale, 0, _kCFStringLowercase);
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
CFStringNormalize (CFMutableStringRef str,
  CFStringNormalizationForm theForm)
{
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
  
  /* Make sure string isn't already normalized.  Use the quick check for
     speed. We still go through the normalization if the quick check does not
     return UNORM_YES. */
  oldContents = (UniChar*)CFStringGetCharactersPtr (str);
  oldContentsLength = CFStringGetLength (str);
  checkResult = unorm_quickCheck (oldContents, oldContentsLength, mode, &err);
  if (U_FAILURE(err) || checkResult == UNORM_YES)
    return;
  
  /* Works just like CFStringCaseMap() above... */
  mStr = (struct __CFMutableString *)str;
  do
    {
      newLength = unorm_normalize (mStr->_contents, mStr->_capacity, mode,
        0, oldContents, oldContentsLength, &err);
    } while (err == U_BUFFER_OVERFLOW_ERROR
        && CFStringCheckCapacityAndGrow(str, newLength, (void**)&oldContents));
  if (U_FAILURE(err))
    return;
  
  mStr->_count = newLength;
  
  if (oldContents != mStr->_contents)
    CFAllocatorDeallocate (mStr->_allocator, (void*)oldContents);
}

Boolean
CFStringTransform (CFMutableStringRef str, CFRange *range,
  CFStringRef transform, Boolean reverse)
{
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
  CFStringGetCharacters (transform, CFRangeMake(0, idLength), transID);
  utrans = utrans_openU (transID, idLength, dir, NULL, 0, NULL, &err);
  if (U_FAILURE(err))
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
  
  mStr = (struct __CFMutableString *)str;
  utrans_transUChars (utrans, mStr->_contents, (int32_t*)&mStr->_count,
    mStr->_capacity, start, (int32_t*)&limit, &err);
  if (U_FAILURE(err))
    return false;
  utrans_close (utrans);
  
  if (range)
    range->length = limit;
  
  return true;
}
