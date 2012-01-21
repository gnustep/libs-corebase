#include "CoreFoundation/CFURL.h"

#include "../CFTesting.h"

int main (void)
{
  CFURLRef url;
  CFURLRef url2;
  CFURLRef url3;
  CFStringRef str;
  
  url = CFURLCreateWithString (NULL,
    CFSTR("http://here.and.there/testing/one.html"), NULL);
  url2 = CFURLCreateWithString (NULL, CFSTR("aaa/bbb/ccc/"), url);
  CFRelease (url);
  url = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url),
    CFSTR("http://here.and.there/testing/aaa/bbb/ccc/"),
    "Simple relative URL absoluteString works");
  str = CFURLCopyPath (url);
  PASS_CFEQ(str, CFSTR("/testing/aaa/bbb/ccc"),
    "Simple relative URL path works");
  
  CFRelease (url);
  CFRelease (url2);
  CFRelease (str);
  
  /* Examples from RFC 3986 */
  url = CFURLCreateWithString (NULL, CFSTR("http://a/b/c/d;p?q"), NULL);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("g:h"), NULL);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("g:h"),
    "g:h resolved against http://a/b/c/d;p?q is g:h");
  CFRelease(url3);
  CFRelease(url2);
  
  CFRelease(url);
  
  return 0;
}