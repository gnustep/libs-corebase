/* CFNumberFormatter.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
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
#include "GSPrivate.h"

#include "CoreFoundation/CFNumberFormatter.h"

#include <unicode/ucurr.h>
#include <unicode/unum.h>

#define BUFFER_SIZE 512

CONST_STRING_DECL(kCFNumberFormatterCurrencyCode,
  "kCFNumberFormatterCurrencyCode");
CONST_STRING_DECL(kCFNumberFormatterDecimalSeparator,
  "kCFNumberFormatterDecimalSeparator");
CONST_STRING_DECL(kCFNumberFormatterCurrencyDecimalSeparator,
  "kCFNumberFormatterCurrencyDecimalSeparator");
CONST_STRING_DECL(kCFNumberFormatterAlwaysShowDecimalSeparator,
  "kCFNumberFormatterAlwaysShowDecimalSeparator");
CONST_STRING_DECL(kCFNumberFormatterGroupingSeparator,
  "kCFNumberFormatterGroupingSeparator");
CONST_STRING_DECL(kCFNumberFormatterUseGroupingSeparator,
  "kCFNumberFormatterUseGroupingSeparator");
CONST_STRING_DECL(kCFNumberFormatterPercentSymbol,
  "kCFNumberFormatterPercentSymbol");
CONST_STRING_DECL(kCFNumberFormatterZeroSymbol,
  "kCFNumberFormatterZeroSymbol");
CONST_STRING_DECL(kCFNumberFormatterNaNSymbol,
  "kCFNumberFormatterNaNSymbol");
CONST_STRING_DECL(kCFNumberFormatterInfinitySymbol,
  "kCFNumberFormatterInfinitySymbol");
CONST_STRING_DECL(kCFNumberFormatterMinusSign,
  "kCFNumberFormatterMinusSign");
CONST_STRING_DECL(kCFNumberFormatterPlusSign,
  "kCFNumberFormatterPlusSign");
CONST_STRING_DECL(kCFNumberFormatterCurrencySymbol,
  "kCFNumberFormatterCurrencySymbol");
CONST_STRING_DECL(kCFNumberFormatterExponentSymbol,
  "kCFNumberFormatterExponentSymbol");
CONST_STRING_DECL(kCFNumberFormatterMinIntegerDigits,
  "kCFNumberFormatterMinIntegerDigits");
CONST_STRING_DECL(kCFNumberFormatterMaxIntegerDigits,
  "kCFNumberFormatterMaxIntegerDigits");
CONST_STRING_DECL(kCFNumberFormatterMinFractionDigits,
  "kCFNumberFormatterMinFractionDigits");
CONST_STRING_DECL(kCFNumberFormatterMaxFractionDigits,
  "kCFNumberFormatterMaxFractionDigits");
CONST_STRING_DECL(kCFNumberFormatterGroupingSize,
  "kCFNumberFormatterGroupingSize");
CONST_STRING_DECL(kCFNumberFormatterSecondaryGroupingSize,
  "kCFNumberFormatterSecondaryGroupingSize");
CONST_STRING_DECL(kCFNumberFormatterRoundingMode,
  "kCFNumberFormatterRoundingMode");
CONST_STRING_DECL(kCFNumberFormatterRoundingIncrement,
  "kCFNumberFormatterRoundingIncrement");
CONST_STRING_DECL(kCFNumberFormatterFormatWidth,
  "kCFNumberFormatterFormatWidth");
CONST_STRING_DECL(kCFNumberFormatterPaddingPosition,
  "kCFNumberFormatterPaddingPosition");
CONST_STRING_DECL(kCFNumberFormatterPaddingCharacter,
  "kCFNumberFormatterPaddingCharacter");
CONST_STRING_DECL(kCFNumberFormatterDefaultFormat,
  "kCFNumberFormatterDefaultFormat");
CONST_STRING_DECL(kCFNumberFormatterMultiplier,
  "kCFNumberFormatterMultiplier");
CONST_STRING_DECL(kCFNumberFormatterPositivePrefix,
  "kCFNumberFormatterPositivePrefix");
CONST_STRING_DECL(kCFNumberFormatterPositiveSuffix,
  "kCFNumberFormatterPositiveSuffix");
CONST_STRING_DECL(kCFNumberFormatterNegativePrefix,
  "kCFNumberFormatterNegativePrefix");
CONST_STRING_DECL(kCFNumberFormatterNegativeSuffix,
  "kCFNumberFormatterNegativeSuffix");
CONST_STRING_DECL(kCFNumberFormatterPerMillSymbol,
  "kCFNumberFormatterPerMillSymbol");
CONST_STRING_DECL(kCFNumberFormatterInternationalCurrencySymbol,
  "kCFNumberFormatterInternationalCurrencySymbol"); 
CONST_STRING_DECL(kCFNumberFormatterCurrencyGroupingSeparator,
  "kCFNumberFormatterCurrencyGroupingSeparator");
CONST_STRING_DECL(kCFNumberFormatterIsLenient,
  "kCFNumberFormatterIsLenient");
CONST_STRING_DECL(kCFNumberFormatterUseSignificantDigits,
  "kCFNumberFormatterUseSignificantDigits");
CONST_STRING_DECL(kCFNumberFormatterMinSignificantDigits,
  "kCFNumberFormatterMinSignificantDigits");
CONST_STRING_DECL(kCFNumberFormatterMaxSignificantDigits,
  "kCFNumberFormatterMaxSignificantDigits");

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



static void
CFNumberFormatterSetAttribute (CFNumberFormatterRef fmt, int attrib,
  CFTypeRef num)
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

static void
CFNumberFormatterSetTextAttribute (CFNumberFormatterRef fmt, int attrib,
  CFTypeRef str)
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

static void
CFNumberFormatterSetSymbol (CFNumberFormatterRef fmt, int symbol,
  CFTypeRef str)
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

static CFTypeRef
CFNumberFormatterCopyAttribute (CFNumberFormatterRef fmt, int attrib)
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

static CFTypeRef
CFNumberFormatterCopyTextAttribute (CFNumberFormatterRef fmt, int attrib)
{
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  len = unum_getTextAttribute (fmt->_fmt, attrib, ubuffer, BUFFER_SIZE, &err);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  return CFStringCreateWithCharacters (NULL, ubuffer, len);
}

static CFTypeRef
CFNumberFormatterCopySymbol (CFNumberFormatterRef fmt, int symbol)
{
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  len = unum_getSymbol (fmt->_fmt, symbol, ubuffer, BUFFER_SIZE, &err);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  return CFStringCreateWithCharacters (NULL, ubuffer, len);
}

static CFTypeRef
CFNumberFormatterCopyDefaultFormat (CFNumberFormatterRef fmt, int attrib)
{
  return CFRetain (fmt->_defaultFormat);
}

static struct _kCFNumberFormatterProperties
{
  const CFStringRef *prop;
  int icuProp;
  void (*set)(CFNumberFormatterRef fmt, int attrib, CFTypeRef value);
  CFTypeRef (*copy)(CFNumberFormatterRef fmt, int attrib);
} _kCFNumberFormatterProperties[] =
{
  { &kCFNumberFormatterDefaultFormat, 0, NULL,
    CFNumberFormatterCopyDefaultFormat },
  { &kCFNumberFormatterCurrencyCode, UNUM_CURRENCY_CODE,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterDecimalSeparator, UNUM_DECIMAL_SEPARATOR_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterCurrencyDecimalSeparator,
    UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterAlwaysShowDecimalSeparator, UNUM_DECIMAL_ALWAYS_SHOWN,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterGroupingSeparator, UNUM_GROUPING_SEPARATOR_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterUseGroupingSeparator, UNUM_GROUPING_USED,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterPercentSymbol, UNUM_PERCENT_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterZeroSymbol, UNUM_ZERO_DIGIT_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterNaNSymbol, UNUM_NAN_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterInfinitySymbol, UNUM_INFINITY_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterMinusSign, UNUM_MINUS_SIGN_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterPlusSign, UNUM_PLUS_SIGN_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterCurrencySymbol, UNUM_CURRENCY_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterExponentSymbol, UNUM_EXPONENTIAL_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterMinIntegerDigits, UNUM_MIN_INTEGER_DIGITS,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterMaxIntegerDigits, UNUM_MAX_INTEGER_DIGITS,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterMinFractionDigits, UNUM_MIN_FRACTION_DIGITS,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterMaxFractionDigits, UNUM_MAX_FRACTION_DIGITS,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterGroupingSize, UNUM_GROUPING_SIZE,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterSecondaryGroupingSize, UNUM_SECONDARY_GROUPING_SIZE,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterRoundingMode, UNUM_ROUNDING_MODE,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterRoundingIncrement, UNUM_ROUNDING_INCREMENT,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterFormatWidth, UNUM_FORMAT_WIDTH,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterPaddingPosition, UNUM_PADDING_POSITION,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterPaddingCharacter, UNUM_PADDING_CHARACTER,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterDefaultFormat, UNUM_MULTIPLIER,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterMultiplier, UNUM_MULTIPLIER,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterPositivePrefix, UNUM_POSITIVE_PREFIX,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterPositiveSuffix, UNUM_POSITIVE_SUFFIX,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterNegativePrefix, UNUM_NEGATIVE_PREFIX,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterNegativeSuffix, UNUM_NEGATIVE_SUFFIX,
    CFNumberFormatterSetTextAttribute, CFNumberFormatterCopyTextAttribute },
  { &kCFNumberFormatterPerMillSymbol, UNUM_PERMILL_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterInternationalCurrencySymbol, UNUM_INTL_CURRENCY_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterCurrencyGroupingSeparator, UNUM_MONETARY_SEPARATOR_SYMBOL,
    CFNumberFormatterSetSymbol, CFNumberFormatterCopySymbol },
  { &kCFNumberFormatterIsLenient, UNUM_LENIENT_PARSE,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterUseSignificantDigits, UNUM_SIGNIFICANT_DIGITS_USED,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterMinSignificantDigits, UNUM_MIN_SIGNIFICANT_DIGITS,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute },
  { &kCFNumberFormatterMaxSignificantDigits, UNUM_MAX_SIGNIFICANT_DIGITS,
    CFNumberFormatterSetAttribute, CFNumberFormatterCopyAttribute }
};
static const CFIndex
_kCFNumberFormatterPropertiesSize = sizeof(_kCFNumberFormatterProperties) /
  sizeof(struct _kCFNumberFormatterProperties);



CFNumberFormatterRef
CFNumberFormatterCreate (CFAllocatorRef allocator, CFLocaleRef loc,
  CFNumberFormatterStyle style)
{
  struct __CFNumberFormatter *new;
  const char *cLocale;
  char buffer[ULOC_FULLNAME_CAPACITY];
  int32_t len;
  UChar ubuffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  if (loc == NULL)
    loc = CFLocaleGetSystem ();
  cLocale = CFLocaleGetCStringIdentifier (loc, buffer, ULOC_FULLNAME_CAPACITY);
  
  new = (struct __CFNumberFormatter *)_CFRuntimeCreateInstance (allocator,
    CFNumberFormatterGetTypeID(),
    sizeof(struct __CFNumberFormatter) - sizeof(CFRuntimeBase),
    NULL);
  if (new == NULL)
    return NULL;
  
  /* Good news, UNumberFormatStyle and NS/CFNumberFormatterStyle match. */
  new->_fmt = unum_open ((UNumberFormatStyle )style, NULL, 0, cLocale,
                         NULL, &err);
  if (U_FAILURE(err))
    {
      CFRelease (new);
      return NULL;
    }
  new->_locale = CFRetain(loc);
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
  new->_format = CFRetain (new->_defaultFormat);
  
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
  CFIndex idx;
  
  for (idx = 0 ; idx < _kCFNumberFormatterPropertiesSize ; ++idx)
    {
      if (key == *(_kCFNumberFormatterProperties[idx].prop))
        {
          (_kCFNumberFormatterProperties[idx].set)(fmt,
            _kCFNumberFormatterProperties[idx].icuProp, value);
          return;
        }
    }
  
  for (idx = 0 ; idx < _kCFNumberFormatterPropertiesSize ; ++idx)
    {
      if (CFEqual(key, *(_kCFNumberFormatterProperties[idx].prop)))
        {
          (_kCFNumberFormatterProperties[idx].set)(fmt,
            _kCFNumberFormatterProperties[idx].icuProp, value);
          return;
        }
    }
}

CFNumberRef
CFNumberFormatterCreateNumberFromString (CFAllocatorRef allocator,
  CFNumberFormatterRef fmt, CFStringRef str, CFRange *rangep,
  CFOptionFlags options)
{
  CFNumberRef result;
  CFNumberType type;
  UInt8 value[sizeof(double)]; /* FIXME: what's the largest value we have? */
  
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
  UInt8 value[sizeof(double)]; /* FIXME */
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
  UChar ubuffer[BUFFER_SIZE];
  int32_t len;
  int64_t inum = 0;
  double  dnum = 0.0;
  UErrorCode err = U_ZERO_ERROR;
  
  switch (numberType)
    {
      case kCFNumberSInt8Type:
        inum = (int64_t)*(SInt8*)valuePtr;
        break;
      case kCFNumberSInt16Type:
        inum = (int64_t)*(SInt16*)valuePtr;
        break;
      case kCFNumberSInt32Type:
        inum = (int64_t)*(SInt32*)valuePtr;
        break;
      case kCFNumberSInt64Type:
        inum = (int64_t)*(SInt64*)valuePtr;
        break;
      case kCFNumberCharType:
        inum = (int64_t)*(char*)valuePtr;
        break;
      case kCFNumberShortType:
        inum = (int64_t)*(short*)valuePtr;
        break;
      case kCFNumberIntType:
        inum = (int64_t)*(int*)valuePtr;
        break;
      case kCFNumberLongType:
        inum = (int64_t)*(long*)valuePtr;
        break;
      case kCFNumberLongLongType:
        inum = (int64_t)*(long long*)valuePtr;
        break;
      case kCFNumberCFIndexType:
        inum = (int64_t)*(CFIndex*)valuePtr;
        break;
      case kCFNumberNSIntegerType: /* FIXME: This isn't defined in CF, so guess */
        inum = (int64_t)*(CFIndex*)valuePtr;
        break;
      case kCFNumberFloat32Type:
        dnum = (double)*(Float32*)valuePtr;
        break;
      case kCFNumberFloat64Type:
        dnum = (double)*(Float64*)valuePtr;
        break;
      case kCFNumberFloatType:
        dnum = (double)*(float*)valuePtr;
        break;
      case kCFNumberDoubleType:
        dnum = *(double*)valuePtr;
        break;
      case kCFNumberCGFloatType: /* FIXME: Guess here, too */
        dnum = *(double*)valuePtr;
        break;
    }
  
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
        len = unum_formatInt64 (fmt->_fmt, inum, ubuffer, BUFFER_SIZE,
          NULL, &err);
        break;
      default: /* must be a float type */
        len = unum_formatDouble (fmt->_fmt, dnum, ubuffer, BUFFER_SIZE,
          NULL, &err);
    }
  
  if (U_FAILURE(err))
    return NULL;
  
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  return CFStringCreateWithCharacters (allocator, ubuffer, len);
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
  int64_t iresult = 0; /* Keep compiler happy */
  
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
      default: /* must be a float type */
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
      case kCFNumberNSIntegerType: /* FIXME: This isn't defined in CF, so guess */
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
      case kCFNumberCGFloatType: /* FIXME: Guess here, too */
        *(double*)valuePtr = (double)dresult;
        break;
    }
  
  if (rangep && parseRange.length != parsePos)
    {
      rangep->length = parsePos;
      return false;
    }
  
  return true;
}

CFTypeRef
CFNumberFormatterCopyProperty (CFNumberFormatterRef fmt,
  CFStringRef key)
{
  CFIndex idx;
  
  for (idx = 0 ; idx < _kCFNumberFormatterPropertiesSize ; ++idx)
    {
      if (key == *(_kCFNumberFormatterProperties[idx].prop))
        return (_kCFNumberFormatterProperties[idx].copy)(fmt,
          _kCFNumberFormatterProperties[idx].icuProp);
    }
  
  for (idx = 0 ; idx < _kCFNumberFormatterPropertiesSize ; ++idx)
    {
      if (CFEqual(key, *(_kCFNumberFormatterProperties[idx].prop)))
        return (_kCFNumberFormatterProperties[idx].copy)(fmt,
          _kCFNumberFormatterProperties[idx].icuProp);
    }
  
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

