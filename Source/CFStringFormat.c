/* CFStringFormat.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: June, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redisibute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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
#include "CoreFoundation/CFByteOrder.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "GSPrivate.h"

#include <math.h>
#include <stddef.h>
#include <string.h>
#include <unicode/unum.h>

#if defined(_MSC_VER)
typedef intptr_t intmax_t;
typedef uintptr_t uintmax_t;
#endif

#define CF_FMT_FLAG_SIGN       (1<<0)
#define CF_FMT_FLAG_LEFT_ALIGN (1<<1)
#define CF_FMT_FLAG_ALT        (1<<2)
#define CF_FMT_FLAG_PAD        (1<<3)
#define CF_FMT_FLAG_GROUP_SEP  (1<<4)

#define CF_FMT_IS_FLAG(c) (c) == '+' \
  || (c) == '-' \
  || (c) == '#' \
  || (c) == '0'
#define CF_FMT_IS_LENGTH(c) (c) == 'L' \
  || (c) == 'h' \
  || (c) == 'j' \
  || (c) == 'l' \
  || (c) == 'q' \
  || (c) == 't' \
  || (c) == 'z'
#define CHAR_IS_CAPS(c) (c) >= 'A' && (c) <= 'Z'

typedef enum
{
  CFCharLength = 1,
  CFShortLength,
  CFLongLength,
  CFLongLongLength,
  CFLongDoubleLength,
  CFSizeTLength,
  CFIntMaxTLength,
  CFPtrDiffTLength
} CFArgLength;

typedef enum
{
  CFUnsignedType = 0,
  CFIntegerType,
  CFFloatType,
  CFDoubleType,
  CFScientificType,
  CFHexType,
  CFOctalType,
  CFStringType,
  CFUStringType,
  CFCharType,
  CFUCharType,
  CFPointerType,
  CFObjectType,
  CFUnknownType = 0xFF
} CFArgType;

typedef union
{
  SInt64  intValue; /* Holds every possible type of integer */
  double  doubleValue;
  void   *ptrValue; /* Includes object pointers */
} CFFormatArgument;

/* For information only:
     %[parameter][flags][width][.precision][length]type */
typedef struct
{
  CFIndex       argPos;
  UniChar       type;
  Boolean       useCaps;
  CFOptionFlags flags;
  CFIndex       width;
  CFIndex       precision;
  CFArgLength   length;
} CFFormatSpec;

typedef CFStringRef (*CFFormatFormatter)(CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions);

typedef struct
{
  CFArgType type;
  CFFormatFormatter formatter;
} CFFormatFormatterInfo;

typedef struct
{
  CFFormatSpec spec;
  CFFormatFormatter fmt;
} CFFormatSpecInfo;

#define CFUnknownFormat    { CFUnknownType,    NULL }
#define CFUnsignedFormat   { CFUnsignedType,   CFFormatInteger }
#define CFIntegerFormat    { CFIntegerType,    CFFormatInteger }
#define CFFloatFormat      { CFFloatType,      CFFormatFloat }
#define CFDoubleFormat     { CFDoubleType,     CFFormatDouble }
#define CFScientificFormat { CFScientificType, CFFormatScientific }
#define CFHexFormat        { CFHexType,        CFFormatHex }
#define CFPointerFormat    { CFPointerType,    CFFormatPointer }
#define CFOctalFormat      { CFOctalType,      CFFormatOctal }
#define CFStringFormat     { CFStringType,     CFFormatString }
#define CFUStringFormat    { CFUStringType,    CFFormatUString }
#define CFCharFormat       { CFCharType,       CFFormatChar }
#define CFUCharFormat      { CFUCharType,      CFFormatUChar }
#define CFObjectFormat     { CFObjectType,     CFFormatObject }

static void
CFFormatUNumberFormatApplySpec (UNumberFormat *fmt, CFFormatSpec *spec)
{
  UErrorCode err = U_ZERO_ERROR;
  
  if (spec->flags & CF_FMT_FLAG_SIGN)
    {
      UChar symbol[] = { 0x002B, 0x0000 };
      unum_setSymbol (fmt, UNUM_PLUS_SIGN_SYMBOL, symbol, 1, &err);
    }
  if (spec->flags & CF_FMT_FLAG_LEFT_ALIGN)
    {
      unum_setAttribute (fmt, UNUM_PADDING_POSITION, UNUM_PAD_AFTER_SUFFIX);
    }
  if (spec->flags & CF_FMT_FLAG_PAD)
    {
      UChar attrib[] = { 0x0030, 0x0000 };
      unum_setTextAttribute (fmt, UNUM_PADDING_CHARACTER, attrib, 1, &err);
    }
  else
    {
      /* ICU, by default, pads with '*' */
      UChar attrib[] = { 0x0020, 0x0000 };
      unum_setTextAttribute (fmt, UNUM_PADDING_CHARACTER, attrib, 1, &err);
    }
  if (spec->flags & CF_FMT_FLAG_GROUP_SEP)
    {
      unum_setAttribute (fmt, UNUM_GROUPING_USED, 1);
    }
  
  if (spec->width >= 0)
    {
      unum_setAttribute (fmt, UNUM_FORMAT_WIDTH, spec->width);
    }
}

#define BUFFER_SIZE 256

static CFStringRef
CFFormatInteger (CFFormatSpec *spec,
                 CFStringRef (*copyDescFunc)(void *, const void *loc),
                 CFFormatArgument *arg,
                 CFDictionaryRef formatOptions)
{
  SInt64 i = 0;
  CFIndex numChars;
  UniChar buffer[BUFFER_SIZE];
  CFStringRef ret;
  UNumberFormat *fmt;
  UErrorCode err = U_ZERO_ERROR;
  
  /* Using unum directly because CFNumberFormatter proved to be too complex
     because of the need to generate CFNumberRefs in order to set any
     property.  Those CFNumberRefs then get turned into int32_t before
     being set, not very efficient either. */
  fmt = unum_open (UNUM_DECIMAL, NULL, 0, NULL, NULL, &err);
  if (U_FAILURE(err))
    return NULL;
  
  CFFormatUNumberFormatApplySpec (fmt, spec);
  
  if (spec->precision >= 0)
    unum_setAttribute (fmt, UNUM_MIN_INTEGER_DIGITS, spec->precision);
  
  if (spec->type == CFIntegerType)
    {
      switch (spec->length)
        {
          case CFCharLength:
            i = (SInt64)((char)arg->intValue);
            break;
          case CFShortLength:
            i = (SInt64)((short)arg->intValue);
            break;
          case CFLongLength:
            i = (SInt64)((long)arg->intValue);
            break;
          case CFLongLongLength:
            i = (SInt64)((long long)arg->intValue);
            break;
          case CFSizeTLength:
            i = (SInt64)((size_t)arg->intValue);
            break;
          case CFIntMaxTLength:
            i = (SInt64)((intmax_t)arg->intValue);
            break;
          case CFPtrDiffTLength:
            i = (SInt64)((ptrdiff_t)arg->ptrValue);
            break;
          default:
            i = (SInt64)((int)arg->intValue);
        }
    }
  else if (spec->type == CFUnsignedType)
    {
      switch (spec->length)
        {
          case CFCharLength:
            i = (SInt64)((unsigned char)arg->intValue);
            break;
          case CFShortLength:
            i = (SInt64)((unsigned short)arg->intValue);
            break;
          case CFLongLength:
            i = (SInt64)((unsigned long)arg->intValue);
            break;
          case CFLongLongLength:
            i = (SInt64)((signed long long)arg->intValue);
            break;
          case CFSizeTLength:
            i = (SInt64)((size_t)arg->intValue); /* Already unsigned */
            break;
          case CFIntMaxTLength:
            i = (SInt64)((uintmax_t)arg->intValue);
            break;
          case CFPtrDiffTLength:
            i = (SInt64)((ptrdiff_t)arg->intValue); /* Unsigned version? */
            break;
          default:
            i = (SInt64)((unsigned int)arg->intValue);
        }
    }
  numChars = unum_formatInt64 (fmt, i, (UChar*)buffer, BUFFER_SIZE,
    NULL, &err);
  ret = CFStringCreateWithCharacters (NULL, buffer, numChars);
  
  unum_close (fmt);
  return ret;
}

static CFStringRef
CFFormatFloat (CFFormatSpec *spec,
               CFStringRef (*copyDescFunc)(void *, const void *loc),
               CFFormatArgument *arg,
               CFDictionaryRef formatOptions)
{
  double d;
  CFIndex numChars;
  UniChar buffer[BUFFER_SIZE];
  CFStringRef ret;
  UNumberFormat *fmt;
  UErrorCode err = U_ZERO_ERROR;
  
  fmt = unum_open (UNUM_DECIMAL, NULL, 0, "en_US_POSIX", NULL, &err);
  if (U_FAILURE(err))
    return NULL;
  
  CFFormatUNumberFormatApplySpec (fmt, spec);
  
  unum_setAttribute (fmt, UNUM_MIN_FRACTION_DIGITS, 6);
  
  if (spec->precision >= 0)
    unum_setAttribute (fmt, UNUM_MAX_FRACTION_DIGITS, spec->precision);
  
  if (spec->length == CFLongDoubleLength)
    d = (double)((long double)arg->doubleValue);
  else
    d = arg->doubleValue;
  numChars = unum_formatDouble (fmt, d, (UChar*)buffer, BUFFER_SIZE,
    NULL, &err);
  if (numChars > BUFFER_SIZE)
    numChars = BUFFER_SIZE;
  ret = CFStringCreateWithCharacters (NULL, buffer, numChars);
  
  unum_close (fmt);
  return ret;
}

static const UChar expPattern[] =
  { '#', '.', '0', '0', '0', '0', '0', '0', 'E', '+', '0' , '0' };
static const int32_t expPatternSize = sizeof(expPattern) / sizeof(UChar);

static CFStringRef
CFFormatScientific (CFFormatSpec *spec,
                    CFStringRef (*copyDescFunc)(void *, const void *loc),
                    CFFormatArgument *arg,
                    CFDictionaryRef formatOptions)
{
  double d;
  CFIndex numChars;
  UniChar buffer[BUFFER_SIZE];
  CFStringRef ret;
  UNumberFormat *fmt;
  UErrorCode err = U_ZERO_ERROR;
  
  fmt = unum_open (UNUM_DECIMAL, NULL, 0, "en_US_POSIX", NULL, &err);
  if (U_FAILURE(err))
    return NULL;
  
  unum_applyPattern (fmt, false, expPattern, expPatternSize, NULL, &err);
  CFFormatUNumberFormatApplySpec (fmt, spec);
  
  if (spec->precision >= 0)
    unum_setAttribute (fmt, UNUM_MAX_FRACTION_DIGITS, spec->precision);
  
  if (!spec->useCaps)
    {
      UChar symbol[] = { 0x0065, 0x0000 };
      unum_setSymbol (fmt, UNUM_EXPONENTIAL_SYMBOL, symbol, 1, &err);
    }
  
  if (spec->length == CFLongDoubleLength)
    /* FIXME: not really supported */
    d = (double)((long double)arg->doubleValue);
  else
    d = arg->doubleValue;
  numChars = unum_formatDouble (fmt, d, (UChar*)buffer, BUFFER_SIZE,
    NULL, &err);
  ret = CFStringCreateWithCharacters (NULL, buffer, numChars);
  
  unum_close (fmt);
  return ret;
}

static CFStringRef
CFFormatDouble (CFFormatSpec *spec,
                CFStringRef (*copyDescFunc)(void *, const void *loc),
                CFFormatArgument *arg,
                CFDictionaryRef formatOptions)
{
  double d = arg->doubleValue;
  CFIndex precision = spec->precision;
  CFIndex numChars;
  UniChar buffer[BUFFER_SIZE];
  CFStringRef ret;
  UNumberFormat *fmt;
  UErrorCode err = U_ZERO_ERROR;
  
  if (d < 0.0001 /* exponent < -4 */
      || (precision < 1 && 1000000.0 <= d) /* exponent > precision */
      || (precision != -1 && d > pow(10.0, (double)precision))) /* Same */
    {
      return CFFormatScientific (spec, copyDescFunc, arg, formatOptions);
    }
  
  /* Very similar to CFFormatInteger() */
  fmt = unum_open (UNUM_DECIMAL, NULL, 0, "en_US_POSIX", NULL, &err);
  if (U_FAILURE(err))
    return NULL;
  
  CFFormatUNumberFormatApplySpec (fmt, spec);
  
  if (spec->precision >= 0)
    unum_setAttribute (fmt, UNUM_MAX_FRACTION_DIGITS, precision);
  
  numChars = unum_formatDouble (fmt, d, (UChar*)buffer, BUFFER_SIZE,
    NULL, &err);
  ret = CFStringCreateWithCharacters (NULL, buffer, numChars);
  
  unum_close (fmt);
  return ret;
}

static CFStringRef
CFFormatUInt64ToString (CFFormatSpec *spec, CFFormatArgument *arg, UInt8 base)
{
  UniChar buffer[BUFFER_SIZE];
  UniChar *right;
  UniChar *left;
  UniChar tmp;
  UInt64 value;
  CFIndex length = 0;
  
  switch (spec->length)
    {
      case CFCharLength:
        value = (unsigned char)arg->intValue;
        break;
      case CFShortLength:
        value = (unsigned short)arg->intValue;
        break;
      case CFLongLength:
        value = (unsigned long)arg->intValue;
        break;
      case CFLongLongLength:
        value = (unsigned long long)arg->intValue;
        break;
      case CFSizeTLength:
        value = (size_t)arg->intValue;
        break;
      case CFIntMaxTLength:
        value = (uintmax_t)arg->intValue;
        break;
      case CFPtrDiffTLength:
        value = (ptrdiff_t)arg->ptrValue;
        break;
      default:
        value = (UInt64)arg->intValue;
    }
  
#define TO_CAP_DIGIT(n) (n < 10 ? n + '0' : n - 10 + 'A')
#define TO_LOWER_DIGIT(n) (n < 10 ? n + '0' : n - 10 + 'a')
  do
    {
      int num = value % base;
      value /= base;
      buffer[length++] =
        spec->useCaps ? TO_CAP_DIGIT(num) : TO_LOWER_DIGIT(num);
    } while (value != 0);
  
  while (length < spec->width) /* pad */
    buffer[length++] = '0';
  
  if (spec->flags & CF_FMT_FLAG_ALT)
    {
      if (base == 16)
        buffer[length++] = spec->useCaps ? 'X' : 'x';
      buffer[length++] = '0';
    }
  
  left = buffer;
  right = buffer + length;
  while (left < --right) /* reverse */
    {
      tmp = *left;
      *left++ = *right;
      *right = tmp;
    }
  
  return CFStringCreateWithCharacters (NULL, buffer, length);
}

static CFStringRef
CFFormatPointer (CFFormatSpec *spec,
                 CFStringRef (*copyDescFunc)(void *, const void *loc),
                 CFFormatArgument *arg,
                 CFDictionaryRef formatOptions)
{
  spec->length = CFPtrDiffTLength;
  spec->flags |= CF_FMT_FLAG_ALT;
  return CFFormatUInt64ToString (spec, arg, 16);
}

static CFStringRef
CFFormatHex (CFFormatSpec *spec,
             CFStringRef (*copyDescFunc)(void *, const void *loc),
             CFFormatArgument *arg,
             CFDictionaryRef formatOptions)
{
  return CFFormatUInt64ToString (spec, arg, 16);
}

static CFStringRef
CFFormatOctal (CFFormatSpec *spec,
               CFStringRef (*copyDescFunc)(void *, const void *loc),
               CFFormatArgument *arg,
               CFDictionaryRef formatOptions)
{
  return CFFormatUInt64ToString (spec, arg, 8);
}

/* Keep in mind that, according to Apple docs, both %C and %S take UniChars
   as arguments, not %lc and %ls, respectively, as the specification
   suggests. */
static CFStringRef
CFFormatUString (CFFormatSpec *spec,
                 CFStringRef (*copyDescFunc)(void *, const void *loc),
                 CFFormatArgument *arg,
                 CFDictionaryRef formatOptions)
{
  CFIndex length;
  CFIndex numChars;
  UniChar *count;
  const UniChar *chars;
  CFAllocatorRef alloc;
  CFMutableStringRef str;
  CFStringRef ret;
  
  alloc = CFAllocatorGetDefault();
  chars = (const UniChar *)arg->ptrValue;
  count = (UniChar *)arg->ptrValue;
  numChars = spec->precision;
  if (numChars < 0)
    {
      while (*count != 0x0000)
        ++count;
      numChars = count - chars;
    }
  length = spec->width;
  if (length < 0)
    {
      length = numChars;
    }
  
  str = CFStringCreateMutable (alloc, length);
  CFStringAppendCharacters (str, chars, numChars);
  if (length > numChars)
    CFStringPad (str, CFSTR(" "), length - numChars, 0);
  
  ret = CFStringCreateCopy (alloc, str);
  CFRelease (str);
  
  return ret;
}

static CFStringRef
CFFormatUChar (CFFormatSpec *spec,
               CFStringRef (*copyDescFunc)(void *, const void *loc),
               CFFormatArgument *arg,
               CFDictionaryRef formatOptions)
{
  return CFStringCreateWithCharacters (CFAllocatorGetDefault(),
    (const UniChar *)&arg->intValue, 1);
}

static CFStringRef
CFFormatString (CFFormatSpec *spec,
                CFStringRef (*copyDescFunc)(void *, const void *loc),
                CFFormatArgument *arg,
                CFDictionaryRef formatOptions)
{
  CFIndex length;
  CFIndex numChars;
  const char *chars;
  CFAllocatorRef alloc;
  CFMutableStringRef str;
  CFStringRef ret;
  
  if (spec->length == CFLongLength)
    return CFFormatUString (spec, copyDescFunc, arg, formatOptions);
  
  alloc = CFAllocatorGetDefault();
  chars = (const char *)arg->ptrValue;
  numChars = spec->precision;
  if (numChars < 0)
    {
      numChars = strlen (chars);
    }
  length = spec->width;
  if (length < 0)
    {
      length = numChars;
    }
  
  str = CFStringCreateMutable (alloc, length);
  CFStringAppendCString (str, chars, CFStringGetSystemEncoding());
  if (length > numChars)
    CFStringPad (str, CFSTR(" "), length - numChars, 0);
  
  ret = CFStringCreateCopy (alloc, str);
  CFRelease (str);
  
  return ret;
}

static CFStringRef
CFFormatChar (CFFormatSpec *spec,
              CFStringRef (*copyDescFunc)(void *, const void *loc),
              CFFormatArgument *arg,
              CFDictionaryRef formatOptions)
{
  UInt8 intValue;
  
  if (spec->length == CFLongLength)
    return CFFormatUChar (spec, copyDescFunc, arg, formatOptions);
  
  intValue = (UInt8)arg->intValue;
  return CFStringCreateWithBytes (CFAllocatorGetDefault(),
    (const UInt8*)&intValue, 1, CFStringGetSystemEncoding(), false);
}

static CFStringRef
CFFormatObject(CFFormatSpec *spec,
               CFStringRef (*copyDescFunc)(void *, const void *loc),
               CFFormatArgument *arg,
               CFDictionaryRef formatOptions)
{
  const CFRuntimeClass *cls;
  /* Disregard all formatting */
  if (copyDescFunc)
    return copyDescFunc (arg->ptrValue, formatOptions);
  
  cls = _CFRuntimeGetClassWithTypeID (CFGetTypeID((CFTypeRef)arg->ptrValue));
  if (cls->copyFormattingDesc)
    return cls->copyFormattingDesc((CFTypeRef)arg->ptrValue, formatOptions);

  return CFCopyDescription ((CFTypeRef)arg->ptrValue);
}

static CFStringRef
CFFormatPercent (CFFormatSpec *spec,
                 CFStringRef (*copyDescFunc)(void *, const void *loc),
                 CFFormatArgument *arg,
                 CFDictionaryRef formatOptions)
{
  return CFSTR("%");
}

/* FIXME: %a and %A are not implemented because I'm not sure how it works. */
static const CFFormatFormatterInfo _kCFStringFormatter[] =
{
  /* 0x40 */
  CFObjectFormat,    CFUnknownFormat,    CFUnknownFormat,    CFUCharFormat,
  CFIntegerFormat,   CFScientificFormat, CFFloatFormat,      CFDoubleFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFOctalFormat,
  /* 0x50 */
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUStringFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFHexFormat,       CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  /* 0x60 */
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFCharFormat,
  CFIntegerFormat,   CFScientificFormat, CFFloatFormat,      CFDoubleFormat,
  CFUnknownFormat,   CFIntegerFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFOctalFormat,
  /* 0x70 */
  CFPointerFormat,   CFUnknownFormat,    CFUnknownFormat,    CFStringFormat,
  CFUnknownFormat,   CFUnsignedFormat,   CFUnknownFormat,    CFUnknownFormat,
  CFHexFormat,       CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat
};

#define CHAR_MIN_TYPE 0x0040
#define CHAR_MAX_TYPE \
  (sizeof(_kCFStringFormatter) / sizeof(CFFormatFormatterInfo))
#define CHAR_CAPS_TYPE 0x005B - CHAR_MIN_TYPE

CFFormatArgument *
CFStringFormatCreateArgumentList (UniChar *start, const UniChar *end,
  va_list args)
{
  CFFormatArgument *argList;
  CFArgType *typeList;
  Boolean *is64Bits;
  UniChar *current;
  UniChar type;
  CFIndex typeIdx;
  CFIndex pos;
  CFIndex callout;
  CFIndex count = 0;
  
  /* Count total number of arguments
     I'm just going to count how many '%' there are and multiply that by
     3 because that's the largest amount of arguments each spec (ie '%*.*s') */
  current = start;
  for(;;)
    {
      while (current < end && *current != '%')
        ++current;
      if (current == end)
        break;
      ++current; /* Skip % */
      ++count;
    }
  
  count *= 3; /* Multiply by 3 */
  typeList = CFAllocatorAllocate (NULL, sizeof(CFArgType) * count, 0);
  is64Bits = CFAllocatorAllocate (NULL, sizeof(Boolean) * count, 0);
  if (typeList == NULL || is64Bits == NULL)
    {
      if (typeList != NULL)
        CFAllocatorDeallocate (NULL, typeList);
      if (is64Bits != NULL)
        CFAllocatorDeallocate (NULL, is64Bits);
      
      return NULL;
    }
  
  /* Get argument values */
  current = start;
  pos = 0;
  count = 0;
  for (;;)
    {
      while (current < end && *current != '%')
        ++current;
      if (current == end)
        break;
      ++current; /* Skip % */
      
      if (*current == '%') /* Skip %% */
        {
          ++current;
          continue;
        }
      
      callout = 0;
      for (;;)
        {
          if (CHAR_IS_DIGIT(*current))
            {
              int num = *(current++) - '0';
              while (CHAR_IS_DIGIT(*current))
                {
                  num *= 10;
                  num += *(current++) - '0';
                }
              if (*current == '$')
                {
                  callout = num - 1;
                  ++current;
                  if (callout > pos)
                    count = num;
                }
            }
          else if (CF_FMT_IS_FLAG(*current) || *current == '.')
            {
              ++current;
            }
          else if (*current == '*')
            {
              typeList[pos] = CFIntegerType;
              pos += 1;
              ++current;
            }
          else
            {
              break;
            }
        }
      is64Bits[pos] = false;
      if (*(current) == 'h')
        {
          ++current;
          if (*current == 'h')
            ++current;
        }
      else if (*(current) == 'l')
        {
          ++current;
#if __LP64__
          is64Bits[pos] = true;
#endif
          if (*(current) == 'l')
            {
#if !__LP64__
              is64Bits[pos] = true;
#endif
              ++current;
            }
        }
      type = *current;
      
      typeIdx = type - CHAR_MIN_TYPE;
      if (typeIdx < CHAR_MAX_TYPE)
        if (callout)
          typeList[callout] = _kCFStringFormatter[typeIdx].type;
        else
          typeList[pos++] = _kCFStringFormatter[typeIdx].type;
      else
        typeList[pos++] = CFUnknownType;
    }
  
  if (pos < count)
    pos = count;
  else
    count = pos;
  argList = CFAllocatorAllocate (NULL, sizeof(CFFormatArgument) * count, 0);
  if (argList != NULL)
    {
      /* Collect the arguments */
      for (pos = 0 ; pos < count ; ++pos)
        {
          switch (typeList[pos])
            {
              case CFCharType:
              case CFUnsignedType:
              case CFHexType:
              case CFOctalType:
              case CFIntegerType:
                if (is64Bits[pos])
                  argList[pos].intValue = va_arg (args, long long);
                else
                  argList[pos].intValue = va_arg (args, int);
                break;
              case CFStringType:
              case CFPointerType:
              case CFObjectType:
                argList[pos].ptrValue = va_arg (args, void*);
                break;
              case CFFloatType:
              case CFDoubleType:
              case CFScientificType:
                argList[pos].doubleValue = va_arg (args, double);
                break;
              default:
                argList[pos].ptrValue = NULL;
            }
        }
    }
  
  CFAllocatorDeallocate (NULL, typeList);
  CFAllocatorDeallocate (NULL, is64Bits);
  return argList;
}

CFIndex
CFStringFormatParseSpec (UniChar *start, const UniChar *end,
  CFFormatSpecInfo *info, CFFormatArgument *argList, CFIndex *arg)
{
  int num;
  CFIndex typeIdx;
  UniChar *current;
  
  current = start;
  ++current; /* Skip % */
  
  /* Initialize the spec */
  memset (info, 0, sizeof(CFFormatSpecInfo));
  info->spec.argPos = -1;
  info->spec.width = -1;
  info->spec.precision = -1;
  
  /* Check parameter field */
  if (CHAR_IS_DIGIT(*current))
    {
      UniChar *revert = current;
      
      num = *(current++) - '0';
      while (CHAR_IS_DIGIT(*current))
        {
          num *= 10;
          num += *(current++) - '0';
        }
      if (*current == '$')
        {
          ++current;
          info->spec.argPos = num - 1;
        }
      else /* Must be a width argument */
        {
          current = revert;
        }
    }
  
  /* Check flags */
  while (CF_FMT_IS_FLAG(*current))
    {
      switch (*current)
        {
          case '+':
            info->spec.flags |= CF_FMT_FLAG_SIGN;
            break;
          case '-':
            info->spec.flags |= CF_FMT_FLAG_LEFT_ALIGN;
            break;
          case '#':
            info->spec.flags |= CF_FMT_FLAG_ALT;
            break;
          case '0':
            info->spec.flags |= CF_FMT_FLAG_PAD;
            break;
        }
      ++current;
    }
  
  /* Check width */
  if (CHAR_IS_DIGIT(*current))
    {
      num = *(current++) - '0';
      while (CHAR_IS_DIGIT(*current))
        {
          num *= 10;
          num += *(current++) - '0';
        }
      info->spec.width = num;
    }
  else if (*current == '*')
    {
      ++current;
      info->spec.width = argList[*arg].intValue;
      *arg += 1;
    }
  
  /* Check precision */
  if (*current == '.')
    {
      ++current;
      if (CHAR_IS_DIGIT(*current))
        {
          num = *(current++) - '0';
          while (CHAR_IS_DIGIT(*current))
            {
              num *= 10;
              num += *(current++) - '0';
            }
          info->spec.precision = num;
        }
      else if (*current == '*')
        {
          ++current;
          info->spec.precision = argList[*arg].intValue;
          *arg += 1;
        }
      else
        {
          return (current - start);
        }
    }
  
  /* Check length */
  switch (*current)
    {
      case 'L':
        ++current;
        info->spec.length = CFLongDoubleLength;
        break;
      case 'h':
        ++current;
        if (*current == 'h')
          {
            ++current;
            info->spec.length = CFCharLength;
          }
        else
          {
            info->spec.length = CFShortLength;
          }
        break;
      case 'j':
        ++current;
        info->spec.length = CFIntMaxTLength;
        break;
      case 'l':
        ++current;
        if (*current == 'l')
          {
            ++current;
            info->spec.length = CFLongLongLength;
          }
        else
          {
            info->spec.length = CFLongLength;
          }
        break;
      case 't':
        ++current;
        info->spec.length = CFPtrDiffTLength;
        break;
      case 'z':
        ++current;
        info->spec.length = CFSizeTLength;
        break;
    }
  
  /* Check type */
  if (*current == '%')
    {
      ++current;
      info->fmt = CFFormatPercent;
      return (current - start);
    }
  typeIdx = *(current++) - CHAR_MIN_TYPE;
  if (typeIdx < CHAR_MAX_TYPE)
    {
      info->fmt = _kCFStringFormatter[typeIdx].formatter;
      info->spec.type = _kCFStringFormatter[typeIdx].type;
      if (info->spec.argPos < 0)
        {
          info->spec.argPos = *arg;
          *arg += 1;
        }
      info->spec.useCaps = (typeIdx < CHAR_CAPS_TYPE);
    }
  else
    {
      info->spec.type = CFUnknownType;
    }
  
  return (current - start);
}

void
_CFStringAppendFormatAndArgumentsAux (CFMutableStringRef outputString,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFDictionaryRef fmtOpts, CFStringRef formatString, va_list args)
{
  CFIndex length;
  CFIndex argNum;
  CFIndex specLength;
  Boolean shouldFree;
  CFFormatArgument *argList;
  CFFormatArgument arg;
  CFFormatSpecInfo info;
  UniChar *current;
  UniChar *uformatString;
  UniChar *start;
  const UniChar *end;
  CFStringRef formattedString;
  
  /* Make sure have a UniChar string */
  uformatString = (UniChar *)CFStringGetCharactersPtr (formatString);
  length = CFStringGetLength (formatString);
  shouldFree = false;
  if (uformatString == NULL)
    {
      uformatString = (UniChar *)CFAllocatorAllocate (NULL,
        length * sizeof(UniChar), 0);
      CFStringGetCharacters (formatString, CFRangeMake(0, length),
        uformatString);
      shouldFree = true;
    }
  
  /* Get the arguments */
  end = (uformatString + length);
  argList = CFStringFormatCreateArgumentList (uformatString, end, args);
  if (argList == NULL)
    return;
  
  /* Write to outputString */
  current = uformatString;
  argNum = 0;
  for (;;)
    {
      start = current;
      while (current < end && *current != '%')
        ++current;
      
      if (current != start)
        CFStringAppendCharacters (outputString, start, (current - start));
      if (current == end)
        break;
      
      specLength = CFStringFormatParseSpec(current, end, &info, argList,
        &argNum);
      
      /* handle it */
      if (info.spec.type == CFUnknownType)
        {
          CFStringAppendCharacters (outputString, current, specLength);
        }
      else
        {
          arg = argList[info.spec.argPos];
          formattedString = (info.fmt)(&(info.spec), copyDescFunc, &arg,
            fmtOpts);
          if (formattedString)
            {
              CFStringAppend (outputString, formattedString);
              CFRelease (formattedString);
            }
          else
            {
              CFStringAppendCharacters (outputString, current, specLength);
            }
        }
      
      current += specLength;
    }
  
  CFAllocatorDeallocate (NULL, argList);
  if (shouldFree == true)
    CFAllocatorDeallocate (NULL, uformatString);
}

CFStringRef
_CFStringCreateWithFormatAndArgumentsAux (CFAllocatorRef alloc,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFDictionaryRef fmtOpts, CFStringRef fmtStr, va_list args)
{
  CFStringRef ret;
  CFMutableStringRef string;
  
  string = CFStringCreateMutable (alloc, 0);
  _CFStringAppendFormatAndArgumentsAux (string, copyDescFunc, fmtOpts,
    fmtStr, args);
  
  ret = CFStringCreateCopy (alloc, string);
  CFRelease (string);
  return ret;
}

