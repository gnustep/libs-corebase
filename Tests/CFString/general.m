#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFArrayRef array;
  CFRange found;
  
  /* This is used by CFURL. */
  array = CFStringCreateArrayBySeparatingStrings (NULL,
    CFSTR("/usr/local/share/GNUstep/"), CFSTR("/"));
  PASS(CFArrayGetCount(array) == 6, "There are 6 strings on separated string.");
  str = CFArrayGetValueAtIndex (array, 1);
  PASS_CFEQ(str, CFSTR("usr"), "Value at index 1 is 'usr'");
  
  str = CFStringCreateByCombiningStrings (NULL, array, CFSTR("\\"));
  PASS_CFEQ(str, CFSTR("\\usr\\local\\share\\GNUstep\\"),
    "Combined string is \\usr\\local\\share\\GNUstep\\");
  
  CFRelease (array);
  
  PASS(CFStringFindWithOptions(str, CFSTR("\\"), CFRangeMake(5, 12), 0, &found),
    "'\\' was found.");
  PASS(found.location == 5 && found.length == 1, "String has range (5, 1)");
  printf ("(%d, %d)", found.location, found.length);
  
  CFRelease (str);
  
  return 0;
}