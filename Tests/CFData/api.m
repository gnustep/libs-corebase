#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

int main (void)
{
  UInt8 bytes[4] = { 1, 2, 3, 4 };
  CFDataRef d, copy;
  CFMutableDataRef m;
  const UInt8 *p;
  UInt8 *mp;

  PASS_CF(CFDataGetTypeID () != 0, "CFDataGetTypeID is not zero.");

  d = CFDataCreate (NULL, bytes, 4);
  PASS_CF(CFGetTypeID (d) == CFDataGetTypeID (),
    "A data object has the data type ID.");

  copy = CFDataCreateCopy (NULL, d);
  PASS_CF(CFEqual (copy, d), "CFDataCreateCopy equals the original.");
  CFRelease (copy);

  m = CFDataCreateMutableCopy (NULL, 0, d);
  PASS_CF(CFEqual (m, d), "CFDataCreateMutableCopy equals the original.");

  mp = CFDataGetMutableBytePtr (m);
  mp[0] = 0xff;
  p = CFDataGetBytePtr (m);
  PASS_CF(p[0] == 0xff, "CFDataGetMutableBytePtr allows writing.");

  CFDataSetLength (m, 6);
  PASS_CF(CFDataGetLength (m) == 6, "CFDataSetLength extends the data.");
  p = CFDataGetBytePtr (m);
  PASS_CF(p[4] == 0 && p[5] == 0, "CFDataSetLength zero-fills the new bytes.");

  CFDataSetLength (m, 2);
  PASS_CF(CFDataGetLength (m) == 2, "CFDataSetLength truncates the data.");

  CFDataIncreaseLength (m, 3);
  PASS_CF(CFDataGetLength (m) == 5, "CFDataIncreaseLength grows the data.");

  CFRelease (m);
  CFRelease (d);

  return 0;
}
