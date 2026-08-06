#include "CoreFoundation/CFURL.h"
#include "../CFTesting.h"

static int
string_matches_either(CFStringRef value, CFStringRef expected1, CFStringRef expected2)
{
  return CFEqual(value, expected1) || CFEqual(value, expected2);
}

int main (void)
{
  CFURLRef url;
  CFURLRef baseURL;
  CFStringRef str;
  
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("C:\\WINDOWS"),
    kCFURLWindowsPathStyle, true);
  PASS_CF(CFURLCanBeDecomposed(url) == true, "C:\\WINDOWS path can be decomposed");
  PASS_CF(string_matches_either(CFURLGetString(url),
      CFSTR("file://localhost/C:/WINDOWS/"),
      CFSTR("file:///C:/WINDOWS/")),
    "Windows style path of file URL C:\\WINDOWS matches an accepted form");
  PASS_CF(CFURLCopyResourceSpecifier (url) == NULL,
    "Resource specifier of C:\\WINDOWS is NULL");
  str = CFURLCopyFileSystemPath (url, kCFURLWindowsPathStyle);
  PASS_CFEQ(str, CFSTR("C:\\WINDOWS"), "File system path is C:\\WINDOWS");
  CFRelease (str);
  str = CFURLCopyPath (url);
  PASS_CFEQ(str, CFSTR("/C:/WINDOWS/"), "Path is /C:/WINDOWS/");
  CFRelease (str);
  str = CFURLCopyStrictPath (url, NULL);
  PASS_CFEQ(str, CFSTR("C:/WINDOWS/"), "Strict path is C:/WINDOWS/");
  CFRelease (str);
  
  CFRelease (url);
  
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("/usr"),
    kCFURLPOSIXPathStyle, true);
  PASS_CF(CFURLCanBeDecomposed(url) == true, "/usr path can be decomposed");
  PASS_CF(string_matches_either(CFURLGetString(url),
      CFSTR("file://localhost/usr/"),
      CFSTR("file:///usr/")),
    "String for file URL /usr matches an accepted form");
  PASS_CF(CFURLCopyResourceSpecifier (url) == NULL,
    "Resource Specifier of /usr is NULL");
  str = CFURLCopyFileSystemPath (url, kCFURLPOSIXPathStyle);
  PASS_CFEQ(str, CFSTR("/usr"), "File system path is /usr");
  CFRelease (str);
  str = CFURLCopyPath (url);
  PASS_CFEQ(str, CFSTR("/usr/"), "Path is /usr/");
  CFRelease (str);
  str = CFURLCopyStrictPath (url, NULL);
  PASS_CFEQ(str, CFSTR("usr/"), "Strict path is usr/");
  CFRelease (str);
  
  CFRelease (url);
  
  baseURL = CFURLCreateWithFileSystemPath (NULL, CFSTR("/usr"),
    kCFURLPOSIXPathStyle, true);
  url = CFURLCreateWithFileSystemPathRelativeToBase (NULL, CFSTR("local/share"),
    kCFURLPOSIXPathStyle, true, baseURL);
  PASS_CF(CFURLCanBeDecomposed(url) == true,
    "local/share path can be decomposed");
  str = CFURLGetString (url);
  PASS_CFEQ(str, CFSTR("local/share/"), "String is local/share/");
  str = CFURLCopyFileSystemPath (url, kCFURLPOSIXPathStyle);
  PASS_CFEQ(str, CFSTR("local/share"),
    "File system path is not resolved against base");
  CFRelease (str);
  PASS_CF(CFURLHasDirectoryPath(url) == true, "local/share is a directory path.");
  
  CFRelease (url);
  CFRelease (baseURL);

  /* CFURLHasDirectoryPath on a URL with an empty string must not read
     index -1. */
  url = CFURLCreateWithString (NULL, CFSTR(""), NULL);
  PASS_CF(CFURLHasDirectoryPath(url) == false,
    "An empty URL is not a directory path (no negative index).");
  CFRelease (url);

  return false;
}
