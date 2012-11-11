#import <Foundation/NSString.h>
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  NSString *str;
  CFStringRef str2;
  
  str = (NSString*)CFSTR("test");
  PASS_CF ([str retainCount] == UINT_MAX,
    "Constant string has maximum retain count.");
  [str release];
  PASS_CF ([str retainCount] == UINT_MAX,
    "Release has no effect of constant string.");
  
  str2 = CFStringCreateWithBytes (NULL, "Test2", 6, kCFStringEncodingASCII, 0);
  [str2 retain];
  PASS_CF ([str2 retainCount] == 2,
    "CFStringRef instances accept -retain method calls");
  
  return 0;
}