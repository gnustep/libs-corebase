#include "CoreFoundation/CFURL.h"
#include "../CFTesting.h"

static CFURLRef
mk (const char *s)
{
  CFStringRef str = CFStringCreateWithCString (NULL, s, kCFStringEncodingUTF8);
  CFURLRef u = CFURLCreateWithString (NULL, str, NULL);
  CFRelease (str);
  return u;
}

int main (void)
{
  CFURLRef u;
  CFStringRef s;

  u = mk ("http://host/dir/file.txt");
  s = CFURLCopyLastPathComponent (u);
  PASS_CFEQ(s, CFSTR("file.txt"),
    "CFURLCopyLastPathComponent returns the last component.");
  CFRelease (s);
  s = CFURLCopyPathExtension (u);
  PASS_CFEQ(s, CFSTR("txt"),
    "CFURLCopyPathExtension returns the extension.");
  CFRelease (s);
  CFRelease (u);

  u = mk ("http://host/dir/");
  s = CFURLCopyLastPathComponent (u);
  PASS_CFEQ(s, CFSTR("dir"),
    "CFURLCopyLastPathComponent ignores a trailing slash.");
  CFRelease (s);
  CFRelease (u);

  u = mk ("http://host/dir/file");
  s = CFURLCopyPathExtension (u);
  PASS_CF(s == NULL,
    "CFURLCopyPathExtension returns NULL when there is no extension.");
  CFRelease (u);

  u = mk ("http://host/file.tar.gz");
  s = CFURLCopyPathExtension (u);
  PASS_CFEQ(s, CFSTR("gz"),
    "CFURLCopyPathExtension returns the final extension.");
  CFRelease (s);
  CFRelease (u);

  return 0;
}
