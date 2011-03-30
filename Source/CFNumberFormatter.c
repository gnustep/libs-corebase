/* CFNumberFormatter.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFRuntime.h"

#include "CoreFoundation/CFNumberFormatter.h"

#include <pthread.h>
#include <unicode/uloc.h> // for ULOC_FULLNAME_CAPACITY
#include <unicode/ucurr.h>
#include <unicode/unum.h>

#define BUFFER_SIZE 512



struct __CFNumberFormatter
{
  CFRuntimeBase  _parent;
  UNumberFormat *_fmt;
  CFLocaleRef    _locale;
  CFNumberFormatterStyle _style;
  CFStringRef    _defaultFormat;
  CFStringRef    _format;
};

static CFTypeID _kCFNumberFormatterTypeID;

static CFTypeRef CFNumberFormatterCopy (CFAllocatorRef alloc, CFTypeRef cf)
{
  UErrorCode err = U_ZERO_ERROR;
  CFNumberFormatterRef o = (CFNumberFormatterRef)cf;
  struct __CFNumberFormatter *copy =
    (struct __CFNumberFormatter *)_CFRuntimeCreateInstance (alloc,
      CFNumberFormatterGetTypeID(),
      sizeof(struct __CFNumberFormatter) - sizeof(CFRuntimeBase),
      NULL);
  
  copy->_fmt = unum_clone (o->_fmt, &err);
  if (U_FAILURE(err))
    {
      CFRelease (copy);
      return NULL;
    }
  copy->_locale = CFRetain (o->_locale);
  copy->_style = o->_style;
  copy->_defaultFormat = CFRetain (o->_defaultFormat);
  copy->_format = CFRetain (o->_format);
  
  return copy;
}

static void CFNumberFormatterFinalize (CFTypeRef cf)
{
  struct __CFNumberFormatter *o = (struct __CFNumberFormatter *)cf;
  unum_close (o->_fmt);
  CFRelease (o->_defaultFormat);
  CFRelease (o->_format);
}

static const CFRuntimeClass CFNumberFormatterClass =
{
  0,
  "CFNumberFormatter",
  NULL,
  CFNumberFormatterCopy,
  CFNumberFormatterFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFNumberFormatterInitialize (void)
{
  _kCFNumberFormatterTypeID = _CFRuntimeRegisterClass(&CFNumberFormatterClass);
}



static inline void
CFNumberFormatterSetAttribute (CFNumberFormatterRef fmt,
  UNumberFormatAttribute attrib, CFTypeRef num)
{
  int32_t value;
  double d;
  switch (attrib)
    {
      case UNUM_GROUPING_USED:
      case UNUM_DECIMAL_ALWAYS_SHOWN:
      case UNUM_SIGNIFICANT_DIGITS_USED:
      case UNUM_LENIENT_PARSE:
        value = (num == kCFBooleanTrue) ? true : false;
        unum_setAttribute (fmt->_fmt, attrib, value);
        return;
      case UNUM_ROUNDING_INCREMENT:
        CFNumberGetValue (num, kCFNumberDoubleType, &d);
        unum_setDoubleAttribute (fmt->_fmt, UNUM_ROUNDING_INCREMENT, d);
        return;
      default:
        CFNumberGetValue (num, kCFNumberSInt32Type, &value);
        unum_setAttribute (fmt->_fmt, attrib, value);
        return;
    }
}

static inline void
CFNumberFormatterSetTextAttribute (CFNumberFormatterRef fmt,
  UNumberFormatTextAttribute attrib, CFStringRef str)
{
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  len = CFStringGetLength (str);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  CFStringGetCharacters (str, CFRangeMake(0, len), ubuffer);
  
  unum_setTextAttribute (fmt->_fmt, attrib, ubuffer, len, &err);
}

static inline void
CFNumberFormatterSetSymbol (CFNumberFormatterRef fmt,
  UNumberFormatSymbol symbol, CFStringRef str)
{
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  len = CFStringGetLength (str);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  CFStringGetCharacters (str, CFRangeMake(0, len), ubuffer);
  
  unum_setSymbol (fmt->_fmt, symbol, ubuffer, len, &err);
}

static inline CFTypeRef
CFNumberFormatterCopyAttribute (CFNumberFormatterRef fmt,
  UNumberFormatAttribute attrib)
{
  int32_t num;
  double d;
  switch (attrib)
    {
      case UNUM_GROUPING_USED:
      case UNUM_DECIMAL_ALWAYS_SHOWN:
      case UNUM_SIGNIFICANT_DIGITS_USED:
      case UNUM_LENIENT_PARSE:
        num = unum_getAttribute (fmt->_fmt, attrib);
        switch (num)
          {
            case 0:
              return kCFBooleanTrue;
            case 1:
              return kCFBooleanFalse;
          }
      case UNUM_ROUNDING_INCREMENT:
        d = unum_getDoubleAttribute (fmt->_fmt, attrib);
        return CFNumberCreate (NULL, kCFNumberDoubleType, (const void*)&d);
      default:
        num = unum_getAttribute (fmt->_fmt, attrib);
        return CFNumberCreate (NULL, kCFNumberSInt32Type, (const void*)&num);
    }
}

static inline CFStringRef
CFNumberFormatterCopyTextAttribute (CFNumberFormatterRef fmt,
  UNumberFormatTextAttribute attrib)
{
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  len = unum_getTextAttribute (fmt->_fmt, attrib, ubuffer, BUFFER_SIZE, &err);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  return CFStringCreateWithCharacters (NULL, ubuffer, len);
}

static inline CFStringRef
CFNumberFormatterCopySymbol (CFNumberFormatterRef fmt,
  UNumberFormatSymbol symbol)
{
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  len = unum_getSymbol (fmt->_fmt, symbol, ubuffer, BUFFER_SIZE, &err);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  return CFStringCreateWithCharacters (NULL, ubuffer, len);
}



CFNumberFormatterRef
CFNumberFormatterCreate (CFAllocatorRef allocator, CFLocaleRef locale,
  CFNumberFormatterStyle style)
{
  struct __CFNumberFormatter *new;
  CFStringRef localeIdent;
  char cLocale[ULOC_FULLNAME_CAPACITY];
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  if (locale == NULL)
    locale = CFLocaleGetSystem ();
  localeIdent = CFLocaleGetIdentifier (locale);
  if (!CFStringGetCString (localeIdent, cLocale, ULOC_FULLNAME_CAPACITY,
      CFStringGetSystemEncoding()))
    return NULL;
  
  new = (struct __CFNumberFormatter *)_CFRuntimeCreateInstance (allocator,
    CFNumberFormatterGetTypeID(),
    sizeof(struct __CFNumberFormatter) - sizeof(CFRuntimeBase),
    NULL);
  if (new == NULL)
    return NULL;
  
  // Good news, UNumberFormatStyle and NSNumberFormatterStyle match.
  new->_fmt = unum_open (style, NULL, 0, cLocale, NULL, &err);
  if (U_FAILURE(err))
    {
      CFRelease (new);
      return NULL;
    }
  new->_locale = CFRetain(locale);
  new->_style = style;
  
  if (style == kCFNumberFormatterNoStyle)
    {
      ubuffer[0] = '#';
      
      new->_defaultFormat =
        CFStringCreateWithCharacters (allocator, ubuffer, 1);
      unum_applyPattern (new->_fmt, false, ubuffer, 1, NULL, &err);
    }
  else
    {
      len = unum_toPattern (new->_fmt, false, ubuffer, BUFFER_SIZE, &err);
      if (len > BUFFER_SIZE)
        len = BUFFER_SIZE;
      new->_defaultFormat = CFStringCreateWithCharacters (allocator,
        ubuffer, len);
    }
  
  return new;
}

void
CFNumberFormatterSetFormat (CFNumberFormatterRef fmt,
  CFStringRef formatString)
{
  UniChar buffer[BUFFER_SIZE];
  CFIndex len;
  UErrorCode err = U_ZERO_ERROR;
  
  len = CFStringGetLength (formatString);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  /* Apple recommends using an in-line buffer if there are a lot of
     characters.  I'll assume this won't be a problem here. */
  CFStringGetCharacters (formatString, CFRangeMake(0, len), buffer);
  
  unum_applyPattern (fmt->_fmt, false, buffer, len, NULL, &err);
  if (U_FAILURE(err))
    return;
  
  if (fmt->_format)
    CFRelease (fmt->_format);
  fmt->_format = CFRetain (formatString);
}

void
CFNumberFormatterSetProperty (CFNumberFormatterRef fmt,
  CFStringRef key, CFTypeRef value)
{
  if (key == kCFNumberFormatterCurrencyCode) // CFString
    CFNumberFormatterSetTextAttribute (fmt, UNUM_CURRENCY_CODE, value);
  else if (key == kCFNumberFormatterDecimalSeparator) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_DECIMAL_SEPARATOR_SYMBOL, value);
  else if (key == kCFNumberFormatterCurrencyDecimalSeparator) // CFString
    CFNumberFormatterSetSymbol (fmt,
      UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL, value);
  else if (key == kCFNumberFormatterAlwaysShowDecimalSeparator) // CFBoolean
    CFNumberFormatterSetAttribute (fmt, UNUM_DECIMAL_ALWAYS_SHOWN, value);
  else if (key == kCFNumberFormatterGroupingSeparator) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_GROUPING_SEPARATOR_SYMBOL, value);
  else if (key == kCFNumberFormatterUseGroupingSeparator) // CFBoolean
    CFNumberFormatterSetAttribute (fmt, UNUM_GROUPING_USED, value);
  else if (key == kCFNumberFormatterPercentSymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_PERCENT_SYMBOL, value);
  else if (key == kCFNumberFormatterZeroSymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_ZERO_DIGIT_SYMBOL, value);
  else if (key == kCFNumberFormatterNaNSymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_NAN_SYMBOL, value);
  else if (key == kCFNumberFormatterInfinitySymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_INFINITY_SYMBOL, value);
  else if (key == kCFNumberFormatterMinusSign) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_MINUS_SIGN_SYMBOL, value);
  else if (key == kCFNumberFormatterPlusSign) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_PLUS_SIGN_SYMBOL, value);
  else if (key == kCFNumberFormatterCurrencySymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_CURRENCY_SYMBOL, value);
  else if (key == kCFNumberFormatterExponentSymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_EXPONENTIAL_SYMBOL, value);
  else if (key == kCFNumberFormatterMinIntegerDigits) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MAX_INTEGER_DIGITS, value);
  else if (key == kCFNumberFormatterMaxIntegerDigits) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MAX_INTEGER_DIGITS, value);
  else if (key == kCFNumberFormatterMinFractionDigits) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MIN_FRACTION_DIGITS, value);
  else if (key == kCFNumberFormatterMaxFractionDigits) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MAX_FRACTION_DIGITS, value);
  else if (key == kCFNumberFormatterGroupingSize) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_GROUPING_SIZE, value);
  else if (key == kCFNumberFormatterSecondaryGroupingSize) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_SECONDARY_GROUPING_SIZE, value);
  else if (key == kCFNumberFormatterRoundingMode) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_ROUNDING_MODE, value);
  else if (key == kCFNumberFormatterRoundingIncrement) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_ROUNDING_INCREMENT, value);
  else if (key == kCFNumberFormatterFormatWidth) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_FORMAT_WIDTH, value);
  else if (key == kCFNumberFormatterPaddingPosition) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_PADDING_POSITION, value);
  else if (key == kCFNumberFormatterPaddingCharacter) // CFString
    CFNumberFormatterSetTextAttribute (fmt, UNUM_PADDING_CHARACTER, value);
  else if (key == kCFNumberFormatterMultiplier) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MULTIPLIER, value);
  else if (key == kCFNumberFormatterPositivePrefix) // CFString
    CFNumberFormatterSetTextAttribute (fmt, UNUM_POSITIVE_PREFIX, value);
  else if (key == kCFNumberFormatterPositiveSuffix) // CFString
    CFNumberFormatterSetTextAttribute (fmt, UNUM_POSITIVE_SUFFIX, value);
  else if (key == kCFNumberFormatterNegativePrefix) // CFString
    CFNumberFormatterSetTextAttribute (fmt, UNUM_NEGATIVE_PREFIX, value);
  else if (key == kCFNumberFormatterNegativeSuffix) // CFString
    CFNumberFormatterSetTextAttribute (fmt, UNUM_POSITIVE_SUFFIX, value);
  else if (key == kCFNumberFormatterPerMillSymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_PERMILL_SYMBOL, value);
  else if (key == kCFNumberFormatterInternationalCurrencySymbol) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_INTL_CURRENCY_SYMBOL, value);
  else if (key == kCFNumberFormatterCurrencyGroupingSeparator) // CFString
    CFNumberFormatterSetSymbol (fmt, UNUM_MONETARY_SEPARATOR_SYMBOL, value);
  else if (key == kCFNumberFormatterIsLenient) // CFBoolean
    CFNumberFormatterSetAttribute (fmt, UNUM_LENIENT_PARSE, value);
  else if (key == kCFNumberFormatterUseSignificantDigits) // CFBoolean
    CFNumberFormatterSetAttribute (fmt, UNUM_SIGNIFICANT_DIGITS_USED, value);
  else if (key == kCFNumberFormatterMinSignificantDigits) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MIN_SIGNIFICANT_DIGITS, value);
  else if (key == kCFNumberFormatterMaxSignificantDigits) // CFNumber
    CFNumberFormatterSetAttribute (fmt, UNUM_MAX_SIGNIFICANT_DIGITS, value);
  
  return;
}

CFNumberRef
CFNumberFormatterCreateNumberFromString (CFAllocatorRef allocator,
  CFNumberFormatterRef fmt, CFStringRef str, CFRange *rangep,
  CFOptionFlags options)
{
  CFNumberRef result;
  CFNumberType type;
  UInt8 value[sizeof(double)]; // FIXME: what's the largest value we have?
  
  type = (options == kCFNumberFormatterParseIntegersOnly) ?
    kCFNumberSInt64Type : kCFNumberFloat64Type;
  if (CFNumberFormatterGetValueFromString (fmt, str, rangep, type, value))
    result = CFNumberCreate (allocator, type, (const void *)value);
  else
    result = NULL;
  
  return result;
}

CFStringRef
CFNumberFormatterCreateStringWithNumber (CFAllocatorRef allocator,
  CFNumberFormatterRef fmt, CFNumberRef number)
{
  UInt8 value[sizeof(double)]; // FIXME: ???
  CFNumberType type = CFNumberGetType (number);
  
  if (CFNumberGetValue (number, type, value))
    return CFNumberFormatterCreateStringWithValue (allocator, fmt, type,
      (const void*)value);
  
  return NULL;
}

CFStringRef
CFNumberFormatterCreateStringWithValue (CFAllocatorRef allocator,
  CFNumberFormatterRef fmt, CFNumberType numberType,
  const void *valuePtr)
{
  return NULL;
}

Boolean
CFNumberFormatterGetDecimalInfoForCurrencyCode (CFStringRef currencyCode,
  int32_t *defaultFractionDigits, double *roundingIncrement)
{
  CFIndex len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  len = CFStringGetLength (currencyCode);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  CFStringGetCharacters (currencyCode, CFRangeMake(0, len), ubuffer);
  
  *defaultFractionDigits = ucurr_getDefaultFractionDigits (ubuffer, &err);
  *roundingIncrement = ucurr_getRoundingIncrement (ubuffer, &err);
  if (U_SUCCESS(err))
    return true;
  
  return false;
}

Boolean
CFNumberFormatterGetValueFromString (CFNumberFormatterRef fmt,
  CFStringRef string, CFRange *rangep, CFNumberType numberType, void *valuePtr)
{
  CFRange parseRange;
  UniChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  int32_t parsePos;
  double  dresult = 0.0;
  int64_t iresult = 0; // Keep compiler happy
  
  if (rangep)
    {
      parseRange = *rangep;
    }
  else
    {
      parseRange.location = 0;
      parseRange.length = CFStringGetLength (string);
    }
  if (parseRange.length > BUFFER_SIZE)
    parseRange.length = BUFFER_SIZE;
  CFStringGetCharacters (string, parseRange, ubuffer);
  
  parsePos = 0;
  switch (numberType)
    {
      case kCFNumberSInt8Type:
      case kCFNumberSInt16Type:
      case kCFNumberSInt32Type:
      case kCFNumberSInt64Type:
      case kCFNumberCharType:
      case kCFNumberShortType:
      case kCFNumberIntType:
      case kCFNumberLongType:
      case kCFNumberLongLongType:
      case kCFNumberCFIndexType:
      case kCFNumberNSIntegerType:
        iresult = unum_parseInt64 (fmt->_fmt, ubuffer, parseRange.length,
          &parsePos, &err);
        break;
      default: // must be a float type
        dresult = unum_parseDouble (fmt->_fmt, ubuffer, parseRange.length,
          &parsePos, &err);
    }
  if (U_FAILURE(err))
    return false;
  
  switch (numberType)
    {
      case kCFNumberSInt8Type:
        *(SInt8*)valuePtr = (SInt8)iresult;
        break;
      case kCFNumberSInt16Type:
        *(SInt16*)valuePtr = (SInt16)iresult;
        break;
      case kCFNumberSInt32Type:
        *(SInt32*)valuePtr = (SInt32)iresult;
        break;
      case kCFNumberSInt64Type:
        *(SInt64*)valuePtr = (SInt64)iresult;
        break;
      case kCFNumberCharType:
        *(char*)valuePtr = (char)iresult;
        break;
      case kCFNumberShortType:
        *(short*)valuePtr = (short)iresult;
        break;
      case kCFNumberIntType:
        *(int*)valuePtr = (int)iresult;
        break;
      case kCFNumberLongType:
        *(long*)valuePtr = (long)iresult;
        break;
      case kCFNumberLongLongType:
        *(long long*)valuePtr = (long long)iresult;
        break;
      case kCFNumberCFIndexType:
        *(CFIndex*)valuePtr = (CFIndex)iresult;
        break;
      case kCFNumberNSIntegerType: // FIXME: This isn't defined in CF, so guess
        *(CFIndex*)valuePtr = (CFIndex)iresult;
        break;
      case kCFNumberFloat32Type:
        *(Float32*)valuePtr = (Float32)dresult;
        break;
      case kCFNumberFloat64Type:
        *(Float64*)valuePtr = (Float64)dresult;
        break;
      case kCFNumberFloatType:
        *(float*)valuePtr = (float)dresult;
        break;
      case kCFNumberDoubleType:
        *(double*)valuePtr = (double)dresult;
        break;
      case kCFNumberCGFloatType: // FIXME: Guess here, too
        *(double*)valuePtr = (double)dresult;
        break;
    }
  
  if (rangep)
    rangep->length = parsePos;
  
  return true;
}

CFTypeRef
CFNumberFormatterCopyProperty (CFNumberFormatterRef fmt,
  CFStringRef key)
{
  if (key == kCFNumberFormatterCurrencyCode) // CFString
    return CFNumberFormatterCopyTextAttribute (fmt, UNUM_CURRENCY_CODE);
  else if (key == kCFNumberFormatterDecimalSeparator) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_DECIMAL_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterCurrencyDecimalSeparator) // CFString
    return CFNumberFormatterCopySymbol (fmt,
      UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterAlwaysShowDecimalSeparator) // CFBoolean
    return CFNumberFormatterCopyAttribute (fmt, UNUM_DECIMAL_ALWAYS_SHOWN);
  else if (key == kCFNumberFormatterGroupingSeparator) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_GROUPING_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterUseGroupingSeparator) // CFBoolean
    return CFNumberFormatterCopyAttribute (fmt, UNUM_GROUPING_USED);
  else if (key == kCFNumberFormatterPercentSymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_PERCENT_SYMBOL);
  else if (key == kCFNumberFormatterZeroSymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_ZERO_DIGIT_SYMBOL);
  else if (key == kCFNumberFormatterNaNSymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_NAN_SYMBOL);
  else if (key == kCFNumberFormatterInfinitySymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_INFINITY_SYMBOL);
  else if (key == kCFNumberFormatterMinusSign) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_MINUS_SIGN_SYMBOL);
  else if (key == kCFNumberFormatterPlusSign) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_PLUS_SIGN_SYMBOL);
  else if (key == kCFNumberFormatterCurrencySymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_CURRENCY_SYMBOL);
  else if (key == kCFNumberFormatterExponentSymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_EXPONENTIAL_SYMBOL);
  else if (key == kCFNumberFormatterMinIntegerDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MAX_INTEGER_DIGITS);
  else if (key == kCFNumberFormatterMaxIntegerDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MAX_INTEGER_DIGITS);
  else if (key == kCFNumberFormatterMinFractionDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MIN_FRACTION_DIGITS);
  else if (key == kCFNumberFormatterMaxFractionDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MAX_FRACTION_DIGITS);
  else if (key == kCFNumberFormatterGroupingSize) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_GROUPING_SIZE);
  else if (key == kCFNumberFormatterSecondaryGroupingSize) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_SECONDARY_GROUPING_SIZE);
  else if (key == kCFNumberFormatterRoundingMode) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_ROUNDING_MODE);
  else if (key == kCFNumberFormatterRoundingIncrement) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_ROUNDING_INCREMENT);
  else if (key == kCFNumberFormatterFormatWidth) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_FORMAT_WIDTH);
  else if (key == kCFNumberFormatterPaddingPosition) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_PADDING_POSITION);
  else if (key == kCFNumberFormatterPaddingCharacter) // CFString
    return CFNumberFormatterCopyTextAttribute (fmt, UNUM_PADDING_CHARACTER);
  else if (key == kCFNumberFormatterDefaultFormat) // CFString
    return CFRetain (fmt->_defaultFormat);
  else if (key == kCFNumberFormatterMultiplier) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MULTIPLIER);
  else if (key == kCFNumberFormatterPositivePrefix) // CFString
    return CFNumberFormatterCopyTextAttribute (fmt, UNUM_POSITIVE_PREFIX);
  else if (key == kCFNumberFormatterPositiveSuffix) // CFString
    return CFNumberFormatterCopyTextAttribute (fmt, UNUM_POSITIVE_SUFFIX);
  else if (key == kCFNumberFormatterNegativePrefix) // CFString
    return CFNumberFormatterCopyTextAttribute (fmt, UNUM_NEGATIVE_PREFIX);
  else if (key == kCFNumberFormatterNegativeSuffix) // CFString
    return CFNumberFormatterCopyTextAttribute (fmt, UNUM_POSITIVE_SUFFIX);
  else if (key == kCFNumberFormatterPerMillSymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_PERMILL_SYMBOL);
  else if (key == kCFNumberFormatterInternationalCurrencySymbol) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_INTL_CURRENCY_SYMBOL);
  else if (key == kCFNumberFormatterCurrencyGroupingSeparator) // CFString
    return CFNumberFormatterCopySymbol (fmt, UNUM_MONETARY_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterIsLenient) // CFBoolean
    return CFNumberFormatterCopyAttribute (fmt, UNUM_LENIENT_PARSE);
  else if (key == kCFNumberFormatterUseSignificantDigits) // CFBoolean
    return CFNumberFormatterCopyAttribute (fmt, UNUM_SIGNIFICANT_DIGITS_USED);
  else if (key == kCFNumberFormatterMinSignificantDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MIN_SIGNIFICANT_DIGITS);
  else if (key == kCFNumberFormatterMaxSignificantDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (fmt, UNUM_MAX_SIGNIFICANT_DIGITS);
  
  return NULL;
}

CFStringRef
CFNumberFormatterGetFormat (CFNumberFormatterRef fmt)
{
  return (fmt->_format) ? fmt->_format : fmt->_defaultFormat;
}

CFLocaleRef
CFNumberFormatterGetLocale (CFNumberFormatterRef fmt)
{
  return fmt->_locale;
}

CFNumberFormatterStyle
CFNumberFormatterGetStyle (CFNumberFormatterRef fmt)
{
  return fmt->_style;
}

CFTypeID
CFNumberFormatterGetTypeID (void)
{
  return _kCFNumberFormatterTypeID;
}
