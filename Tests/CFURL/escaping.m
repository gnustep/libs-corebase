#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFString.h"

#include "../CFTesting.h"

#include <stdlib.h>
#include <string.h>

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

  /* A '%' with no following hex digits must be rejected without reading
     past the end of the string.  Use a no-copy string with an exact-size
     backing buffer so an over-read is a genuine out-of-bounds access. */
  {
    char *raw = malloc (4);
    CFStringRef trailing;

    memcpy (raw, "abc%", 4);
    trailing = CFStringCreateWithBytesNoCopy (NULL, (const UInt8 *) raw, 4,
      kCFStringEncodingASCII, false, kCFAllocatorNull);
    str2 = CFURLCreateStringByReplacingPercentEscapes (NULL, trailing,
      CFSTR(""));
    PASS_CF(str2 == NULL,
      "A trailing '%%' is rejected without overreading the string.");
    if (str2 != NULL)
      CFRelease (str2);
    CFRelease (trailing);
    free (raw);
  }

  return 0;
}