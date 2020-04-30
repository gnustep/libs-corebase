/* NSCFNumber.m
   
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   Written by: Frederik Seiffert
   Date: March, 2020
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSObject.h>
#import <Foundation/NSValue.h>

#include "NSCFType.h"
#include "CoreFoundation/CFNumber.h"

@interface NSCFNumber : NSNumber
NSCFTYPE_VARS
@end

@interface NSNumber (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSCFNumber
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (const char *) objCType
{
  CFNumberType type = CFNumberGetType((CFNumberRef)self);
  switch (type)
    {
      case kCFNumberSInt8Type:
        return @encode(SInt8);
      case kCFNumberSInt16Type:
        return @encode(SInt16);
      case kCFNumberSInt32Type:
        return @encode(SInt32);
      case kCFNumberSInt64Type:
        return @encode(SInt64);
      case kCFNumberFloat32Type:
        return @encode(Float32);
      case kCFNumberFloat64Type:
        return @encode(Float64);
      case kCFNumberCharType:
        return @encode(char);
      case kCFNumberShortType:
        return @encode(short);
      case kCFNumberIntType:
        return @encode(int);
      case kCFNumberLongType:
        return @encode(long);
      case kCFNumberLongLongType:
        return @encode(long long);
      case kCFNumberFloatType:
        return @encode(float);
      case kCFNumberDoubleType:
        return @encode(double);
      case kCFNumberCFIndexType:
        return @encode(CFIndex);
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
      case kCFNumberNSIntegerType:
        return @encode(NSInteger);
      case kCFNumberCGFloatType:
        return @encode(CGFloat);
#endif
    }
    return NULL;
}

- (NSString*) descriptionWithLocale: (id)aLocale
{
  return AUTORELEASE((id) CFCopyDescription(self));
}

- (BOOL) boolValue
{
  BOOL value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberCharType, &value);
  return value;
}

- (signed char) charValue
{
  signed char value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberCharType, &value);
  return value;
}

- (double) doubleValue
{
  double value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberDoubleType, &value);
  return value;
}

- (float) floatValue
{
  float value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberFloatType, &value);
  return value;
}

- (signed int) intValue
{
  signed int value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberIntType, &value);
  return value;
}

- (signed long) longValue
{
  signed long value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberLongType, &value);
  return value;
}

- (signed long long) longLongValue
{
  signed long long value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberLongLongType, &value);
  return value;
}

- (signed short) shortValue
{
  signed short value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberShortType, &value);
  return value;
}

- (unsigned char) unsignedCharValue
{
  unsigned char value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberCharType, &value);
  return value;
}

- (unsigned int) unsignedIntValue
{
  unsigned int value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberIntType, &value);
  return value;
}

- (unsigned long) unsignedLongValue
{
  unsigned long value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberLongType, &value);
  return value;
}

- (unsigned long long) unsignedLongLongValue
{
  unsigned long long value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberLongLongType, &value);
  return value;
}

- (unsigned short) unsignedShortValue
{
  unsigned short value;
  CFNumberGetValue((CFNumberRef)self, kCFNumberShortType, &value);
  return value;
}

@end

@implementation NSNumber (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFNumberGetTypeID();
}
@end
