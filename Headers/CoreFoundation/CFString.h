/* CFString.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of CoreBase.
   
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

#ifndef __COREFOUNDATION_CFSTRING_H__
#define __COREFOUNDATION_CFSTRING_H__

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>

#include <stdarg.h>

//
// Data Types
//
typedef CFOptionFlags CFStringCompareFlags;
typedef UInt32 CFStringEncoding;

typedef struct _CFStringInlineBuffer CFStringInlineBuffer;
struct _CFStringInlineBuffer
{
  UniChar buffer[64];
  CFStringRef theString;
  const UniChar *directBuffer;
  CFRange rangeToBuffer;
  CFIndex bufferedRangeStart;
  CFIndex bufferedRangeEnd;
};

//
// Constants
//
enum CFStringCompareFlags
{
  kCFCompareCaseInsensitive = 1,
  kCFCompareBackwards = 4,
  kCFCompareAnchored = 8,
/* FIXME: Not supported by GNUstep
  kCFCompareNonliteral = 16,
  kCFCompareLocalized = 32,
*/
  kCFCompareNumerically = 64,
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
  kCFCompareDiacriticInsensitive = 128,
  kCFCompareWidthInsensitive = 256,
  kCFCompareForcedOrdering = 512
#endif
};

enum CFStringBuiltInEncodings
{
  kCFStringEncodingMacRoman = 0,
  kCFStringEncodingWindowsLatin1 = 0x0500,
  kCFStringEncodingISOLatin1 = 0x0201,
  kCFStringEncodingNextStepLatin = 0x0B01,
  kCFStringEncodingASCII = 0x0600,
  kCFStringEncodingUnicode = 0x0100,
  kCFStringEncodingUTF8 = 0x08000100,
  kCFStringEncodingNonLossyASCII = 0x0BFF,
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
  kCFStringEncodingUTF16 = 0x0100,
  kCFStringEncodingUTF16BE = 0x10000100,
  kCFStringEncodingUTF16LE = 0x14000100,
  kCFStringEncodingUTF32 = 0x0c000100,
  kCFStringEncodingUTF32BE = 0x18000100,
  kCFStringEncodingUTF32LE = 0x1c000100
#endif
};

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
# define kCFStringEncodingInvalidId (0xffffffffU)
#endif

#ifdef __OBJC__
#define CFSTR(x) (@ x)
#else
#	ifndef __has_builtin
#	define __has_builtin(x) 0
#	endif
#	if __has_builtin(__builtin___NSStringMakeConstantString)
#		define CFSTR(x) __builtin___NSStringMakeConstantString(x)
	#endif
#endif

typedef enum _CFStringNormalizationForm CFStringNormalizationForm;
enum _CFStringNormalizationForm
{
  kCFStringNormalizationFormD = 0,
  kCFStringNormalizationFormKD = 1,
  kCFStringNormalizationFormC = 2,
  kCFStringNormalizationFormKC = 3
};

const CFStringRef kCFStringTransformStripCombiningMarks;
const CFStringRef kCFStringTransformToLatin;
const CFStringRef kCFStringTransformFullwidthHalfwidth;
const CFStringRef kCFStringTransformLatinKatakana;
const CFStringRef kCFStringTransformLatinHiragana;
const CFStringRef kCFStringTransformHiraganaKatakana;
const CFStringRef kCFStringTransformMandarinLatin;
const CFStringRef kCFStringTransformLatinHangul;
const CFStringRef kCFStringTransformLatinArabic;
const CFStringRef kCFStringTransformLatinHebrew;
const CFStringRef kCFStringTransformLatinThai;
const CFStringRef kCFStringTransformLatinCyrillic;
const CFStringRef kCFStringTransformLatinGreek;
const CFStringRef kCFStringTransformToXMLHex;
const CFStringRef kCFStringTransformToUnicodeName;
const CFStringRef kCFStringTransformStripDiacritics;

//
// Creating a CFString
//
CFArrayRef
CFStringCreateArrayBySeparatingStrings (CFAllocatorRef alloc,
  CFStringRef theString, CFStringRef separatorString);

CFStringRef
CFStringCreateByCombiningStrings (CFAllocatorRef alloc, CFArrayRef theArray,
  CFStringRef separatorString);

CFStringRef
CFStringCreateCopy (CFAllocatorRef alloc, CFStringRef theString);

CFStringRef
CFStringCreateFromExternalRepresentation (CFAllocatorRef alloc, CFDataRef data,
  CFStringEncoding encoding);

CFStringRef
CFStringCreateWithBytes (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalRepresentation);

CFStringRef
CFStringCreateWithCharacters (CFAllocatorRef alloc, const UniChar *chars,
  CFIndex numChars);

CFStringRef
CFStringCreateWithCharactersNoCopy (CFAllocatorRef alloc, const UniChar *chars,
  CFIndex numChars, CFAllocatorRef contentsDeallocator);

CFStringRef
CFStringCreateWithCString (CFAllocatorRef alloc, const char *cStr,
  CFStringEncoding encoding);

CFStringRef
CFStringCreateWithCStringNoCopy (CFAllocatorRef alloc, const char *cStr,
  CFStringEncoding encoding, CFAllocatorRef contentsDeallocator);

CFStringRef
CFStringCreateWithFormat (CFAllocatorRef alloc, CFDictionaryRef formatOptions,
  CFStringRef format, ...);

CFStringRef
CFStringCreateWithFormatAndArguments (CFAllocatorRef alloc,
  CFDictionaryRef formatOptions, CFStringRef format, va_list arguments);

CFStringRef
CFStringCreateWithPascalString (CFAllocatorRef alloc, ConstStr255Param pStr,
  CFStringEncoding encoding);

CFStringRef
CFStringCreateWithPascalStringNoCopy (CFAllocatorRef alloc,
  ConstStr255Param pStr, CFStringEncoding encoding,
  CFAllocatorRef contentsDeallocate);

CFStringRef
CFStringCreateWithSubstring (CFAllocatorRef alloc, CFStringRef str, CFRange range);

//
// Searching Strings
//
CFArrayRef
CFStringCreateArrayWithFindResults (CFAllocatorRef alloc, CFStringRef theString,
  CFStringRef stringToFind, CFRange rangeToSearch,
  CFStringCompareFlags compareOptions);

CFRange
CFStringFind (CFStringRef theString, CFStringRef stringToFind,
  CFStringCompareFlags compareOptions);

Boolean
CFStringFindWithOptions (CFStringRef theString, CFStringRef stringToFind,
  CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result);

Boolean
CFStringFindWithOptionsAndLocale (CFStringRef theString,CFStringRef stringToFind,
  CFRange rangeToSearch, CFStringCompareFlags searchOptions,
  CFLocaleRef locale, CFRange *result);

void
CFStringGetLineBounds (CFStringRef theString, CFRange range,
  CFIndex *lineBeginIndex, CFIndex *lineEndIndex, CFIndex *contentsEndIndex);

//
// Comparing Strings
//
CFComparisonResult
CFStringCompare (CFStringRef theString1, CFStringRef theString2,
  CFStringCompareFlags compareOptions);

CFComparisonResult
CFStringCompareWithOptions (CFStringRef theString1, CFStringRef theString2,
  CFRange rangeToCOmpare, CFStringCompareFlags compareOptions);

Boolean
CFStringHasPrefix (CFStringRef theString, CFStringRef prefix);

Boolean
CFStringHasSuffix (CFStringRef theString, CFStringRef suffix);

//
// Accessing Characters
//
CFDataRef
CFStringCreateExternalRepresentation (CFAllocatorRef alloc,
  CFStringRef theString, CFStringEncoding encoding, UInt8 lossByte);

CFIndex
CFStringGetBytes (CFStringRef theString, CFRange range,
  CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation,
  UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen);

UniChar
CFStringGetCharacterAtIndex (CFStringRef theString, CFIndex idx);

UniChar
CFStringGetCharacterFromInlineBuffer (CFStringInlineBuffer *buf, CFIndex idx);

void
CFStringGetCharacters (CFStringRef theString, CFRange range, UniChar *buffer);

const UniChar *
CFStringGetCharactersPtr (CFStringRef theString);

Boolean
CFStringGetCString (CFStringRef theString, char *buffer, CFIndex bufferSize,
  CFStringEncoding encoding);

const char *
CFStringGetCStringPtr (CFStringRef theString, CFStringEncoding encoding);

CFIndex
CFStringGetLength (CFStringRef str);

Boolean
CFStringGetPascalString (CFStringRef theString, StringPtr buffer,
  CFIndex bufferSize, CFStringEncoding encoding);

ConstStringPtr
CFStringGetPascalStringPtr (CFStringRef theString, CFStringEncoding encoding);

CFRange
CFStringGetRangeOfComposedCharactersAtIndex (CFStringRef theString,
  CFIndex theIndex);

void
CFStringInitInlineBuffer (CFStringRef str, CFStringInlineBuffer *buf,
  CFRange range);

//
// Working With Encodings
//
CFStringRef
CFStringConvertEncodingToIANACharSetName (CFStringEncoding encoding);

unsigned long
CFStringConvertEncodingToNSStringEncoding (CFStringEncoding encoding);

UInt32
CFStringConvertEncodingToWindowsCodepage (CFStringEncoding encoding);

CFStringEncoding
CFStringConvertIANACharSetNameToEncoding (CFStringRef theString);

CFStringEncoding
CFStringConvertNSStringEncodingToEncoding (unsigned long encoding);

CFStringEncoding
CFStringConvertWindowsCodepageToEncoding (UInt32 codepage);

CFStringEncoding
CFStringGetFastestEncoding (CFStringRef theString);

const CFStringEncoding *
CFStringGetListOfAvailableEncodings (void);

CFIndex
CFStringGetMaximumSizeForEncoding (CFIndex length, CFStringEncoding encoding);

CFStringEncoding
CFStringGetMostCompatibleMacStringEncoding (CFStringEncoding encoding);

CFStringRef
CFStringGetNameOfEncoding (CFStringEncoding encoding);

CFStringEncoding
CFStringGetSmallestEncoding (CFStringRef theString);

CFStringEncoding
CFStringGetSystemEncoding (void);

Boolean
CFStringIsEncodingAvailable (CFStringEncoding encoding);

//
// Getting Numeric Values
//
double
CFStringGetDoubleValue (CFStringRef str);

SInt32
CFStringGetIntValue (CFStringRef str);

//
// Getting String Properties
//
void
CFShowStr (CFStringRef str);

CFTypeID
CFStringGetTypeID (void);

//
// New Functions
//
#if 0 //OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
Boolean
CFStringFindCharacterFromSet (CFStringRef theString, CFCharacterSetRef theSet,
  CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result);
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
CFStringRef
CFStringCreateWithFileSystemRepresentation (CFAllocatorRef alloc,
  const char *buffer);

Boolean
CFStringGetFileSystemRepresentation (CFStringRef string, char *buffer,
  CFIndex maxBufLen);

CFIndex
CFStringGetMaximumSizeOfFileSystemRepresentation (CFStringRef string);
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
CFComparisonResult
CFStringCompareWithOptionsAndLocale (CFStringRef theString1,
  CFStringRef theString2, CFRange rangeToCOmpare,
  CFStringCompareFlags compareOptions, CFLocaleRef locale);

CFStringRef
CFStringCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalReprentation,
  CFAllocatorRef contentsDeallocator);

void
CFStringGetParagraphBounds (CFStringRef string, CFRange range,
  CFIndex *parBeginIndex, CFIndex *parEndIndex, CFIndex *contentsEndIndex);
#endif

//
// CFMutableString
//
void
CFStringAppend (CFMutableStringRef theString, CFStringRef appendedString);

void
CFStringAppendCharacters (CFMutableStringRef theString,
  const UniChar *chars, CFIndex numChars);

void
CFStringAppendCString (CFMutableStringRef theString, const char *cStr,
  CFStringEncoding encoding);

void
CFStringAppendFormat (CFMutableStringRef theString,
  CFDictionaryRef formatOptions, CFStringRef format, ...);

void
CFStringAppendFormatAndArguments (CFMutableStringRef theString,
  CFDictionaryRef formatOptions, CFStringRef format, va_list arguments);

void
CFStringAppendPascalString (CFMutableStringRef theString,
  ConstStr255Param pStr, CFStringEncoding encoding);

void
CFStringCapitalize (CFMutableStringRef theString, CFLocaleRef locale);

CFMutableStringRef
CFStringCreateMutable (CFAllocatorRef alloc, CFIndex maxLength);

CFMutableStringRef
CFStringCreateMutableCopy (CFAllocatorRef alloc, CFIndex maxLength,
  CFStringRef theString);

CFMutableStringRef
CFStringCreateMutableWithExternalCharactersNoCopy (CFAllocatorRef alloc,
  UniChar *chars, CFIndex numChars, CFIndex capacity,
  CFAllocatorRef externalCharactersAllocator);

void
CFStringDelete (CFMutableStringRef theString, CFRange range);

CFIndex
CFStringFindAndReplace (CFMutableStringRef theString,
  CFStringRef stringToFind, CFStringRef replacementString,
  CFRange rangeToSearch, CFOptionFlags compareOptions);

void
CFStringFold (CFMutableStringRef theString, CFOptionFlags theFlags,
  CFLocaleRef theLocale);

void
CFStringInsert (CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr);

void
CFStringLowercase (CFMutableStringRef theString, CFLocaleRef locale);

//void
//CFStringNormalize (CFMutableStringRef theString,
//  CFStringNormalizationForm theForm);

void
CFStringPad (CFMutableStringRef theString, CFStringRef padString,
  CFIndex length, CFIndex indexIntoPad);

void
CFStringReplace (CFMutableStringRef theString, CFRange range,
  CFStringRef replacement);

void
CFStringSetExternalCharactersNoCopy (CFMutableStringRef theString,
  UniChar *chars, CFIndex length, CFIndex capacity);

Boolean
CFStringTransform (CFMutableStringRef string, CFRange *range,
  CFStringRef transform, Boolean reverse);

void
CFStringTrim (CFMutableStringRef theString, CFStringRef trimString);

void
CFStringTrimWhitespace (CFMutableStringRef theString);

void
CFStringUppercase (CFMutableStringRef theString, CFLocaleRef locale);

#endif /* __COREFOUNDATION_CFSTRING_H__ */
