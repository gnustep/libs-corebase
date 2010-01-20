#include <Foundation/NSString.h> // For NSConstantString
#include <Foundation/NSError.h>

#include "CoreFoundation/CFError.h"

const CFStringRef kCFErrorDomainPOSIX = 
  (CFStringRef)@"NSPOSIXErrorDomain";
const CFStringRef kCFErrorDomainOSStatus = 
  (CFStringRef)@"NSOSStatusErrorDomain";
const CFStringRef kCFErrorDomainMach = 
  (CFStringRef)@"NSMACHErrorDomain";
const CFStringRef kCFErrorDomainCocoa = 
  (CFStringRef)@"NSCocoaErrorDomain";

const CFStringRef kCFErrorLocalizedDescriptionKey = 
  (CFStringRef)@"NSLocalizedDescriptionKey";
const CFStringRef kCFErrorLocalizedFailureReasonKey = 
  (CFStringRef)@"NSLocalizedFailureReasonErrorKey";
const CFStringRef kCFErrorLocalizedRecoverySuggestionKey = 
  (CFStringRef)@"NSLocalizedRecoverySuggestionErrorKey";
const CFStringRef kCFErrorDescriptionKey = 
  (CFStringRef)@"kCFErrorDescriptionKey"; // FIXME
const CFStringRef kCFErrorUnderlyingErrorKey = 
  (CFStringRef)@"NSUnderlyingErrorKey";



CFStringRef CFErrorCopyDescription (CFErrorRef err)
{
  return (CFStringRef)[(NSError *)err localizedDescription];
}

CFStringRef CFErrorCopyFailureReason (CFErrorRef err)
{
  return (CFStringRef)[(NSError *)err localizedFailureReason];
}

CFStringRef CFErrorCopyRecoverySuggestion (CFErrorRef err)
{
  return (CFStringRef)[(NSError *)err localizedRecoverySuggestion];
}

CFDictionaryRef CFErrorCopyUserInfo (CFErrorRef err)
{
  return (CFDictionaryRef)[(NSError *)err userInfo];
}

CFErrorRef CFErrorCreate (CFAllocatorRef allocator, CFStringRef domain,
  CFIndex code, CFDictionaryRef userInfo)
{
  return (CFErrorRef)[[NSError allocWithZone: (NSZone *)allocator]
                       initWithDomain: (NSString *)domain
                                 code: code
                             userInfo: (NSDictionary *)userInfo];
}

CFErrorRef CFErrorCreateWithUserInfoKeysAndValues (CFAllocatorRef allocator,
  CFStringRef domain, CFIndex code, const void *const *userInfoKeys,
  const void *const *userInfoValues, CFIndex numUserInfoValues)
{
  CFDictionaryRef userInfo = CFDictionaryCreate (allocator, 
    (const void **)userInfoKeys, (const void **)userInfoValues, 
    numUserInfoValues, NULL, NULL);
  CFErrorRef ret = CFErrorCreate (allocator, domain, code, userInfo);
  
  CFRelease (userInfo);
  return ret;
}

CFIndex CFErrorGetCode (CFErrorRef err)
{
  return [(NSError *)err code];
}

CFStringRef CFErrorGetDomain (CFErrorRef err)
{
  return (CFStringRef)[(NSError *)err domain];
}

CFTypeID CFErrorGetTypeID (void)
{
  return (CFTypeID)[NSError class];
}

