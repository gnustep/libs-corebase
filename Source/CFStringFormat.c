/* CFStringFormat.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: June, 2011
   
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
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFNumberFormatter.h"

#define CF_FMT_FLAG_SIGN       (1<<0)
#define CF_FMT_FLAG_LEFT_ALIGN (1<<1)
#define CF_FMT_FLAG_ALT        (1<<2)
#define CF_FMT_FLAG_PAD        (1<<3)

#define CF_FMT_POUND    0x0023
#define CF_FMT_DOLLAR   0x0024
#define CF_FMT_PERCENT  0x0025
#define CF_FMT_ASTERISK 0x002A
#define CF_FMT_PLUS     0x002B
#define CF_FMT_MINUS    0x002D
#define CF_FMT_PERIOD   0x002E

#define CF_FMT_ZERO  0x0030
#define CF_FMT_ONE   0x0031
#define CF_FMT_TWO   0x0032
#define CF_FMT_THREE 0x0033
#define CF_FMT_FOUR  0x0034
#define CF_FMT_FIVE  0x0035
#define CF_FMT_SIX   0x0036
#define CF_FMT_SEVEN 0x0037
#define CF_FMT_EIGHT 0x0038
#define CF_FMT_NINE  0x0039

#define CF_FMT_CAP_L 0x004C
#define CF_FMT_H 0x0068
#define CF_FMT_L 0x006C
#define CF_FMT_Z 0x007A
#define CF_FMT_J 0x006A
#define CF_FMT_T 0x0074

#define CF_FMT_IS_DIGIT(c) (c) >= CF_FMT_ZERO && (c) <= CF_FMT_NINE
#define CF_FMT_IS_FLAG(c) (c) == CF_FMT_PLUS \
  || (c) == CF_FMT_MINUS \
  || (c) == CF_FMT_POUND \
  || (c) == CF_FMT_ZERO
#define CF_FMT_IS_LENGTH(c) (c) == CF_FMT_H \
  || (c) == CF_FMT_L \
  || (c) == CF_FMT_CAP_L \
  || (c) == CF_FMT_Z \
  || (c) == CF_FMT_J \
  || (c) == CF_FMT_T

typedef enum
{
  CFCharIntLength,
  CFShortIntLength,
  CFLongLength,
  CFLongLongLength,
  CFLongDoubleLength,
  CFSizeTLength,
  CFIntMaxTLength,
  CFPtrDiffLength
} CFArgLength;

typedef enum
{
  CFUnknownType,
  CFUnsignedType,
  CFIntegerType,
  CFFloatType,
  CFDoubleType,
  CFScientificType,
  CFHexType,
  CFOctalType,
  CFStringType,
  CFCharType,
  CFPointerType,
  CFObjectType
} CFArgType;

typedef union
{
  SInt64  intValue; // Holds every possible type of integer
  double  doubleValue;
  void   *ptrValue; // Includes object pointers
} CFFormatArgument;

/* For information only:
     %[parameter][flags][width][.precision][length]type */
typedef struct
{
  CFIndex       argPos;
  CFArgType     type;
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
#define CFUnsignedFormat   { CFUnsignedType,   CFIntegerFormatter }
#define CFIntegerFormat    { CFIntegerType,    CFIntegerFormatter }
#define CFFloatFormat      { CFFloatType,      CFDoubleFormatter }
#define CFDoubleFormat     { CFDoubleType,     CFDoubleFormatter }
#define CFScientificFormat { CFScientificType, CFDoubleFormatter }
#define CFHexFormat        { CFHexType,        CFHexFormatter }
#define CFOctalFormat      { CFOctalType,      CFOctalFormatter }
#define CFStringFormat     { CFStringType,     CFStringFormatter }
#define CFCharFormat       { CFCharType,       CFCharFormatter }
#define CFPointerFormat    { CFPointerType,    CFHexFormatter }
#define CFObjectFormat     { CFObjectType,     CFObjectFormatter }

static CFStringRef CFIntegerFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  CFStringRef ret;
  CFNumberRef num;
  CFAllocatorRef alloc;
  CFNumberFormatterRef fmt;
  
  alloc = CFAllocatorGetDefault ();
  fmt = CFNumberFormatterCreate (alloc, NULL, kCFNumberFormatterNoStyle);
  
  
  ret = CFNumberFormatterCreateStringWithValue (NULL, fmt,
    kCFNumberIntType, (const void*)&(arg->intValue));
  
  CFRelease(fmt);
  return ret;
}

static CFStringRef CFDoubleFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  return CFSTR("Double");
}

static CFStringRef CFHexFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  return CFSTR("Hex");
}

static CFStringRef CFOctalFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  return CFSTR("Octal");
}

static CFStringRef CFStringFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  return CFSTR("String");
}

static CFStringRef CFCharFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  return CFStringCreateWithBytes (CFAllocatorGetDefault(),
    (const UInt8*)&arg->intValue, 1, CFStringGetSystemEncoding(), false);
}

static CFStringRef CFObjectFormatter (CFFormatSpec *spec,
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

static CFStringRef CFPercentFormatter (CFFormatSpec *spec,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFFormatArgument *arg,
  CFDictionaryRef formatOptions)
{
  return CFSTR("%");
}

static const CFFormatFormatterInfo _kCFStringFormatter[] =
{
  /* 0x40 */
  CFObjectFormat,    CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFUnknownFormat,   CFScientificFormat, CFFloatFormat,      CFDoubleFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
  /* 0x50 */
  CFUnknownFormat,   CFUnknownFormat,    CFUnknownFormat,    CFUnknownFormat,
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

#define CF_FMT_MIN_TYPE 0x0040
#define CF_FMT_MAX_TYPE \
  (sizeof(_kCFStringFormatter) / sizeof(CFFormatFormatterInfo))

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
  CFIndex count = 0;
  
  /* Count total number of arguments
     I'm just going to count how many '%' there are and multiply that by
     3 because that's the largest amount of arguments each spec (ie '%*.*s') */
  current = start;
  for(;;)
    {
      while (*current != CF_FMT_PERCENT && current < end)
        ++current;
      if (current == end)
        break;
      ++current; // Skip %
      ++count;
    }
  
  count *= 3; // Multiply by 3
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
      while (*current != CF_FMT_PERCENT && current < end)
        ++current;
      if (current == end)
        break;
      ++current; // Skip %
      
      if (*current == CF_FMT_PERCENT) // Skip %%
        {
          ++current;
          continue;
        }
      
      for (;;)
        {
          if (CF_FMT_IS_DIGIT(*current))
            {
              int num = *(current++) - CF_FMT_ZERO;
              while (CF_FMT_IS_DIGIT(*current))
                {
                  num *= 10;
                  num += *(current++) - CF_FMT_ZERO;
                }
              if (*current == CF_FMT_DOLLAR)
                {
                  int callout = num - 1;
                  ++current;
                  typeList[callout] = CFIntegerType;
                  if (callout > count)
                    count = callout;
                }
            }
          else if (CF_FMT_IS_FLAG(*current))
            {
              ++current;
            }
          else if (*current == CF_FMT_ASTERISK)
            {
              typeList[pos] = CFIntegerType;
              pos += 1;
              ++current;
            }
          else if (*current == CF_FMT_PERIOD)
            {
              ++current;
            }
          else
            {
              break;
            }
        }
      is64Bits[pos] = false;
      if (*(current) == CF_FMT_H)
        {
          ++current;
          if (*current == CF_FMT_H)
            ++current;
        }
      else if (*(current) == CF_FMT_L)
        {
          ++current;
#if __LP64__
          is64Bits[pos] = true;
#endif
          if (*(current) == CF_FMT_L)
            {
#if !__LP64__
              is64Bits[pos] = true;
#endif
              ++current;
            }
        }
      type = *current;
      
      typeIdx = type - CF_FMT_MIN_TYPE;
      if (typeIdx < CF_FMT_MAX_TYPE)
        typeList[pos++] = _kCFStringFormatter[typeIdx].type;
      else
        typeList[pos++] = CFUnknownType;
      
      if (pos < count)
        pos = count;
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
  ++current; // Skip %
  
  /* Initialize the spec */
  memset (info, 0, sizeof(CFFormatSpecInfo));
  info->spec.argPos = -1;
  info->spec.width = -1;
  info->spec.precision = -1;
  
  /* Check parameter field */
  if (CF_FMT_IS_DIGIT(*current))
    {
      UniChar *revert = current;
      
      num = *(current++) - CF_FMT_ZERO;
      while (CF_FMT_IS_DIGIT(*current))
        {
          num *= 10;
          num += *(current++) - CF_FMT_ZERO;
        }
      if (*current == CF_FMT_DOLLAR)
        {
          ++current;
          info->spec.argPos = num - 1;
        }
      else // Must be a width argument
        {
          current = revert;
        }
    }
  
  /* Check flags */
  while (CF_FMT_IS_FLAG(*current))
    {
      switch (*current)
        {
          case CF_FMT_PLUS:
            info->spec.flags |= CF_FMT_FLAG_SIGN;
            break;
          case CF_FMT_MINUS:
            info->spec.flags |= CF_FMT_FLAG_LEFT_ALIGN;
            break;
          case CF_FMT_POUND:
            info->spec.flags |= CF_FMT_FLAG_ALT;
            break;
          case CF_FMT_ZERO:
            info->spec.flags |= CF_FMT_FLAG_PAD;
            break;
        }
      ++current;
    }
  
  /* Check width */
  if (CF_FMT_IS_DIGIT(*current))
    {
      num = *(current++) - CF_FMT_ZERO;
      while (CF_FMT_IS_DIGIT(*current))
        {
          num *= 10;
          num += *(current++) - CF_FMT_ZERO;
        }
      info->spec.width = num;
    }
  else if (*current == CF_FMT_ASTERISK)
    {
      ++current;
      info->spec.width = argList[*arg].intValue;
      *arg += 1;
    }
  
  /* Check precision */
  if (*current == CF_FMT_PERIOD)
    {
      ++current;
      if (CF_FMT_IS_DIGIT(*current))
        {
          num = *(current++) - CF_FMT_ZERO;
          while (CF_FMT_IS_DIGIT(*current))
            {
              num *= 10;
              num += *(current++) - CF_FMT_ZERO;
            }
          info->spec.precision = num;
        }
      else if (*current == CF_FMT_ASTERISK)
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
  // FIXME
  
  /* Check type */
  if (*current == CF_FMT_PERCENT)
    {
      ++current;
      info->fmt = CFPercentFormatter;
      return (current - start);
    }
  typeIdx = *(current++) - CF_FMT_MIN_TYPE;
  if (typeIdx < CF_FMT_MAX_TYPE)
    {
      info->fmt = _kCFStringFormatter[typeIdx].formatter;
      info->spec.type = _kCFStringFormatter[typeIdx].type;
      if (info->spec.argPos < 0)
        {
          info->spec.argPos = *arg;
          *arg += 1;
        }
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
      while (*current != CF_FMT_PERCENT && current < end)
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
