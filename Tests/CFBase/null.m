#include "CoreFoundation/CFBase.h"
#include "../CFTesting.h"

int main (void)
{
  PASS_CF (CFNullGetTypeID () != 0, "CFNullGetTypeID is registered.");
  PASS_CF (CFGetTypeID (kCFNull) == CFNullGetTypeID (),
    "kCFNull has the null type ID.");
  PASS_CF (CFEqual (kCFNull, kCFNull), "kCFNull is equal to itself.");

  return 0;
}
