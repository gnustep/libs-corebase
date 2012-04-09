#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFTimeZoneRef tz;
  
  tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  PASS(tz != NULL, "CFTimeZone create successfully.");
  PASS_CFEQ(CFTimeZoneGetName(tz), CFSTR("GMT+0000"),
    "CFTimeZone has correct name.");
  
  CFRelease(tz);
  
  return 0;
}
