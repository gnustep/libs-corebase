#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFTimeZoneRef tz;
  
  tz = CFTimeZoneCreateWithName (NULL, CFSTR("America/Chicago"), true);
  PASS(tz != NULL, "CFTimeZone create successfully.");
  
  return 0;
}
