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
  
  return 0;
}
