#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

/* Build an error whose user info carries the three localized strings that
   CFErrorCopy{Description,FailureReason,RecoverySuggestion} read back. */
static CFErrorRef
makeError (CFStringRef domain, CFIndex code)
{
  const void *keys[3];
  const void *values[3];
  CFDictionaryRef userInfo;
  CFErrorRef err;

  keys[0] = kCFErrorLocalizedDescriptionKey;
  keys[1] = kCFErrorLocalizedFailureReasonKey;
  keys[2] = kCFErrorLocalizedRecoverySuggestionKey;
  values[0] = CFSTR("description");
  values[1] = CFSTR("failure reason");
  values[2] = CFSTR("recovery suggestion");

  userInfo = CFDictionaryCreate (NULL, keys, values, 3,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  err = CFErrorCreate (NULL, domain, code, userInfo);
  CFRelease (userInfo);

  return err;
}

int main (void)
{
  CFErrorRef err;
  CFErrorRef equal;
  CFErrorRef otherCode;
  CFErrorRef otherDomain;
  CFStringRef str;

  err = makeError (kCFErrorDomainPOSIX, 1);

  str = CFErrorCopyDescription (err);
  PASS_CFEQ(str, CFSTR("description"),
    "CFErrorCopyDescription reads the localized description.");
  CFRelease (str);

  str = CFErrorCopyFailureReason (err);
  PASS_CFEQ(str, CFSTR("failure reason"),
    "CFErrorCopyFailureReason reads the localized failure reason.");
  CFRelease (str);

  str = CFErrorCopyRecoverySuggestion (err);
  PASS_CFEQ(str, CFSTR("recovery suggestion"),
    "CFErrorCopyRecoverySuggestion reads the localized recovery suggestion.");
  CFRelease (str);

  /* Two errors with the same domain, code and user info are equal. */
  equal = makeError (kCFErrorDomainPOSIX, 1);
  PASS_CFEQ(err, equal, "Errors with matching fields are equal.");
  PASS_CF(CFHash (err) == CFHash (equal),
    "Equal errors have the same hash.");
  CFRelease (equal);

  /* Differing code breaks equality. */
  otherCode = makeError (kCFErrorDomainPOSIX, 2);
  PASS_CFNEQ(err, otherCode, "Errors with different codes are not equal.");
  CFRelease (otherCode);

  /* Differing domain breaks equality. */
  otherDomain = makeError (kCFErrorDomainCocoa, 1);
  PASS_CFNEQ(err, otherDomain, "Errors with different domains are not equal.");
  CFRelease (otherDomain);

  CFRelease (err);

  return 0;
}
