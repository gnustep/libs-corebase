#include "CoreFoundation/CFString.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  CFStringRef str2;
  void *ptr;
  
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("%c %c%c"), 'r', 'u', 'n');
  str2 = CFSTR ("r un");
  PASS_CFEQ(str1, str2, "Characters are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("'%2$04d %d'"), 3, 0x00a4);
  str2 = CFSTR ("'0164 3'");
  PASS_CFEQ(str1, str2, "Integers are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("%f %1$-5.2g %e"), 5.5, .000003);
  str2 = CFSTR ("5.500000 5.5   3.000000e-06");
  PASS_CFEQ(str1, str2, "Float/Doubles are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("%x %1$#08X %hhx"), 10788, 256);
  str2 = CFSTR ("2a24 0X00002A24 0");
  PASS_CFEQ(str1, str2, "Hexs are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("%o %1$#06o %hho"), 10788, 256);
  str2 = CFSTR ("25044 0025044 0");
  PASS_CFEQ(str1, str2, "Octals are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  ptr = (void*)12;
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("%p %s"), ptr, "A longer than usual string.");
  str2 = CFSTR ("0xc A longer than usual string.");
  PASS_CFEQ(str1, str2, "Strings are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  str2 = CFSTR ("Object test");
  str1 = CFStringCreateWithFormat (NULL, NULL,
    CFSTR("%@"), str2);
  PASS_CFEQ(str1, str2, "Objects are formatted correctly");
  CFRelease(str1);
  CFRelease(str2);
  
  return 0;
}
