#import <Foundation/NSValue.h>
#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

int main (void)
{
  NSNumber *i = [NSNumber numberWithInt: 5];

  PASS_CF(CFNumberGetType((CFNumberRef)i) == kCFNumberIntType,
    "An integer NSNumber reports kCFNumberIntType, not kCFNumberDoubleType.");

  return 0;
}
