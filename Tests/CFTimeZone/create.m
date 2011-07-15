#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFTimeZoneRef tz;
  
  tz = CFTimeZoneCreateWithName (NULL, CFSTR("America/Chicago"), true);
  PASS(tz != NULL, "CFTimeZone create successfully.");
  PASS_CFEQ(CFTimeZoneGetName(tz), CFSTR("America/Chicago"),
    "CFTimeZone has correct name.");
  
  CFRelease(tz);
  
  return 0;
}
