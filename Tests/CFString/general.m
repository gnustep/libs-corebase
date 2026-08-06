#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFArray.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFArrayRef array;
  CFRange found;
  
  /* This is used by CFURL. */
  array = CFStringCreateArrayBySeparatingStrings (NULL,
    CFSTR("/usr/local/share/GNUstep/"), CFSTR("/"));
  PASS_CF(CFArrayGetCount(array) == 6, "There are 6 strings on separated string.");
  str = CFArrayGetValueAtIndex (array, 1);
  PASS_CFEQ(str, CFSTR("usr"), "Value at index 1 is 'usr'");
  
  str = CFStringCreateByCombiningStrings (NULL, array, CFSTR("\\"));
  PASS_CFEQ(str, CFSTR("\\usr\\local\\share\\GNUstep\\"),
    "Combined string is \\usr\\local\\share\\GNUstep\\");
  
  CFRelease (array);
  
  PASS_CF(CFStringFindWithOptions(str, CFSTR("\\"), CFRangeMake(5, 12), 0, &found),
    "'\\' was found.");
  PASS_CF(found.location == 10 && found.length == 1, "String has range (%d, %d)",
       (int)found.location, (int)found.length);
  
  CFRelease (str);

  /* An empty array must not read past the end of its (empty) storage. */
  array = CFArrayCreate (NULL, NULL, 0, &kCFTypeArrayCallBacks);
  str = CFStringCreateByCombiningStrings (NULL, array, CFSTR(","));
  PASS_CF(str == NULL, "Combining an empty array returns NULL without overreading.");
  CFRelease (array);

  return 0;
}