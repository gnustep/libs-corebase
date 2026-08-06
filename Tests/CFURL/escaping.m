#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFString.h"

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

  /* A non-ASCII character escapes to its multi-byte UTF-8 form: U+20AC ->
     "%E2%82%AC" (9 chars).  The output buffer used to be sized at 3 chars per
     input character, so this overran it. */
  {
    const UniChar mb[] = { 0x20AC, 'a', 'b' };
    CFStringRef mbStr = CFStringCreateWithCharacters (NULL, mb, 3);

    str = CFURLCreateStringByAddingPercentEscapes (NULL, mbStr, NULL, NULL,
      kCFStringEncodingUTF8);
    PASS_CFEQ(str, CFSTR("%E2%82%ACab"),
      "A non-ASCII character is escaped to multi-byte UTF-8 without overflow.");
    CFRelease (str);
    CFRelease (mbStr);
  }

  return 0;
}