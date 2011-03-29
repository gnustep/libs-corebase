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

static const CFRuntimeClass CFNumberFormatterClass =
{
  0,
  "CFNumberFormatter",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFNumberFormatterInitialize (void)
{
  _kCFNumberFormatterTypeID = _CFRuntimeRegisterClass(&CFNumberFormatterClass);
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
              return (CFTypeRef)kCFBooleanTrue;
            case 1:
              return (CFTypeRef)kCFBooleanFalse;
          }
      case UNUM_ROUNDING_INCREMENT:
        d = unum_getDoubleAttribute (fmt->_fmt, attrib);
        return (CFTypeRef)CFNumberCreate (NULL, kCFNumberDoubleType, (const void*)&d);
      default:
        num = unum_getAttribute (fmt->_fmt, attrib);
        return (CFTypeRef)CFNumberCreate (NULL, kCFNumberSInt32Type, (const void*)&num);
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
      CFRelease ((CFTypeRef)new);
      return NULL;
    }
  new->_locale = CFRetain((CFTypeRef)locale);
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
CFNumberFormatterSetFormat (CFNumberFormatterRef formatter,
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
  
  unum_applyPattern (formatter->_fmt, false, buffer, len, NULL, &err);
  if (U_FAILURE(err))
    return;
  
  if (formatter->_format)
    CFRelease ((CFTypeRef)formatter->_format);
  formatter->_format = CFRetain ((CFTypeRef)formatString);
}

void
CFNumberFormatterSetProperty (CFNumberFormatterRef formatter,
  CFStringRef key, CFTypeRef value)
{
  if (key == kCFNumberFormatterCurrencyCode) // CFString
    {
    }
  else if (key == kCFNumberFormatterDecimalSeparator) // CFString
    {
    }
  else if (key == kCFNumberFormatterCurrencyDecimalSeparator) // CFString
    {
    }
  else if (key == kCFNumberFormatterAlwaysShowDecimalSeparator) // CFBoolean
    {
    }
  else if (key == kCFNumberFormatterGroupingSeparator) // CFString
    {
    }
  else if (key == kCFNumberFormatterUseGroupingSeparator) // CFBoolean
    {
    }
  else if (key == kCFNumberFormatterPercentSymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterZeroSymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterNaNSymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterInfinitySymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterMinusSign) // CFString
    {
    }
  else if (key == kCFNumberFormatterPlusSign) // CFString
    {
    }
  else if (key == kCFNumberFormatterCurrencySymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterExponentSymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterMinIntegerDigits) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterMaxIntegerDigits) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterMinFractionDigits) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterMaxFractionDigits) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterGroupingSize) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterSecondaryGroupingSize) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterRoundingMode) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterRoundingIncrement) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterFormatWidth) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterPaddingPosition) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterPaddingCharacter) // CFString
    {
    }
  else if (key == kCFNumberFormatterDefaultFormat) // CFString
    {
      // Default format cannot be overridden.
      return;
    }
  else if (key == kCFNumberFormatterMultiplier) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterPositivePrefix) // CFString
    {
    }
  else if (key == kCFNumberFormatterPositiveSuffix) // CFString
    {
    }
  else if (key == kCFNumberFormatterNegativePrefix) // CFString
    {
    }
  else if (key == kCFNumberFormatterNegativeSuffix) // CFString
    {
    }
  else if (key == kCFNumberFormatterPerMillSymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterInternationalCurrencySymbol) // CFString
    {
    }
  else if (key == kCFNumberFormatterCurrencyGroupingSeparator) // CFString
    {
    }
  else if (key == kCFNumberFormatterIsLenient) // CFBoolean
    {
    }
  else if (key == kCFNumberFormatterUseSignificantDigits) // CFBoolean
    {
    }
  else if (key == kCFNumberFormatterMinSignificantDigits) // CFNumber
    {
    }
  else if (key == kCFNumberFormatterMaxSignificantDigits) // CFNumber
    {
    }
}

CFNumberRef
CFNumberFormatterCreateNumberFromString (CFAllocatorRef allocator,
  CFNumberFormatterRef formatter, CFStringRef string, CFRange *rangep,
  CFOptionFlags options)
{
  return NULL;
}

CFStringRef
CFNumberFormatterCreateStringWithNumber (CFAllocatorRef allocator,
  CFNumberFormatterRef formatter, CFNumberRef number)
{
  return NULL;
}

CFStringRef
CFNumberFormatterCreateStringWithValue (CFAllocatorRef allocator,
  CFNumberFormatterRef formatter, CFNumberType numberType,
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
CFNumberFormatterGetValueFromString (CFNumberFormatterRef formatter,
  CFStringRef string, CFRange *rangep, CFNumberType numberType, void *valuePtr)
{
  return false;
}

CFTypeRef
CFNumberFormatterCopyProperty (CFNumberFormatterRef formatter,
  CFStringRef key)
{
  if (key == kCFNumberFormatterCurrencyCode) // CFString
    return (CFTypeRef)CFNumberFormatterCopyTextAttribute (formatter,
      UNUM_CURRENCY_CODE);
  else if (key == kCFNumberFormatterDecimalSeparator) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_DECIMAL_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterCurrencyDecimalSeparator) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterAlwaysShowDecimalSeparator) // CFBoolean
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_DECIMAL_ALWAYS_SHOWN);
  else if (key == kCFNumberFormatterGroupingSeparator) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_GROUPING_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterUseGroupingSeparator) // CFBoolean
    return CFNumberFormatterCopyAttribute (formatter, UNUM_GROUPING_USED);
  else if (key == kCFNumberFormatterPercentSymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_PERCENT_SYMBOL);
  else if (key == kCFNumberFormatterZeroSymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_ZERO_DIGIT_SYMBOL);
  else if (key == kCFNumberFormatterNaNSymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter, UNUM_NAN_SYMBOL);
  else if (key == kCFNumberFormatterInfinitySymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_INFINITY_SYMBOL);
  else if (key == kCFNumberFormatterMinusSign) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_MINUS_SIGN_SYMBOL);
  else if (key == kCFNumberFormatterPlusSign) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_PLUS_SIGN_SYMBOL);
  else if (key == kCFNumberFormatterCurrencySymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_CURRENCY_SYMBOL);
  else if (key == kCFNumberFormatterExponentSymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_EXPONENTIAL_SYMBOL);
  else if (key == kCFNumberFormatterMinIntegerDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_MAX_INTEGER_DIGITS);
  else if (key == kCFNumberFormatterMaxIntegerDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_MAX_INTEGER_DIGITS);
  else if (key == kCFNumberFormatterMinFractionDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_MIN_FRACTION_DIGITS);
  else if (key == kCFNumberFormatterMaxFractionDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_MAX_FRACTION_DIGITS);
  else if (key == kCFNumberFormatterGroupingSize) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_GROUPING_SIZE);
  else if (key == kCFNumberFormatterSecondaryGroupingSize) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_SECONDARY_GROUPING_SIZE);
  else if (key == kCFNumberFormatterRoundingMode) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_ROUNDING_MODE);
  else if (key == kCFNumberFormatterRoundingIncrement) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_ROUNDING_INCREMENT);
  else if (key == kCFNumberFormatterFormatWidth) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_FORMAT_WIDTH);
  else if (key == kCFNumberFormatterPaddingPosition) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_PADDING_POSITION);
  else if (key == kCFNumberFormatterPaddingCharacter) // CFString
    return (CFTypeRef)CFNumberFormatterCopyTextAttribute (formatter,
      UNUM_PADDING_CHARACTER);
  else if (key == kCFNumberFormatterDefaultFormat) // CFString
    return CFRetain ((CFTypeRef)formatter->_defaultFormat);
  else if (key == kCFNumberFormatterMultiplier) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter, UNUM_MULTIPLIER);
  else if (key == kCFNumberFormatterPositivePrefix) // CFString
    return (CFTypeRef)CFNumberFormatterCopyTextAttribute (formatter,
      UNUM_POSITIVE_PREFIX);
  else if (key == kCFNumberFormatterPositiveSuffix) // CFString
    return (CFTypeRef)CFNumberFormatterCopyTextAttribute (formatter,
      UNUM_POSITIVE_SUFFIX);
  else if (key == kCFNumberFormatterNegativePrefix) // CFString
    return (CFTypeRef)CFNumberFormatterCopyTextAttribute (formatter,
      UNUM_NEGATIVE_PREFIX);
  else if (key == kCFNumberFormatterNegativeSuffix) // CFString
    return (CFTypeRef)CFNumberFormatterCopyTextAttribute (formatter,
      UNUM_POSITIVE_SUFFIX);
  else if (key == kCFNumberFormatterPerMillSymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_PERMILL_SYMBOL);
  else if (key == kCFNumberFormatterInternationalCurrencySymbol) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_INTL_CURRENCY_SYMBOL);
  else if (key == kCFNumberFormatterCurrencyGroupingSeparator) // CFString
    return (CFTypeRef)CFNumberFormatterCopySymbol (formatter,
      UNUM_MONETARY_SEPARATOR_SYMBOL);
  else if (key == kCFNumberFormatterIsLenient) // CFBoolean
    return CFNumberFormatterCopyAttribute (formatter, UNUM_LENIENT_PARSE);
  else if (key == kCFNumberFormatterUseSignificantDigits) // CFBoolean
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_SIGNIFICANT_DIGITS_USED);
  else if (key == kCFNumberFormatterMinSignificantDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_MIN_SIGNIFICANT_DIGITS);
  else if (key == kCFNumberFormatterMaxSignificantDigits) // CFNumber
    return CFNumberFormatterCopyAttribute (formatter,
      UNUM_MAX_SIGNIFICANT_DIGITS);
  
  return NULL;
}

CFStringRef
CFNumberFormatterGetFormat (CFNumberFormatterRef formatter)
{
  return (formatter->_format) ? formatter->_format : formatter->_defaultFormat;
}

CFLocaleRef
CFNumberFormatterGetLocale (CFNumberFormatterRef formatter)
{
  return formatter->_locale;
}

CFNumberFormatterStyle
CFNumberFormatterGetStyle (CFNumberFormatterRef formatter)
{
  return formatter->_style;
}

CFTypeID
CFNumberFormatterGetTypeID (void)
{
  return _kCFNumberFormatterTypeID;
}
