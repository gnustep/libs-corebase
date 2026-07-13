#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

int main (void)
{
  CFErrorRef err;
  CFDictionaryRef userInfo;
  CFDictionaryRef copy;
  const void *keys[1];
  const void *values[1];

  keys[0] = kCFErrorLocalizedDescriptionKey;
  values[0] = CFSTR("something went wrong");

  userInfo = CFDictionaryCreate (NULL, keys, values, 1,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  err = CFErrorCreate (NULL, kCFErrorDomainPOSIX, 42, userInfo);
  PASS_CF(err != NULL, "CFError created.");

  PASS_CF(CFErrorGetTypeID () != 0, "CFError type ID is not zero.");
  PASS_CF(CFGetTypeID (err) == CFErrorGetTypeID (),
    "CFError has the CFError type ID.");

  PASS_CFEQ(CFErrorGetDomain (err), kCFErrorDomainPOSIX,
    "CFErrorGetDomain returns the domain.");
  PASS_CF(CFErrorGetCode (err) == 42, "CFErrorGetCode returns the code.");

  copy = CFErrorCopyUserInfo (err);
  PASS_CF(copy != NULL, "CFErrorCopyUserInfo returns a dictionary.");
  PASS_CFEQ(copy, userInfo, "Copied user info is equal to the original.");
  CFRelease (copy);

  CFRelease (err);
  CFRelease (userInfo);

  /* A NULL domain is not allowed. */
  err = CFErrorCreate (NULL, NULL, 0, NULL);
  PASS_CF(err == NULL, "CFErrorCreate with a NULL domain returns NULL.");

  /* With no user info a valid, empty dictionary is substituted. */
  err = CFErrorCreate (NULL, kCFErrorDomainCocoa, 7, NULL);
  PASS_CF(err != NULL, "CFError created without user info.");
  copy = CFErrorCopyUserInfo (err);
  PASS_CF(copy != NULL,
    "CFErrorCopyUserInfo returns a dictionary even without user info.");
  PASS_CF(CFDictionaryGetCount (copy) == 0,
    "Substituted user info dictionary is empty.");
  CFRelease (copy);
  CFRelease (err);

  /* CFErrorCreateWithUserInfoKeysAndValues builds the user info for us. */
  keys[0] = kCFErrorLocalizedFailureReasonKey;
  values[0] = CFSTR("the reason");
  err = CFErrorCreateWithUserInfoKeysAndValues (NULL, kCFErrorDomainMach, 13,
    keys, values, 1);
  PASS_CF(err != NULL,
    "CFErrorCreateWithUserInfoKeysAndValues created an error.");
  PASS_CFEQ(CFErrorGetDomain (err), kCFErrorDomainMach,
    "Keys-and-values error has the requested domain.");
  PASS_CF(CFErrorGetCode (err) == 13,
    "Keys-and-values error has the requested code.");
  copy = CFErrorCopyUserInfo (err);
  PASS_CF(CFDictionaryGetCount (copy) == 1,
    "Keys-and-values error carries one user info entry.");
  PASS_CFEQ(CFDictionaryGetValue (copy, kCFErrorLocalizedFailureReasonKey),
    CFSTR("the reason"), "Keys-and-values error preserved the value.");
  CFRelease (copy);
  CFRelease (err);

  return 0;
}
