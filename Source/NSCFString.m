/* NSCFString.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: May, 2011
   
   This file is part of CoreBase.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSObject.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSCharacterSet.h>

#include "NSCFType.h"
#include "CoreFoundation/CFString.h"
#include "GSPrivate.h"

/* NSCFString inherits from NSMutableString and doesn't have any ivars
   because it is only an ObjC wrapper around CFString. */
@interface NSCFString : NSMutableString
NSCFTYPE_VARS
@end

@interface NSString (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@interface NSMutableString (CoreBaseAdditions)
- (void) _cfTrimWhitespace;
@end

@implementation NSCFString

/* Class variables */
static NSStringEncoding *nsencodings = NULL;

+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id) initWithBytes: (const void*) bytes
              length: (NSUInteger) length
            encoding: (NSStringEncoding) encoding
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  CFStringEncoding enc = CFStringConvertNSStringEncodingToEncoding (encoding);
  
  RELEASE(self);
  
  self = (NSCFString*)CFStringCreateWithBytes (alloc, bytes, length, enc,
    false);
  return self;
}

- (id) initWithBytesNoCopy: (void*) bytes
                    length: (NSUInteger) length
                  encoding: (NSStringEncoding) encoding 
              freeWhenDone: (BOOL) flag
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  CFAllocatorRef deallocator = flag ? kCFAllocatorDefault : kCFAllocatorNull;
  CFStringEncoding enc = CFStringConvertNSStringEncodingToEncoding (encoding);
  
  RELEASE(self);
  
  self = (NSCFString*)CFStringCreateWithBytesNoCopy (alloc, bytes, length, enc,
    false, deallocator);
  return self;
}

- (id) initWithCharacters: (const unichar*) chars
                   length: (NSUInteger) length
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  
  RELEASE(self);
  
  self = (NSCFString*)CFStringCreateWithCharacters (alloc, chars, length);
  return self;
}

- (id) initWithCharactersNoCopy: (unichar*) chars
                         length: (NSUInteger) length
                   freeWhenDone: (BOOL) flag
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  CFAllocatorRef deallocator = flag ? kCFAllocatorDefault : kCFAllocatorNull;
  
  RELEASE(self);
  
  self = (NSCFString*)CFStringCreateWithCharactersNoCopy (alloc, chars, length,
    deallocator);
  return self;
}

- (id) initWithString: (NSString*) string
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  
  RELEASE(self);
  
  self = (NSCFString*)CFStringCreateWithSubstring (alloc, string,
    CFRangeMake(0, CFStringGetLength(string)));
  return self;
}

/*
- (id) initWithFormat: (NSString*) format
            arguments: (va_list) argList
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  
  RELEASE(self);
  
  self = (NSCFString*)CFStringCreateWithFormatAndArguments (alloc, NULL,
    format, argList);
  return self;
}

- (id) initWithFormat: (NSString*) format
               locale: (id) locale
            arguments: (va_list) argList
{
  if ([locale isKindOfClass: [NSLocale class]])
    return nil; // FIXME
    
  RELEASE(self);
    
  return (NSCFString*)
    CFStringCreateWithFormatAndArguments (CFAllocatorGetDefault(),
    (CFDictionaryRef)locale, format, argList);
}
*/

- (id) initWithData: (NSData*)data
           encoding: (NSStringEncoding)encoding
{
  CFStringEncoding enc = CFStringConvertNSStringEncodingToEncoding (encoding);
  CFAllocatorRef alloc = kCFAllocatorDefault;
  
  RELEASE(self);
  
  self = (NSCFString*)
    CFStringCreateFromExternalRepresentation (alloc, (CFDataRef)data, enc);
  return self;
}

- (NSString*)stringByReplacingOccurrencesOfString: (NSString*)replace
                                       withString: (NSString*)by
                                          options: (NSStringCompareOptions)opts
                                            range: (NSRange)searchRange
{
  // GNUstep's NSString uses GSMutableStringClass here
  // so we cannot use the base implementation (it would break the bridging)
  return nil; // FIXME
}

- (NSString*) stringByReplacingCharactersInRange: (NSRange)aRange 
                                      withString: (NSString*)by;
{
  // GNUstep's NSString uses GSMutableStringClass here
  // so we cannot use the base implementation (it would break the bridging)
  return nil; // FIXME
}

- (NSUInteger) length
{
  return (NSUInteger)CFStringGetLength (self);
}

- (unichar) characterAtIndex: (NSUInteger) index
{
  return (unichar)CFStringGetCharacterAtIndex (self, index);
}

- (void) getCharacters: (unichar*) buffer
                 range: (NSRange) aRange
{
  CFRange cfRange = CFRangeMake (aRange.location, aRange.length);
  CFStringGetCharacters (self, cfRange, buffer);
}

- (NSArray*) componentsSeparatedByString: (NSString*) separator
{
  return (NSArray*)
    CFStringCreateArrayBySeparatingStrings (CFAllocatorGetDefault(),
    self, separator);
}

- (NSRange) rangeOfCharacterFromSet: (NSCharacterSet*) aSet
                            options: (NSUInteger) mask
                              range: (NSRange) aRange
{
  CFRange cfRange = CFRangeMake (aRange.location, aRange.length);
  CFRange ret;
  
  if (!CFStringFindCharacterFromSet (self,
      (CFCharacterSetRef)aSet, cfRange, (CFStringCompareFlags)mask,
      &ret))
    ret = CFRangeMake (kCFNotFound, 0);
  
  return NSMakeRange (ret.location, ret.length);
}

- (NSRange) rangeOfString: (NSString *) aString
                  options: (NSStringCompareOptions) mask
                    range: (NSRange) searchRange
                   locale: (NSLocale *) locale
{
  CFRange cfRange = CFRangeMake (searchRange.location, searchRange.length);
  CFRange ret;
  
  if (!CFStringFindWithOptionsAndLocale (self,
      aString, cfRange, (CFStringCompareFlags)mask,
      (CFLocaleRef)locale, &ret))
    ret = CFRangeMake (kCFNotFound, 0);
  
  return NSMakeRange (ret.location, ret.length);
}

/*
- (NSArray *) componentsSeparatedByCharactersInSet: (NSCharacterSet *)separator
{
  return nil; // FIXME
}

- (NSRange) rangeOfComposedCharacterSequencesForRange: (NSRange)range
{
  return NSMakeRange (NSNotFound, 0); // FIXME, even NSString doesn't implement this
}

- (NSDictionary*) propertyListFromStringsFileFormat
{
  // FIXME ???
  return nil;
}
*/

- (NSComparisonResult) compare: (NSString*) string 
                       options: (NSUInteger) mask 
                         range: (NSRange) compareRange 
                        locale: (id) locale
{
  CFRange cfRange = CFRangeMake (compareRange.location, compareRange.length);
  if (nil != locale
      && ![locale isKindOfClass: [NSLocale class]])
    {
      locale = [NSLocale currentLocale];
    }
  return (NSComparisonResult)CFStringCompareWithOptionsAndLocale (self,
     string, cfRange, (CFStringCompareFlags)mask,
    (CFLocaleRef)locale);
}

- (BOOL) hasPrefix: (NSString*) aString
{
  return CFStringHasPrefix (self, aString);
}

- (BOOL) hasSuffix: (NSString*) aString
{
  return CFStringHasSuffix (self, aString);
}

- (NSString*) capitalizedString
{
  CFMutableStringRef copy;
  copy = CFStringCreateMutableCopy (NULL, 0, self);
  CFStringCapitalize (copy, NULL);
  return (NSString*) AUTORELEASE(copy);
}

- (NSString*) lowercaseString
{
  CFMutableStringRef copy;
  copy = CFStringCreateMutableCopy (NULL, 0, self);
  CFStringLowercase (copy, NULL);
  return (NSString*)AUTORELEASE(copy);
}

- (NSString*) uppercaseString
{
  CFMutableStringRef copy;
  copy = CFStringCreateMutableCopy (NULL, 0, self);
  CFStringUppercase (copy, NULL);
  return (NSString*)AUTORELEASE(copy);
}

- (BOOL) getCString: (char*) buffer
          maxLength: (NSUInteger) maxLength
           encoding: (NSStringEncoding) encoding
{
  CFStringEncoding enc = CFStringConvertEncodingToNSStringEncoding (encoding);
  return (BOOL)CFStringGetCString (self, buffer, maxLength, enc);
}

- (NSUInteger) lengthOfBytesUsingEncoding: (NSStringEncoding) encoding
{
  return [self lengthOfBytesUsingEncoding: encoding];
}

- (NSUInteger) maximumLengthOfBytesUsingEncoding: (NSStringEncoding) encoding
{
  CFStringEncoding enc = CFStringConvertNSStringEncodingToEncoding (encoding);
  return CFStringGetMaximumSizeForEncoding ([self length], enc);
}


- (NSData*) dataUsingEncoding: (NSStringEncoding) encoding
         allowLossyConversion: (BOOL) flag
{
  CFStringEncoding enc = CFStringConvertEncodingToNSStringEncoding (encoding);
  return (NSData*)CFStringCreateExternalRepresentation (NULL,
    self, enc, flag ? '?' : 0);
}


- (NSStringEncoding) fastestEncoding
{
  CFStringEncoding enc = CFStringGetFastestEncoding (self);
  return CFStringConvertEncodingToNSStringEncoding (enc);
}

- (NSStringEncoding) smallestEncoding
{
  CFStringEncoding enc = CFStringGetSmallestEncoding (self);
  return CFStringConvertEncodingToNSStringEncoding (enc);
}

- (BOOL) getFileSystemRepresentation: (char*) buffer
                           maxLength: (NSUInteger) size
{
  return (BOOL)CFStringGetFileSystemRepresentation (self, buffer, size);
}

- (NSString*) substringWithRange: (NSRange) aRange
{
  CFRange cfRange = CFRangeMake (aRange.location, aRange.length);
  return (NSString*)CFStringCreateWithSubstring (NULL, self, cfRange);
}

+ (NSStringEncoding*) availableStringEncodings
{
  if (!nsencodings)
  {
    int count = 0, i;
    
    const CFStringEncoding* encodings;
    NSStringEncoding* converted;
    
    encodings = CFStringGetListOfAvailableEncodings();

    for (i = 0; encodings[i] != 0; i++)
      count++;
    
    converted = (NSStringEncoding*)
      CFAllocatorAllocate (kCFAllocatorSystemDefault,
                           (count+1) * sizeof(NSStringEncoding), 0);
    
    for (i = 0; i < count; i++)
      converted[i] = CFStringConvertEncodingToNSStringEncoding(encodings[i]);
    
    if (GSAtomicCompareAndSwapPointer (&nsencodings, NULL, converted) != NULL)
      CFAllocatorDeallocate (kCFAllocatorSystemDefault, converted);
  }
  
  return nsencodings;
}

- (void) getLineStart: (NSUInteger *) startIndex
                  end: (NSUInteger *) lineEndIndex
          contentsEnd: (NSUInteger *) contentsEndIndex
             forRange: (NSRange) aRange
{
  CFRange cfRange = CFRangeMake (aRange.location, aRange.length);
  CFStringGetLineBounds (self, cfRange, (CFIndex*)startIndex,
    (CFIndex*)lineEndIndex, (CFIndex*)contentsEndIndex);
}

- (NSString*) stringByPaddingToLength: (NSUInteger)newLength
                           withString: (NSString*)padString
                      startingAtIndex: (NSUInteger)padIndex
{
  CFMutableStringRef copy;
  copy = CFStringCreateMutableCopy(NULL, 0, self);
  CFStringPad(copy, padString, newLength, padIndex);
  return copy;
}


- (void) getParagraphStart: (NSUInteger *) startPtr
                       end: (NSUInteger *) parEndPtr
               contentsEnd: (NSUInteger *) contentsEndPtr
                  forRange: (NSRange)range
{
  CFRange cfRange = CFRangeMake (range.location, range.length);
  CFStringGetParagraphBounds (self, cfRange, (CFIndex*)startPtr,
    (CFIndex*)parEndPtr, (CFIndex*)contentsEndPtr);
}

- (NSRange) rangeOfComposedCharacterSequencesForRange: (NSRange)range
{
  return NSMakeRange (NSNotFound, 0); // FIXME, even NSString doesn't implement this
}



//
// NSMutableString methods
//
- (id) initWithCapacity: (NSUInteger)capacity
{
  CFAllocatorRef alloc = kCFAllocatorDefault;
  
  RELEASE(self);
    
  self = (NSCFString*)CFStringCreateMutable (alloc, capacity);
  return self;
}

- (void) appendFormat: (NSString*) format, ...
{
  va_list args;
  
  if (format == nil)
    [NSException raise: NSInvalidArgumentException format: @"format is nil."];

  va_start(args, format);
  CFStringAppendFormatAndArguments (self, NULL,
    format, args);
  va_end (args);
}

- (void) appendString: (NSString*) aString
{
  CFStringAppend (self, aString);
}

- (void) deleteCharactersInRange: (NSRange) range
{
  CFRange cfRange = CFRangeMake (range.location, range.length);
  CFStringDelete (self, cfRange);
}

- (void) insertString: (NSString*) aString atIndex: (NSUInteger) loc
{
  CFStringInsert (self, loc, aString);
}

- (void) replaceCharactersInRange: (NSRange) range 
                       withString: (NSString*) aString
{
  CFRange cfRange = CFRangeMake (range.location, range.length);
  CFStringReplace (self, cfRange, NULL);
}

- (NSUInteger) replaceOccurrencesOfString: (NSString*) replace
                               withString: (NSString*) by
                                  options: (NSUInteger) opts
                                    range: (NSRange) searchRange
{
  CFRange cfRange = CFRangeMake (searchRange.location, searchRange.length);
  if (replace == nil)
    [NSException raise: NSInvalidArgumentException
                format: @"Target string is nil."];
  if (by == nil)
    [NSException raise: NSInvalidArgumentException
                format: @"Replacement is nil."];
  /* FIXME: raise exception for out of range */
  
  return CFStringFindAndReplace (self, replace,
    by, cfRange, (CFOptionFlags)opts);
}

- (void) setString: (NSString*) aString
{
  CFStringReplaceAll (self, aString);
}

@end

@implementation NSString (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFStringGetTypeID();
}
@end

@implementation NSMutableString (CoreBaseAdditions)
- (void) _cfTrimWhitespace
{
  NSString* trimmed;

  trimmed = [self stringByTrimmingCharactersInSet:
                  [NSCharacterSet whitespaceAndNewlineCharacterSet]];

  [self setString: trimmed];
  [trimmed release];
}

@end


