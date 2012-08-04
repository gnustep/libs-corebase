#include "CoreFoundation/CFURL.h"
#include "../CFTesting.h"

int main (void)
{
  CFURLRef url;
  CFURLRef baseURL;
  CFStringRef str;
  
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("C:\\WINDOWS"),
    kCFURLWindowsPathStyle, true);
  PASS_CF(CFURLCanBeDecomposed(url) == true, "C:\\WINDOWS path can be decomposed");
  PASS_CFEQ(CFURLGetString(url), CFSTR("file://localhost/C:/WINDOWS/"),
    "Windows style path of file URL C:\\WINDOWS is file://localhost/C:/WINDOWS/");
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
  PASS_CFEQ(CFURLGetString(url), CFSTR("file://localhost/usr/"),
    "String for file URL /usr is /usr/");
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
  
  return false;
}