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
  
  testHopeful = TRUE;
  
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
  CFRelease (str);
  str = CFURLCopyFileSystemPath (url, kCFURLWindowsPathStyle);
  PASS_CFEQ(str, CFSTR("C:\\WINDOWS\\"), "File system path is C:\\WINDOWS\\");
  
  CFRelease (url);
  CFRelease (url2);
  CFRelease (str);
  
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("/usr"),
    kCFURLPOSIXPathStyle, true);
  PASS_CFEQ(CFURLGetString(url), CFSTR("/usr/"),
    "String for file URL /usr is /usr/");
  url2 = CFURLCopyAbsoluteURL (url);
  PASS_CFEQ(CFURLGetString(url2), CFSTR("file://localhost/usr/"),
    "File URL /usr is file://localhost/usr/");
  str = CFURLCopyResourceSpecifier (url);
  PASS_CFEQ(str, CFSTR("//localhost/usr/"),
    "Resource Specifier of /usr is //localhost/usr/");
  CFRelease (str);
  str = CFURLCopyFileSystemPath (url, kCFURLPOSIXPathStyle);
  PASS_CFEQ(str, CFSTR("/usr/"), "File system path is /usr/");
  
  testHopeful = FALSE;
  
  CFRelease (url);
  CFRelease (url2);
  CFRelease (str);
  
  url = CFURLCreateWithString (NULL,
    CFSTR("http://user:password@www.w3.org:5/silly-file-path/"), NULL);
  str = CFURLCopyPath (url);
  PASS_CFEQ(str, CFSTR("/silly-file-path/"), "Path of "
    "http://user:password@www.w3.org:5/silly-file-path/ is /silly-file-path/");
  CFRelease (str);
  str = CFURLCopyUserName (url);
  PASS_CFEQ(str, CFSTR("user"), "User name of "
    "http://user:password@www.w3.org:5/silly-file-path/ is user");
  CFRelease (str);
  str = CFURLCopyPassword (url);
  PASS_CFEQ(str, CFSTR("password"), "Password of "
    "http://user:password@www.w3.org:5/silly-file-path/ is password");
  CFRelease (str);
  str = CFURLCopyResourceSpecifier (url);
  PASS_CFEQ(str, CFSTR("//user:password@www.w3.org:5/silly-file-path/"),
    "resourceSpecifier of http://www.w3.org/silly-file-path/ is "
    "//www.w3.org/silly-file-path/");
  
  CFRelease (url);
  CFRelease (str);
  
  url = CFURLCreateWithString (NULL,
    CFSTR("http://www.w3.org/silly-file-name?query#fragment"), NULL);
  str = CFURLCopyScheme (url);
  PASS_CFEQ(str, CFSTR("http"),
       "Scheme of http://www.w3.org/silly-file-name is http");
  CFRelease (str);
  str = CFURLCopyHostName (url);
  PASS_CFEQ(str, CFSTR("www.w3.org"),
    "Host of http://www.w3.org/silly-file-name is www.w3.org");
  CFRelease (str);
  str = CFURLCopyStrictPath (url, NULL);
  testHopeful = TRUE;
  PASS_CFEQ(str, CFSTR("silly-file-name"),
    "Strict path of http://www.w3.org/silly-file-name is silly-file-name");
  testHopeful = FALSE;
  CFRelease (str);
  str = CFURLCopyResourceSpecifier (url);
  PASS_CFEQ(str, CFSTR("//www.w3.org/silly-file-name?query#fragment"),
    "Resource specifier of http://www.w3.org/silly-file-name is "
    "//www.w3.org/silly-file-name?query#fragment");
  CFRelease(str);
  str = CFURLCopyQueryString (url, NULL);
  PASS_CFEQ(str, CFSTR("query"), "Query of "
    "http://www.w3.org/silly-file-name?query#fragment is query");
  CFRelease (str);
  str = CFURLCopyFragment (url, NULL);
  PASS_CFEQ(str, CFSTR("fragment"), "Fragment of "
    "http://www.w3.org/silly-file-name?query#fragment is fragment");
  CFRelease (str);
  
  url = CFURLCreateWithString (NULL, CFSTR("this isn't a URL"), NULL);
  PASS(url == NULL, "URL with 'this isn't a URL' returns NULL");
  
  return 0;
}