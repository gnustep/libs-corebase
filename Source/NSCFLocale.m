/* NSCFLocale.m
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Lubos Dolezel
   Date: March, 2013
   
   This file is part of GNUstep CoreBase Library.
   
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

#import <Foundation/NSLocale.h>

#include "NSCFType.h"
#include "CoreFoundation/CFLocale.h"

@interface NSCFLocale : NSLocale
NSCFTYPE_VARS
@end

@interface NSLocale (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

static CFStringRef NSLocaleKeyToCFLocaleKey(id key)
{
  CFStringRef cfKey = NULL;

#define CASE(keyName) if([key isEqual: NS##keyName]) cfKey = kCF##keyName; else
  CASE(LocaleIdentifier)
  CASE(LocaleLanguageCode)
  CASE(LocaleCountryCode)
  CASE(LocaleScriptCode)
  CASE(LocaleVariantCode)
  CASE(LocaleExemplarCharacterSet)
  CASE(LocaleCalendar)
  CASE(LocaleCollationIdentifier)
  CASE(LocaleUsesMetricSystem)
  CASE(LocaleMeasurementSystem)
  CASE(LocaleDecimalSeparator)
  CASE(LocaleGroupingSeparator)
  CASE(LocaleCurrencySymbol)
  CASE(LocaleCurrencyCode)
  CASE(LocaleCollatorIdentifier)
  CASE(LocaleQuotationBeginDelimiterKey)
  CASE(LocaleQuotationEndDelimiterKey)
  CASE(LocaleAlternateQuotationBeginDelimiterKey)
  CASE(LocaleAlternateQuotationEndDelimiterKey);
  
#undef CASE
  return cfKey;
}

@implementation NSCFLocale
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id) initWithLocaleIdentifier:(NSString*)string
{
  RELEASE(self);

  self = (NSCFLocale*) CFLocaleCreate(kCFAllocatorDefault,
    (CFStringRef) string);
  return self;
}

- (NSString*) localeIdentifier
{
  return (NSString*) CFLocaleGetIdentifier((CFLocaleRef) self);
}

- (id) objectForKey:(id)key
{
  CFStringRef cfKey = NSLocaleKeyToCFLocaleKey(key);

  return (id) CFLocaleGetValue((CFLocaleRef) self, cfKey);
}

- (NSString*) displayNameForKey:(id)key value:(id)value
{
  CFStringRef cfKey = NSLocaleKeyToCFLocaleKey(key);
  return (NSString*)
    CFLocaleCopyDisplayNameForPropertyValue((CFLocaleRef) self, cfKey,
                                            (CFStringRef) value);
}

@end

@implementation NSLocale (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFLocaleGetTypeID();
}
@end

