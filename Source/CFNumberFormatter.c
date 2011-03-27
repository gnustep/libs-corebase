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

#define BUFFER_SIZE 1024



struct __CFNumberFormatter
{
  CFRuntimeBase  _parent;
  UNumberFormat *_fmt;
  CFLocaleRef    _locale;
  CFNumberFormatterStyle _style;
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



CFNumberFormatterRef
CFNumberFormatterCreate (CFAllocatorRef allocator, CFLocaleRef locale,
  CFNumberFormatterStyle style)
{
  struct __CFNumberFormatter *new;
  CFStringRef localeIdent;
  char cLocale[ULOC_FULLNAME_CAPACITY];
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
  UChar buffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  len = CFStringGetLength (currencyCode);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  CFStringGetCharacters (currencyCode, CFRangeMake(0, len), buffer);
  
  *defaultFractionDigits = ucurr_getDefaultFractionDigits (buffer, &err);
  *roundingIncrement = ucurr_getRoundingIncrement (buffer, &err);
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
  
  return NULL;
}

CFStringRef
CFNumberFormatterGetFormat (CFNumberFormatterRef formatter)
{
  return NULL;
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
