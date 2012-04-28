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
  
  url = CFURLCreateWithString (NULL,
    CFSTR("http://user:password@www.w3.org:5/silly-file-path/"), NULL);
  PASS(CFURLCanBeDecomposed(url) == true,
    "http://user:password@www.w3.org:5/silly-file-path/ path can be decomposed");
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
  PASS(str == NULL,
    "resourceSpecifier of http://www.w3.org/silly-file-path/ is NULL");
  
  url2 = CFURLCreateWithString (NULL, CFSTR("additional/string"), url);
  PASS(CFURLCanBeDecomposed(url2) == true,
    "additional/string path can be decomposed since base URL is decomposable");
  str = CFURLGetString (url2);
  PASS_CFEQ(str, CFSTR("additional/string"), "String is additional/string");
  str = CFURLCopyPath (url2);
  PASS_CFEQ(str, CFSTR("additional/string"),
    "Copied path is not resolved against base.");
  CFRelease (str);
  str = CFURLCopyResourceSpecifier (url2);
  PASS(str == NULL, "Resource specifier of relative url is additional/string");
  
  CFRelease (url);
  CFRelease (url2);
  
  url = CFURLCreateWithString (NULL,
    CFSTR("http://www.w3.org/silly-file-name?query#fragment"), NULL);
  PASS(CFURLCanBeDecomposed(url) == true,
    "http://www.w3.org/silly-file-name?query#fragment path can be decomposed");
  str = CFURLCopyScheme (url);
  PASS_CFEQ(str, CFSTR("http"),
       "Scheme of http://www.w3.org/silly-file-name is http");
  CFRelease (str);
  str = CFURLCopyHostName (url);
  PASS_CFEQ(str, CFSTR("www.w3.org"),
    "Host of http://www.w3.org/silly-file-name is www.w3.org");
  CFRelease (str);
  str = CFURLCopyStrictPath (url, NULL);
  PASS_CFEQ(str, CFSTR("silly-file-name"),
    "Strict path of http://www.w3.org/silly-file-name is silly-file-name");
  CFRelease (str);
  str = CFURLCopyResourceSpecifier (url);
  PASS_CFEQ(str, CFSTR("query#fragment"),
    "Resource specifier of http://www.w3.org/silly-file-name?query#fragment is"
    " query#fragment");
  CFRelease(str);
  str = CFURLCopyQueryString (url, NULL);
  PASS_CFEQ(str, CFSTR("query"), "Query of "
    "http://www.w3.org/silly-file-name?query#fragment is query");
  CFRelease (str);
  str = CFURLCopyFragment (url, NULL);
  PASS_CFEQ(str, CFSTR("fragment"), "Fragment of "
    "http://www.w3.org/silly-file-name?query#fragment is fragment");
  CFRelease (str);
  
  CFRelease (url);
  
  url = CFURLCreateWithString (NULL,
    CFSTR("http://www.w3.org/silly;type=test"), NULL);
  str = CFURLCopyParameterString (url, NULL);
  PASS_CFEQ(str, CFSTR("type=test"), "Parameter string for "
    "http://www.w3.org/silly;type=test");
  CFRelease(str);
  
  CFRelease(url);
  
  url = CFURLCreateWithString (NULL, CFSTR("this isn't a URL"), NULL);
  PASS(url == NULL, "URL with 'this isn't a URL' returns NULL");
  
  return 0;
}