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
  PASS_CFEQ(str, CFSTR("/testing/aaa/bbb/ccc/"),
    "Simple relative URL path works");
  
  CFRelease (url);
  CFRelease (url2);
  CFRelease (str);
  
  url = CFURLCreateWithString (NULL, CFSTR("http://a"), NULL);
  url2 = CFURLCreateWithString (NULL, CFSTR("../g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/g"),
    "../g resolved against http://a is http://a/g");
  
  CFRelease(url);
  CFRelease(url2);
  CFRelease(url3);
  
  /* Examples from RFC 3986 */
  url = CFURLCreateWithString (NULL, CFSTR("http://a/b/c/d;p?q"), NULL);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("g:h"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("g:h"),
    "g:h resolved against http://a/b/c/d;p?q is g:h");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("./g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/c/g"),
    "./g resolved against http://a/b/c/d;p?q is http://a/b/c/g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("//g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://g"),
    "//g resolved against http://a/b/c/d;p?q is http://g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("#s"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/c/d;p?q#s"),
    "#s resolved against http://a/b/c/d;p?q is http:/a/b/c/d;p?q#s");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("g;x?y#s"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/c/g;x?y#s"),
    "g;x?y#s resolved against http://a/b/c/d;p?q is http://a/b/c/g;x?y#s");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("../.."), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/"),
    "../.. resolved against http://a/b/c/d;p?q is http://a/");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("../../g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/g"),
    "../../g resolved against http://a/b/c/d;p?q is http://a/g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("../../../../g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/g"),
    "../../../../g resolved against http://a/b/c/d;p?q is http://a/g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("/./g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/g"),
    "/./g resolved against http://a/b/c/d;p?q is http://a/g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("/../g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/g"),
    "/../g resolved against http://a/b/c/d;p?q is http://a/g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("..g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/c/..g"),
    "..g resolved against http://a/b/c/d;p?q is http://a/b/c/..g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("./../g"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/g"),
    "./../g resolved against http://a/b/c/d;p?q is http://a/b/g");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("g;x=1/./y"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/c/g;x=1/y"),
    "g;x=1/./y resolved against http://a/b/c/d;p?q is http://a/b/c/g;x=1/y");
  CFRelease(url3);
  CFRelease(url2);
  
  url2 = CFURLCreateWithString (NULL, CFSTR("g?y/../x"), url);
  url3 = CFURLCopyAbsoluteURL (url2);
  PASS_CFEQ(CFURLGetString(url3), CFSTR("http://a/b/c/g?y/../x"),
    "g?y/../x resolved against http://a/b/c/d;p?q is http://a/b/c/g?y/../x");
  CFRelease(url3);
  CFRelease(url2);
  
  CFRelease(url);
  
  return 0;
}