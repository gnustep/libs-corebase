#include "CoreFoundation/CFURL.h"

#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFStringRef str2;
  CFStringRef urlStr = CFSTR("http://www.gnustep.org/Test file.html");
  
  str = CFURLCreateStringByAddingPercentEscapes (NULL, urlStr, NULL, NULL,
    kCFStringEncodingUTF8);
  PASS_CFEQ(str, CFSTR("http://www.gnustep.org/Test%20file.html"),
    "URL string is escaped correctly.");
  CFRelease (str);
  
  str = CFURLCreateStringByAddingPercentEscapes (NULL, urlStr, CFSTR(" "),
    CFSTR("ep"), kCFStringEncodingUTF8);
  PASS_CFEQ(str, CFSTR("htt%70://www.gnust%65%70.org/T%65st fil%65.html"),
    "Correct characters escaped.");
  
  str2 = CFURLCreateStringByReplacingPercentEscapes (NULL, str, NULL);
  PASS_CFEQ(str2, str, "No percent escapes were replaced.");
  CFRelease (str2);
  
  str2 = CFURLCreateStringByReplacingPercentEscapes (NULL, str, CFSTR(""));
  PASS_CFEQ(str2, urlStr, "Percent escapes replaced correctly.");
  
  CFRelease (str);
  CFRelease (str2);
  
  str = CFURLCreateStringByAddingPercentEscapes (NULL, urlStr, NULL,
    CFSTR("nm/."), kCFStringEncodingUTF8);
  PASS_CFEQ(str, CFSTR("http:%2F%2Fwww%2Eg%6Eustep%2Eorg%2FTest%20file%2Eht%6Dl"),
    "Correct characters escaped.");
  
  str2 = CFURLCreateStringByReplacingPercentEscapes (NULL, str, CFSTR(""));
  PASS_CFEQ(str2, urlStr, "Percent escapes replaced correctly.");
  
  CFRelease (str);
  CFRelease (str2);
  
  return 0;
}