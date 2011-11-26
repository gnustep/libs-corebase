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

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFCharacterSet.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFLocale.h>

#include <stdarg.h>

CF_EXTERN_C_BEGIN

/** @defgroup CFString
    @brief The CFString type defines opaque objects representing strings.
    
    CFString is "toll-free bridged" to NSString.
    @{
 */

//
// Data Types
//
typedef UInt32 CFStringEncoding;

//
// Constants
//
enum CFStringCompareFlags
{
  kCFCompareCaseInsensitive = 1,
  kCFCompareBackwards = 4,
  kCFCompareAnchored = 8,
  kCFCompareNonliteral = 16,
  kCFCompareLocalized = 32,
  kCFCompareNumerically = 64,
#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
  kCFCompareDiacriticInsensitive = 128,
  kCFCompareWidthInsensitive = 256,
  kCFCompareForcedOrdering = 512
#endif
};
typedef enum CFStringCompareFlags CFStringCompareFlags;

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
#if MAC_OS_X_VERSION_10_4 <= MAC_OS_X_VERSION_MAX_ALLOWED
  kCFStringEncodingUTF16 = 0x0100,
  kCFStringEncodingUTF16BE = 0x10000100,
  kCFStringEncodingUTF16LE = 0x14000100,
  kCFStringEncodingUTF32 = 0x0c000100,
  kCFStringEncodingUTF32BE = 0x18000100,
  kCFStringEncodingUTF32LE = 0x1c000100
#endif
};

#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
# define kCFStringEncodingInvalidId (0xffffffffU)
#endif

/** @def CFSTR(x)
    @brief Creates a constant string object.
    
    @warning This macro will create the constant string at runtime.
 */
/* FIXME: Unfortunately, NSConstantString does not get completely initialized.
   We cannot use @"" or __builtin___NSStringMakeConstantString().
#ifdef __OBJC__
 If we're in Objective-C mode, just make this an ObjC string.
#define CFSTR(x) ((CFStringRef)(@ x))
#else
  // If this compiler doesn't have __has_builtin(), it probably doesn't have
  // any useful builtins  either
# ifndef __has_builtin
#   define __has_builtin(x) 0
# endif
  // If we have a builtin function for constructing Objective-C strings,
  // let's use that.
# if __has_builtin(__builtin___NSStringMakeConstantString)
#   define CFSTR(x) \
      ((CFStringRef)__builtin___NSStringMakeConstantString("" x ""))
# else */
  // If nothing else works, fall back to the really slow path.  The 'pure'
  // attribute tells the compiler that this function will always return the
  // same result with the same input.  If it has any skill, then constant
  // propagation passes will magically make sure that this function is called
  // as few times as possible.
    CFStringRef __CFStringMakeConstantString (const char *str)
      __attribute__ ((pure));
#   define CFSTR(x) __CFStringMakeConstantString("" x "")
/*# endif
#endif */

//
// Creating a CFString
//
/** @name Creating a CFString
    @{
 */
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
CFStringCreateWithSubstring (CFAllocatorRef alloc, CFStringRef str,
  CFRange range);

#if MAC_OS_X_VERSION_10_4 <= MAC_OS_X_VERSION_MAX_ALLOWED
CFStringRef
CFStringCreateWithFileSystemRepresentation (CFAllocatorRef alloc,
  const char *buffer);
#endif

#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
CFStringRef
CFStringCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalReprentation,
  CFAllocatorRef contentsDeallocator);
#endif
/** @}
 */

//
// Searching Strings
//
/** @name Searching CFStrings
    @{
 */
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

#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
Boolean
CFStringFindCharacterFromSet (CFStringRef theString, CFCharacterSetRef theSet,
  CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result);
#endif

#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
void
CFStringGetParagraphBounds (CFStringRef string, CFRange range,
  CFIndex *parBeginIndex, CFIndex *parEndIndex, CFIndex *contentsEndIndex);
#endif
/** @}
 */

//
// Comparing Strings
//
/** @name Comparing String
    @{
 */
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

#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
CFComparisonResult
CFStringCompareWithOptionsAndLocale (CFStringRef theString1,
  CFStringRef theString2, CFRange rangeToCOmpare,
  CFStringCompareFlags compareOptions, CFLocaleRef locale);
#endif
/** @}
 */

//
// Accessing Characters
//
/** @name Accessing Characters
    @{
 */
CFDataRef
CFStringCreateExternalRepresentation (CFAllocatorRef alloc,
  CFStringRef theString, CFStringEncoding encoding, UInt8 lossByte);

CFIndex
CFStringGetBytes (CFStringRef theString, CFRange range,
  CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation,
  UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen);

UniChar
CFStringGetCharacterAtIndex (CFStringRef theString, CFIndex idx);

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

CFRange
CFStringGetRangeOfComposedCharactersAtIndex (CFStringRef theString,
  CFIndex theIndex);

#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
UTF32Char
CFStringGetLongCharacterForSurrogatePair (UniChar surrogateHigh,
  UniChar surrogateLow);

Boolean
CFStringGetSurrogatePairForLongCharacter (UTF32Char character,
  UniChar *surrogates);

Boolean
CFStringIsSurrogateHighCharacter (UniChar character);

Boolean
CFStringIsSurrogateLowCharacter (UniChar character);
#endif
/** @}
 */

//
// Working With Encodings
//
/** @name Working with Encodings
    @{
 */
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

#if MAC_OS_X_VERSION_10_4 <= MAC_OS_X_VERSION_MAX_ALLOWED
Boolean
CFStringGetFileSystemRepresentation (CFStringRef string, char *buffer,
  CFIndex maxBufLen);

CFIndex
CFStringGetMaximumSizeOfFileSystemRepresentation (CFStringRef string);
#endif
/** @}
 */

//
// Getting Numeric Values
//
/** @name Getting Numeric Values
    @{
 */
double
CFStringGetDoubleValue (CFStringRef str);

SInt32
CFStringGetIntValue (CFStringRef str);
/** @}
 */

//
// Getting String Properties
//
/** @name Getting String Properties
    @{
 */
void
CFShow (CFTypeRef obj);

void
CFShowStr (CFStringRef str);

CFTypeID
CFStringGetTypeID (void);
/** @}
 */



/** @name Pascal Strings
    @{
 */
CFStringRef
CFStringCreateWithPascalString (CFAllocatorRef alloc, ConstStr255Param pStr,
  CFStringEncoding encoding);

CFStringRef
CFStringCreateWithPascalStringNoCopy (CFAllocatorRef alloc,
  ConstStr255Param pStr, CFStringEncoding encoding,
  CFAllocatorRef contentsDeallocate);

Boolean
CFStringGetPascalString (CFStringRef theString, StringPtr buffer,
  CFIndex bufferSize, CFStringEncoding encoding);

ConstStringPtr
CFStringGetPascalStringPtr (CFStringRef theString, CFStringEncoding encoding);
/** @}
 */
/** @}
 */



/** @defgroup CFMutableString
 *  @{
 */
#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum CFStringNormalizationForm
{
  kCFStringNormalizationFormD = 0,
  kCFStringNormalizationFormKD = 1,
  kCFStringNormalizationFormC = 2,
  kCFStringNormalizationFormKC = 3
};
typedef enum CFStringNormalizationForm CFStringNormalizationForm;
#endif

#if MAC_OS_X_VERSION_10_4 <= MAC_OS_X_VERSION_MAX_ALLOWED
CF_EXPORT const CFStringRef kCFStringTransformStripCombiningMarks;
CF_EXPORT const CFStringRef kCFStringTransformToLatin;
CF_EXPORT const CFStringRef kCFStringTransformFullwidthHalfwidth;
CF_EXPORT const CFStringRef kCFStringTransformLatinKatakana;
CF_EXPORT const CFStringRef kCFStringTransformLatinHiragana;
CF_EXPORT const CFStringRef kCFStringTransformHiraganaKatakana;
CF_EXPORT const CFStringRef kCFStringTransformMandarinLatin;
CF_EXPORT const CFStringRef kCFStringTransformLatinHangul;
CF_EXPORT const CFStringRef kCFStringTransformLatinArabic;
CF_EXPORT const CFStringRef kCFStringTransformLatinHebrew;
CF_EXPORT const CFStringRef kCFStringTransformLatinThai;
CF_EXPORT const CFStringRef kCFStringTransformLatinCyrillic;
CF_EXPORT const CFStringRef kCFStringTransformLatinGreek;
CF_EXPORT const CFStringRef kCFStringTransformToXMLHex;
CF_EXPORT const CFStringRef kCFStringTransformToUnicodeName;
#endif
#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
CF_EXPORT const CFStringRef kCFStringTransformStripDiacritics;
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

void
CFStringInsert (CFMutableStringRef str, CFIndex idx, CFStringRef insertedStr);

void
CFStringLowercase (CFMutableStringRef theString, CFLocaleRef locale);

void
CFStringPad (CFMutableStringRef theString, CFStringRef padString,
  CFIndex length, CFIndex indexIntoPad);

void
CFStringReplace (CFMutableStringRef theString, CFRange range,
  CFStringRef replacement);

void
CFStringReplaceAll (CFMutableStringRef theString, CFStringRef replacement);

void
CFStringSetExternalCharactersNoCopy (CFMutableStringRef theString,
  UniChar *chars, CFIndex length, CFIndex capacity);

void
CFStringTrim (CFMutableStringRef theString, CFStringRef trimString);

void
CFStringTrimWhitespace (CFMutableStringRef theString);

void
CFStringUppercase (CFMutableStringRef theString, CFLocaleRef locale);

#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
CFIndex
CFStringFindAndReplace (CFMutableStringRef theString,
  CFStringRef stringToFind, CFStringRef replacementString,
  CFRange rangeToSearch, CFOptionFlags compareOptions);

void
CFStringNormalize (CFMutableStringRef theString,
  CFStringNormalizationForm theForm);
#endif

#if MAC_OS_X_VERSION_10_4 <= MAC_OS_X_VERSION_MAX_ALLOWED
Boolean
CFStringTransform (CFMutableStringRef string, CFRange *range,
  CFStringRef transform, Boolean reverse);
#endif

#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
void
CFStringFold (CFMutableStringRef theString, CFOptionFlags theFlags,
  CFLocaleRef theLocale);
#endif
/** @}
 */



/** @defgroup CFStringInlineBuffer
 *  @{
 */
#define __kCFStringInlineBufferLength 64
struct CFStringInlineBuffer
{
  UniChar buffer[__kCFStringInlineBufferLength];
  CFStringRef theString;
  const UniChar *directBuffer;
  CFRange rangeToBuffer;
  CFIndex bufferedRangeStart;
  CFIndex bufferedRangeEnd;
};
typedef struct CFStringInlineBuffer CFStringInlineBuffer;

CF_INLINE void
CFStringInitInlineBuffer (CFStringRef str, CFStringInlineBuffer *buf,
  CFRange range)
{
  buf->theString = str;
  buf->rangeToBuffer = range;
  buf->directBuffer = CFStringGetCharactersPtr (str);
  buf->bufferedRangeStart = 0;
  buf->bufferedRangeEnd = 0;
}

CF_INLINE UniChar
CFStringGetCharacterFromInlineBuffer (CFStringInlineBuffer *buf, CFIndex idx)
{
  if (buf->directBuffer)
    {
      if (idx < 0 || idx >= buf->rangeToBuffer.length)
        return 0;
      return buf->directBuffer[idx + buf->rangeToBuffer.location];
    }
  else if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart)
    {
      CFRange range;
      
      if (idx < 0 || idx >= buf->rangeToBuffer.length)
        return 0;
      
      buf->bufferedRangeStart = idx - 4;
      if (buf->bufferedRangeStart < 0)
        buf->bufferedRangeStart = 0;
      buf->bufferedRangeEnd =
        buf->bufferedRangeStart + __kCFStringInlineBufferLength;
      if (buf->bufferedRangeEnd > buf->rangeToBuffer.length)
        buf->bufferedRangeEnd = buf->rangeToBuffer.length;
      
      range = CFRangeMake (buf->rangeToBuffer.location + buf->bufferedRangeStart,
        buf->bufferedRangeEnd - buf->bufferedRangeStart);
      
      CFStringGetCharacters (buf->theString, range, buf->buffer);
    }
  
  return buf->buffer[(idx - buf->bufferedRangeStart)];
}
/** @}
 */

CF_EXTERN_C_END

#endif /* __COREFOUNDATION_CFSTRING_H__ */
