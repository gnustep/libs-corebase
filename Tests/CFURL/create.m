#include "CoreFoundation/CFURL.h"

#include "../CFTesting.h"

int main (void)
{
  CFURLRef url;
  CFURLRef url2;
  CFStringRef str;
  
  url = CFURLCreateWithString (NULL, CFSTR("http://www.gnustep.org"), NULL);
  PASS(url != NULL, "URL created.");
  
  url2 = CFURLCopyAbsoluteURL (url);
  PASS(url == url2, "Absolute URL of an absolute URL are the same.");
  
  CFRelease (url);
  CFRelease (url2);
  
  /* These next few tests copied from gnustep-base. */
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("C:\\WINDOWS"),
    kCFURLWindowsPathStyle, true);
  PASS_CFEQ(CFURLGetString(url), CFSTR("C:\\WINDOWS\\"),
    "Windows style path of file URL C:\\WINDOWS is C:\\WINDOWS");
  url2 = CFURLCopyAbsoluteURL (url);
  PASS_CFEQ(CFURLGetString(url2), CFSTR("file://localhost/C:%5CWINDOWS/"),
    "Windows style file URL C:\\WINDOWS is file://localhost/C:%%5CWINDOWS/");
  str = CFURLCopyResourceSpecifier (url);
  PASS_CFEQ(str, CFSTR("//localhost/C:%5CWINDOWS/"),
    "Resource specifier of C:\\WINDOWS is //localhost/C:%%5CWINDOWS/");
  
  CFRelease (url);
  CFRelease (url2);
  CFRelease (str);
  
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("/usr"),
    kCFURLPOSIXPathStyle, true);
  PASS_CFEQ(CFURLGetString(url), CFSTR("/usr/"),
    "Path of file URL /usr is /usr");
  url2 = CFURLCopyAbsoluteURL (url);
  PASS_CFEQ(CFURLGetString(url2), CFSTR("file://localhost/usr/"),
    "File URL /usr is file://localhost/usr/");
  str = CFURLCopyResourceSpecifier (url);
  PASS_CFEQ(str, CFSTR("//localhost/usr/"),
    "Resource Specifier of /usr is //localhost/usr/");
  
  CFRelease (url);
  CFRelease (url2);
  CFRelease (str);
  
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
  
  return 0;
}