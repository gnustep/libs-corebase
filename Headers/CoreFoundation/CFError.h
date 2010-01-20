#ifndef __COREFOUNDATION_CFERROR_H__
#define __COREFOUNDATION_CFERROR_H__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <GNUstepBase/GSVersionMacros.h>

// CFError implementation didn't show up until OS X 10.5
#if OS_API_VERSION(GS_API_MACOSX_10_5, GS_API_LATEST)

typedef struct __CFError * CFErrorRef;

const CFStringRef kCFErrorDomainPOSIX;
const CFStringRef kCFErrorDomainOSStatus;
const CFStringRef kCFErrorDomainMach;
const CFStringRef kCFErrorDomainCocoa;

const CFStringRef kCFErrorLocalizedDescriptionKey;
const CFStringRef kCFErrorLocalizedFailureReasonKey;
const CFStringRef kCFErrorLocalizedRecoverySuggestionKey;
const CFStringRef kCFErrorDescriptionKey;
const CFStringRef kCFErrorUnderlyingErrorKey;



//
// Creating a CFError
//
CFErrorRef
CFErrorCreate (CFAllocatorRef allocator, CFStringRef domain, CFIndex code,
  CFDictionaryRef userInfo);

CFErrorRef
CFErrorCreateWithUserInfoKeysAndValues (CFAllocatorRef allocator,
  CFStringRef domain, CFIndex code, const void *const *userInfoKeys,
  const void *const *userInfoValues, CFIndex numUserInfoValues);

//
// Getting Information About an Error
//
CFStringRef
CFErrorGetDomain (CFErrorRef err);

CFIndex
CFErrorGetCode (CFErrorRef err);

CFDictionaryRef
CFErrorCopyUserInfo (CFErrorRef err);

CFStringRef
CFErrorCopyDescription (CFErrorRef err);

CFStringRef
CFErrorCopyFailureReason (CFErrorRef err);

CFStringRef
CFErrorCopyRecoverySuggestion (CFErrorRef err);

//
// Getting the CFError Type ID
//
CFTypeID
CFErrorGetTypeID (void);

#endif

#endif /* __COREFOUNDATION_CFERROR_H__ */
