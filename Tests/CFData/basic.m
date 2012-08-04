#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

const UInt8 bytes[] = { 0xDE, 0xAD, 0xBA, 0xBA };

int main (void)
{
  CFDataRef data;
  CFDataRef data2;
  CFIndex length;
  UInt8 *copy;
  
  data = CFDataCreate (NULL, bytes, sizeof(bytes));
  PASS_CF(data != NULL, "Data created.");
  
  length = CFDataGetLength (data);
  copy = CFAllocatorAllocate (NULL, length, 0);
  CFDataGetBytes (data, CFRangeMake(0, length), copy);
  
  data2 = CFDataCreateWithBytesNoCopy (NULL, copy, length, kCFAllocatorDefault);
  PASS_CFEQ(data, data2, "Copy of data is equal to original.");
  
  CFRelease (data);
  CFRelease (data2);
  
  return 0;
}