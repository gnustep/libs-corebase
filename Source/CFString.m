/* CFString.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of GNUstep CoreBase Library.
   
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

#include <Foundation/Foundation.h>

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFCharacterSet.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"

#include <stdarg.h>

const CFStringRef kCFStringTransformStripCombiningMarks =
  @"kCFStringTransformStripCombiningMarks";
const CFStringRef kCFStringTransformToLatin =
  @"kCFStringTransformToLatin";
const CFStringRef kCFStringTransformFullwidthHalfwidth =
  @"kCFStringTransformFullwidthHalfwidth";
const CFStringRef kCFStringTransformLatinKatakana =
  @"kCFStringTransformLatinKatakana";
const CFStringRef kCFStringTransformLatinHiragana =
  @"kCFStringTransformLatinHiragana";
const CFStringRef kCFStringTransformHiraganaKatakana  =
  @"kCFStringTransformHiraganaKatakana";
const CFStringRef kCFStringTransformMandarinLatin =
  @"kCFStringTransformMandarinLatin";
const CFStringRef kCFStringTransformLatinHangul =
  @"kCFStringTransformLatinHangul";
const CFStringRef kCFStringTransformLatinArabic =
  @"kCFStringTransformLatinArabic";
const CFStringRef kCFStringTransformLatinHebrew =
  @"kCFStringTransformLatinHebrew";
const CFStringRef kCFStringTransformLatinThai =
  @"kCFStringTransformLatinThai";
const CFStringRef kCFStringTransformLatinCyrillic =
  @"kCFStringTransformLatinCyrillic";
const CFStringRef kCFStringTransformLatinGreek =
  @"kCFStringTransformLatinGreek";
const CFStringRef kCFStringTransformToXMLHex =
  @"kCFStringTransformToXMLHex";
const CFStringRef kCFStringTransformToUnicodeName =
  @"kCFStringTransformToUnicodeName";
const CFStringRef kCFStringTransformStripDiacritics =
  @"kCFStringTransformStripDiacritics";



void CFShowStr (CFStringRef str)
{
  /* FIXME: unimplemented */
}

CFComparisonResult CFStringCompare (CFStringRef theString1,
  CFStringRef theString2, CFStringCompareFlags compareOptions)
{
  return CFStringCompareWithOptions (theString1, theString2,
    (CFRange){0, CFStringGetLength(theString1)}, compareOptions);
}

CFComparisonResult CFStringCompareWithOptions (CFStringRef theString1,
  CFStringRef theString2, CFRange rangeToCompare,
  CFStringCompareFlags compareOptions)
{
  return [theString1 compare: (NSString *)theString2
                     options: compareOptions
                       range: rangeToCompare];
}

CFComparisonResult CFStringCompareWithOptionsAndLocale (CFStringRef theString1,
  CFStringRef theString2, CFRange rangeToCompare,
  CFStringCompareFlags compareOptions, CFLocaleRef locale)
{
  return [theString1 compare: theString2
                     options: compareOptions
                       range: rangeToCompare
                      locale: (NSDictionary *) locale]; // is this right?
}

CFStringRef CFStringConvertEncodingToIANACharSetName (CFStringEncoding encoding)
{
  return NULL;
}

unsigned long CFStringConvertEncodingToNSStringEncoding
  (CFStringEncoding encoding)
{
  /* FIXME:  Someone please look over these! */
  switch (encoding)
    {
      case kCFStringEncodingMacRoman:
        return NSMacOSRomanStringEncoding;
      case kCFStringEncodingWindowsLatin1:
        return NSWindowsCP1252StringEncoding;
      case kCFStringEncodingISOLatin1:
        return NSISOLatin1StringEncoding;
      case kCFStringEncodingNextStepLatin:
        return NSNEXTSTEPStringEncoding;
      case kCFStringEncodingASCII:
        return NSASCIIStringEncoding;
      case kCFStringEncodingUnicode:
        return NSUnicodeStringEncoding;
      case kCFStringEncodingUTF8:
        return NSUTF8StringEncoding;
      case kCFStringEncodingNonLossyASCII:
        return NSNonLossyASCIIStringEncoding;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
      case kCFStringEncodingUTF16BE:
        return NSUTF16BigEndianStringEncoding;
      case kCFStringEncodingUTF16LE:
        return NSUTF16LittleEndianStringEncoding;
      case kCFStringEncodingUTF32:
        return NSUTF32StringEncoding;
      case kCFStringEncodingUTF32BE:
        return NSUTF32BigEndianStringEncoding;
      case kCFStringEncodingUTF32LE:
        return NSUTF32LittleEndianStringEncoding;
#endif
      default:
        return GSUndefinedEncoding;
    }
}

UInt32 CFStringConvertEncodingToWindowsCodepage (CFStringEncoding encoding)
{
  return 0;
}

CFStringEncoding CFStringConvertIANACharSetNameToEncoding
  (CFStringRef theString)
{
  return 0;
}

CFStringEncoding CFStringConvertNSStringEncodingToEncoding
  (unsigned long encoding)
{
  switch (encoding)
    {
      case NSASCIIStringEncoding:
        return kCFStringEncodingASCII;
      case NSNEXTSTEPStringEncoding:
        return kCFStringEncodingNextStepLatin;
      case NSJapaneseEUCStringEncoding:
        return kCFStringEncodingInvalidId; /* FIXME */
      case NSUTF8StringEncoding:
        return kCFStringEncodingUTF8;
      case NSISOLatin1StringEncoding:
        return kCFStringEncodingISOLatin1;
      case NSSymbolStringEncoding:
        return kCFStringEncodingInvalidId; /* FIXME */
      case NSNonLossyASCIIStringEncoding:
        return kCFStringEncodingNonLossyASCII;
      case NSShiftJISStringEncoding:
        return kCFStringEncodingShiftJIS;
      case NSISOLatin2StringEncoding:
        return kCFStringEncodingISOLatin2;
      case NSUnicodeStringEncoding:
        return kCFStringEncodingUnicode;
      case NSWindowsCP1251StringEncoding:
        return kCFStringEncodingDOSCyrillic;
      case NSWindowsCP1252StringEncoding:
        return kCFStringEncodingWindowsLatin1;
      case NSWindowsCP1253StringEncoding:
        return kCFStringEncodingWindowsGreek;
      case NSWindowsCP1254StringEncoding:
        return kCFStringEncodingDOSTurkish; /* FIXME: ??? */
      case NSWindowsCP1250StringEncoding:
        return kCFStringEncodingWindowsLatin2;
      case NSISO2022JPStringEncoding:
        return kCFStringEncodingISO_2022_JP;
      case NSMacOSRomanStringEncoding:
        return kCFStringEncodingMacRoman;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
      case NSUTF16BigEndianStringEncoding:
        return kCFStringEncodingUTF16BE;
      case NSUTF16LittleEndianStringEncoding:
        return kCFStringEncodingUTF16LE;
      case NSUTF32StringEncoding:
        return kCFStringEncodingUTF32;
      case NSUTF32BigEndianStringEncoding:
        return kCFStringEncodingUTF32BE;
      case NSUTF32LittleEndianStringEncoding:
        return kCFStringEncodingUTF32LE;
#endif
      default:
        return kCFStringEncodingInvalidId;
    }
}

CFStringEncoding CFStringConvertWindowsCodepageToEncoding (UInt32 codepage)
{
  return 0;
}

CFArrayRef CFStringCreateArrayBySeparatingStrings (CFAllocatorRef alloc,
  CFStringRef theString, CFStringRef separatorString)
{
  return [theString componentsSeparatedByString: separatorString];
}

CFArrayRef CFStringCreateArrayWithFindResults (CFAllocatorRef alloc,
  CFStringRef theString, CFStringRef stringToFind, CFRange rangeToSearch,
  CFStringCompareFlags compareOptions)
{
  return NULL;
}

CFStringRef CFStringCreateByCombiningStrings (CFAllocatorRef alloc,
  CFArrayRef theArray, CFStringRef separatorString)
{
  return [theArray componentsJoinedByString: separatorString];
}

CFStringRef CFStringCreateCopy (CFAllocatorRef alloc, CFStringRef theString)
{
  return [theString copy];
}

CFStringRef CFStringCreateExternalRepresentation (CFAllocatorRef alloc,
  CFStringRef theString, CFStringEncoding encoding, UInt8 lossByte)
{
  return NULL;
}

CFStringRef CFStringCreateFromExternalRepresentation (CFAllocatorRef alloc,
  CFDataRef data, CFStringEncoding encoding)
{
  return [[NSString allocWithZone: alloc] initWithData: (NSData *)data
    encoding: CFStringConvertEncodingToNSStringEncoding(encoding)];
}

CFStringRef CFStringCreateWithBytes (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalRepresentation)
{
  return CFStringCreateWithBytesNoCopy (alloc, bytes, numBytes, encoding,
    isExternalRepresentation, NULL);
}

CFStringRef CFStringCreateWithBytesNoCopy (CFAllocatorRef alloc,
  const UInt8 *bytes, CFIndex numBytes, CFStringEncoding encoding,
  Boolean isExternalRepresenation, CFAllocatorRef contentsDeallocator)
{
  BOOL freeWhenDone = contentsDeallocator == kCFAllocatorNull ? NO : YES;
  
  return [[NSString allocWithZone: alloc]
    initWithBytesNoCopy: (const char *)bytes
    length: numBytes
    encoding: CFStringConvertEncodingToNSStringEncoding(encoding)
    freeWhenDone: freeWhenDone];
}

CFStringRef CFStringCreateWithCharacters (CFAllocatorRef alloc,
  const UniChar *chars, CFIndex numChars)
{
  return CFStringCreateWithBytes (alloc, (const UInt8 *)chars,
    numChars * sizeof(UniChar), kCFStringEncodingUnicode, false);
}

CFStringRef CFStringCreateWithCharactersNoCopy (CFAllocatorRef alloc,
  const UniChar *chars, CFIndex numChars, CFAllocatorRef contentsDeallocator)
{
  return CFStringCreateWithBytesNoCopy (alloc, (const char *)chars, numChars *
    sizeof(UniChar), CFStringGetSystemEncoding(), false, contentsDeallocator);
}

CFStringRef CFStringCreateWithCString (CFAllocatorRef alloc,
  const char *cStr, CFStringEncoding encoding)
{
  return [[NSString allocWithZone: alloc] initWithCString: cStr
    encoding: CFStringConvertEncodingToNSStringEncoding(encoding)];
}

CFStringRef CFStringCreateWithCStringNoCopy (CFAllocatorRef alloc,
  const char *cStr, CFStringEncoding encoding,
  CFAllocatorRef contentsDeallocator)
{
  BOOL freeWhenDone = contentsDeallocator == kCFAllocatorNull ? NO : YES;
  
  return [[NSString allocWithZone: alloc] initWithBytesNoCopy: cStr
    length: strlen(cStr)
    encoding: CFStringConvertEncodingToNSStringEncoding(encoding)
    freeWhenDone: freeWhenDone];
}

CFStringRef CFStringCreateWithFileSystemRepresentation (CFAllocatorRef alloc,
  const char *buffer)
{
  return CFStringCreateWithCString (alloc, buffer,
    CFStringGetSystemEncoding());
}

CFStringRef CFStringCreateWithFormat (CFAllocatorRef alloc,
  CFDictionaryRef formatOptions, CFStringRef format, ...)
{
  CFStringRef ret;
  va_list ap;
  
  va_start (ap, format);
  ret = CFStringCreateWithFormatAndArguments (alloc, formatOptions,
    format, ap);
  va_end(ap);
  
  return ret;
}

CFStringRef CFStringCreateWithFormatAndArguments (CFAllocatorRef alloc,
  CFDictionaryRef formatOptions, CFStringRef format, va_list arguments)
{
  return [[NSString allocWithZone: alloc]
    initWithFormat: format
         arguments: arguments];
}

CFStringRef CFStringCreateWithPascalString ( CFAllocatorRef alloc,
  ConstStr255Param pStr, CFStringEncoding encoding)
{
  return CFStringCreateWithPascalStringNoCopy (alloc, pStr,
    encoding, NULL);
}

CFStringRef CFStringCreateWithPascalStringNoCopy (CFAllocatorRef alloc,
  ConstStr255Param pStr, CFStringEncoding encoding,
  CFAllocatorRef contentsDeallocator)
{
  /* FIXME: Unimplemented */
  return NULL;
}

CFStringRef CFStringCreateWithSubstring (CFAllocatorRef alloc,
  CFStringRef str, CFRange range)
{
  return [str substringWithRange: range];
}

CFRange CFStringFind (CFStringRef theString, CFStringRef stringToFind,
  CFStringCompareFlags compareOptions)
{
  CFRange ret;
  
  if (CFStringFindWithOptions (theString, stringToFind,
    (CFRange){0, CFStringGetLength(theString)}, compareOptions, &ret) == false)
    {
      ret = CFRangeMake (0, 0);
    }
  
  return ret;
}

Boolean CFStringFindCharacterFromSet (CFStringRef theString,
  CFCharacterSetRef theSet, CFRange rangeToSearch,
  CFStringCompareFlags searchOptions, CFRange *result)
{
  *result = (CFRange)[theString rangeOfCharacterFromSet: theSet
                                                options: searchOptions
                                                  range: rangeToSearch];
  if (result->location == 0 && result->length == 0);
    {
      return false;
    }
  
  return true;
}

Boolean CFStringFindWithOptions (CFStringRef theString,
  CFStringRef stringToFind, CFRange rangeToSearch,
  CFStringCompareFlags searchOptions, CFRange *result)
{
  return CFStringFindWithOptionsAndLocale (theString, stringToFind,
    rangeToSearch, searchOptions, NULL, result);
}

Boolean CFStringFindWithOptionsAndLocale (CFStringRef theString,
  CFStringRef stringToFind, CFRange rangeToSearch,
  CFStringCompareFlags searchOptions, CFLocaleRef locale, CFRange *result)
{
  /* FIXME: correct method is not implemented in GNUstep. */
  *result = (CFRange)[theString rangeOfString: stringToFind
                                      options: searchOptions
                                        range: rangeToSearch];
  if (result->location == 0 && result->length == 0)
    {
      return false;
    }
  return true;
}

CFIndex CFStringGetBytes (CFStringRef theString, CFRange range,
  CFStringEncoding encoding, UInt8 lossByte,
  Boolean isExternalRepresentation, UInt8 *buffer, CFIndex maxBufLen,
  CFIndex *usedBufLen)
{
  return 0;
}

UniChar CFStringGetCharacterAtIndex (CFStringRef theString, CFIndex idx)
{
  return [theString characterAtIndex: idx];
}

UniChar CFStringGetCharacterFromInlineBuffer (CFStringInlineBuffer *buf,
  CFIndex idx)
{
  return 0;
}

void CFStringGetCharacters (CFStringRef theString, CFRange range,
  UniChar *buffer)
{
  [theString getCharacters: buffer range: range];
}

const UniChar * CFStringGetCharactersPtr (CFStringRef theString)
{
  UniChar *ret;
  
  [theString getCharacters: ret];
  
  return ret;
}

Boolean CFStringGetCString (CFStringRef theString, char *buffer,
  CFIndex bufferSize, CFStringEncoding encoding)
{
  return [theString getCString: buffer
    maxLength: bufferSize
    encoding: CFStringConvertEncodingToNSStringEncoding(encoding)];
}

const char * CFStringGetCStringPtr (CFStringRef theString,
  CFStringEncoding encoding)
{
  return [theString
    cStringUsingEncoding: CFStringConvertEncodingToNSStringEncoding(encoding)];
}

double CFStringGetDoubleValue (CFStringRef str)
{
  return [str doubleValue];
}

CFStringEncoding CFStringGetFastestEncoding (CFStringRef theString)
{
  return [theString fastestEncoding];
}

Boolean CFStringGetFileSystemRepresentation (CFStringRef string,
  char *buffer, CFIndex maxBufLen)
{
  return CFStringGetCString (string, buffer, maxBufLen,
    CFStringGetSystemEncoding());
}

SInt32 CFStringGetIntValue (CFStringRef str)
{
  return [str intValue];
}

CFIndex CFStringGetLength (CFStringRef theString)
{
  return [theString length];
}

void CFStringGetLineBounds (CFStringRef theString, CFRange range,
  CFIndex *lineBeginIndex, CFIndex *lineEndIndex, CFIndex *contentsEndIndex)
{
}

const CFStringEncoding * CFStringGetListOfAvailableEncodings (void)
{
  return NULL;
}

CFIndex CFStringGetMaximumSizeForEncoding (CFIndex length,
  CFStringEncoding encoding)
{
  return 0;
}

CFIndex CFStringGetMaximumSizeOfFileSystemRepresentation (CFStringRef string)
{
  return [string
    maximumLengthOfBytesUsingEncoding: [NSString defaultCStringEncoding]];
}

CFStringEncoding CFStringGetMostCompatibleMacStringEncoding
  (CFStringEncoding encoding)
{
  return 0;
}

CFStringRef CFStringGetNameOfEncoding (CFStringEncoding encoding)
{
  /* FIXME: GSPrivateEncodingName () would be nice here */
  return [NSString localizedNameOfStringEncoding:
    CFStringConvertEncodingToNSStringEncoding(encoding)];
}

void CFStringGetParagraphBounds (CFStringRef string, CFRange range,
  CFIndex *parBeginIndex, CFIndex *parEndIndex, CFIndex *contentsEndIndex)
{
  [string getParagraphStart: (NSUInteger *)parBeginIndex
                        end: (NSUInteger *)parEndIndex
                contentsEnd: (NSUInteger *)contentsEndIndex
                   forRange: range];
}

Boolean CFStringGetPascalString (CFStringRef theString,
  StringPtr buffer, CFIndex bufferSize, CFStringEncoding encoding)
{
  return false;
}

ConstStringPtr CFStringGetPascalStringPtr (CFStringRef theString,
  CFStringEncoding encoding)
{
  return NULL;
}

CFRange CFStringGetRangeOfComposedCharactersAtIndex (CFStringRef theString,
  CFIndex theIndex)
{
  return [theString rangeOfComposedCharacterSequenceAtIndex: theIndex];
}

CFStringEncoding CFStringGetSmallestEncoding (CFStringRef theString)
{
  return [theString smallestEncoding];
}

CFStringEncoding CFStringGetSystemEncoding (void)
{
  return CFStringConvertEncodingToNSStringEncoding
    ([NSString defaultCStringEncoding]);
}

CFTypeID CFStringGetTypeID (void)
{
  return (CFTypeID)[NSString class];
}

Boolean CFStringHasPrefix (CFStringRef theString, CFStringRef prefix)
{
  return [theString hasPrefix: prefix];
}

Boolean CFStringHasSuffix (CFStringRef theString, CFStringRef suffix)
{
  return [theString hasSuffix: suffix];
}

void CFStringInitInlineBuffer (CFStringRef str, CFStringInlineBuffer *buf,
  CFRange range)
{
}

Boolean CFStringIsEncodingAvailable (CFStringEncoding encoding)
{
  return false;
}

//
// CFMutableString
//
void CFStringAppend (CFMutableStringRef theString, CFStringRef appendedString)
{
  [theString appendString: appendedString];
}

void CFStringAppendCharacters (CFMutableStringRef theString,
  const UniChar *chars, CFIndex numChars)
{
}

void CFStringAppendCString (CFMutableStringRef theString, const char *cStr,
  CFStringEncoding encoding)
{
}

void CFStringAppendFormat (CFMutableStringRef theString,
  CFDictionaryRef formatOptions, CFStringRef format, ...)
{
  va_list ap;
  
  va_start(ap, format);
  CFStringAppendFormatAndArguments (theString, formatOptions, format, ap);
  va_end(ap);
}

void CFStringAppendFormatAndArguments (CFMutableStringRef theString,
  CFDictionaryRef formatOptions, CFStringRef format, va_list arguments)
{
  /* FIXME: ??? */
}

void CFStringAppendPascalString (CFMutableStringRef theString,
  ConstStr255Param pStr, CFStringEncoding encoding)
{
}

void CFStringCapitalize (CFMutableStringRef theString, CFLocaleRef locale)
{
}

CFMutableStringRef CFStringCreateMutable (CFAllocatorRef alloc,
  CFIndex maxLength)
{
  return [[NSMutableString allocWithZone: alloc] initWithCapacity: maxLength];
}

CFMutableStringRef CFStringCreateMutableCopy (CFAllocatorRef alloc,
  CFIndex maxLength, CFStringRef theString)
{
  return [theString mutableCopyWithZone: alloc];
}

CFMutableStringRef CFStringCreateMutableWithExternalCharactersNoCopy
  (CFAllocatorRef alloc, UniChar *chars, CFIndex numChars, CFIndex capacity,
  CFAllocatorRef externalCharactersAllocator)
{
  BOOL freeWhenDone =
    externalCharactersAllocator == kCFAllocatorNull ? NO : YES;
  
  return [[NSMutableString allocWithZone: alloc]
    initWithCharactersNoCopy: chars
                      length: numChars
                freeWhenDone: freeWhenDone];
}

void CFStringDelete (CFMutableStringRef theString, CFRange range)
{
  [theString deleteCharactersInRange: range];
}

CFIndex CFStringFindAndReplace (CFMutableStringRef theString,
  CFStringRef stringToFind, CFStringRef replacementString,
  CFRange rangeToSearch, CFOptionFlags compareOptions)
{
  return [theString replaceOccurrencesOfString: stringToFind
                                    withString: replacementString
                                       options: compareOptions
                                         range: rangeToSearch];
}

void CFStringFold (CFMutableStringRef theString, CFOptionFlags theFlags,
  CFLocaleRef theLocale)
{
}

void CFStringInsert (CFMutableStringRef str, CFIndex idx,
  CFStringRef insertedStr)
{
  [str insertString: insertedStr atIndex: idx];
}

void CFStringLowercase (CFMutableStringRef theString, CFLocaleRef locale)
{
}

void CFStringNormalize (CFMutableStringRef theString,
  CFStringNormalizationForm theForm)
{
}

void CFStringPad (CFMutableStringRef theString, CFStringRef padString,
  CFIndex length, CFIndex indexIntoPad)
{
}

void CFStringReplace (CFMutableStringRef theString, CFRange range,
  CFStringRef replacement)
{
  [theString replaceCharactersInRange: range withString: replacement];
}

void CFStringSetExternalCharactersNoCopy (CFMutableStringRef theString,
  UniChar *chars, CFIndex length, CFIndex capacity)
{
}

Boolean CFStringTransform (CFMutableStringRef string, CFRange *range,
  CFStringRef transform, Boolean reverse)
{
}

void CFStringTrim (CFMutableStringRef theString, CFStringRef trimString)
{
}

void CFStringTrimWhitespace (CFMutableStringRef theString)
{
}

void CFStringUppercase (CFMutableStringRef theString, CFLocaleRef locale)
{
}
