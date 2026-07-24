#include "CoreFoundation/CFBase.h"
#include "../CFTesting.h"

int main (void)
{
  PASS_CFEQ (CFCopyTypeIDDescription (CFNullGetTypeID ()), CFSTR ("CFNull"),
    "The null type is described as CFNull.");

  return 0;
}
