#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFUUID.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFUUIDRef uuid1;
  CFUUIDRef uuid2;
  
  uuid1 = CFUUIDCreate (NULL);
  PASS_CF(uuid1 != NULL, "CFUUIDCreate() returns non-NULL.");
  if (uuid1 == NULL)
    return 1;
  
  uuid2 = CFUUIDCreateFromUUIDBytes (NULL, CFUUIDGetUUIDBytes(uuid1));
  PASS_CFEQ(uuid1, uuid2, "UUID creates with CFUUIDBytes is correct.");
  CFRelease(uuid2);
  
  str = CFUUIDCreateString (NULL, uuid1);
  PASS_CF(str != NULL, "CFUUIDCreateString() returns non-NULL.");
  
  CFRelease (uuid1);
  CFRelease (str);

  /* CFUUIDCreateFromString must parse the 16 bytes correctly and round-trip.
     The old parser byte-swapped and shifted the bytes (and wrote past the
     16-byte struct), so this produced a garbage UUID. */
  {
    CFStringRef in = CFSTR("12345678-1234-1234-1234-123456789012");
    CFUUIDRef uuid3 = CFUUIDCreateFromString (NULL, in);
    CFStringRef back;

    PASS_CF(uuid3 != NULL, "CFUUIDCreateFromString() returns non-NULL.");
    back = uuid3 ? CFUUIDCreateString (NULL, uuid3) : NULL;
    PASS_CFEQ(back, in,
      "A UUID string round-trips through CFUUIDCreateFromString.");
    if (back)
      CFRelease (back);
    if (uuid3)
      CFRelease (uuid3);
  }

  return 0;
}
