#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

static Boolean
mem (UniChar c)
{
  return CFCharacterSetIsCharacterMember (
    CFCharacterSetGetPredefined (kCFCharacterSetDecomposable), c);
}

int main (void)
{
  PASS_CF(mem (0x00C0),
    "U+00C0 (canonical decomposition) is decomposable.");
  PASS_CF(!mem (0xFB00),
    "U+FB00 (compatibility-only ligature) is not decomposable.");
  PASS_CF(!mem (0x00BD),
    "U+00BD (compatibility-only fraction) is not decomposable.");
  PASS_CF(!mem ('A'), "A is not decomposable.");
  PASS_CF(!mem (0x0301), "A combining mark is not decomposable.");

  return 0;
}
