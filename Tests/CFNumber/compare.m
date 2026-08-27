#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

int main (void)
{
  int a = 1, b = 2;
  double x = 1.5, y = 2.5;
  CFNumberRef na, nb, nx, ny;

  na = CFNumberCreate (NULL, kCFNumberIntType, &a);
  nb = CFNumberCreate (NULL, kCFNumberIntType, &b);
  PASS_CF(CFNumberCompare (na, nb, NULL) == kCFCompareLessThan,
    "CFNumberCompare orders a smaller integer first.");
  PASS_CF(CFNumberCompare (nb, na, NULL) == kCFCompareGreaterThan,
    "CFNumberCompare orders a larger integer last.");
  PASS_CF(CFNumberCompare (na, na, NULL) == kCFCompareEqualTo,
    "Equal integers compare equal.");

  nx = CFNumberCreate (NULL, kCFNumberDoubleType, &x);
  ny = CFNumberCreate (NULL, kCFNumberDoubleType, &y);
  PASS_CF(CFNumberCompare (nx, ny, NULL) == kCFCompareLessThan,
    "CFNumberCompare orders a smaller double first.");
  PASS_CF(CFNumberCompare (ny, nx, NULL) == kCFCompareGreaterThan,
    "CFNumberCompare orders a larger double last.");
  PASS_CF(CFNumberCompare (na, nx, NULL) == kCFCompareLessThan,
    "CFNumberCompare compares an integer against a double.");

  CFRelease (na);
  CFRelease (nb);
  CFRelease (nx);
  CFRelease (ny);

  return 0;
}
