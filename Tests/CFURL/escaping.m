#include "CoreFoundation/CFURL.h"

#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFStringRef str2;
  
  str = CFURLCreateStringByAddingPercentEscapes (NULL,
    CFSTR("http://www.gnustep.org/Test file.html"), NULL, NULL,
    kCFStringEncodingUTF8);
  
  PASS_CFEQ(str, CFSTR("http://www.gnustep.org/Test%20file.html"),
    "URL string is escaped correctly.");
  
  return 0;
}