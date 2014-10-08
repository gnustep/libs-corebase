#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef o;
  
  o = CFStringCreateWithFormat (NULL, NULL, CFSTR("%z"), "test");
  PASS_CF(o == NULL, "Format with invalid spec (%%z) returns NULL.");

  o = CFStringCreateWithFormat (NULL, NULL, CFSTR("%s %01.2z"), "test", NULL);
  PASS_CF(o == NULL, "Complex format with invalid spec (%%01.2z) returns NULL.");

  o = CFStringCreateWithFormat (NULL, NULL, CFSTR("%s %1$s"), "test1", "test2");
  PASS_CF(o == NULL, "Specifying only 1 positional parameter returns NULL.");

  return 0;
}
