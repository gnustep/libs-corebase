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

static Boolean
urleq (CFURLRef u, const char *want)
{
  Boolean ok = false;
  if (u != NULL)
    {
      CFStringRef w = CFStringCreateWithCString (NULL, want,
        kCFStringEncodingUTF8);
      ok = CFEqual (CFURLGetString (u), w);
      CFRelease (w);
    }
  return ok;
}

int main (void)
{
  CFURLRef u, r;

  u = mk ("http://host/dir/file.txt");
  r = CFURLCreateCopyDeletingLastPathComponent (NULL, u);
  PASS_CF(urleq (r, "http://host/dir/"),
    "CFURLCreateCopyDeletingLastPathComponent removes the last component.");
  if (r) CFRelease (r);
  r = CFURLCreateCopyDeletingPathExtension (NULL, u);
  PASS_CF(urleq (r, "http://host/dir/file"),
    "CFURLCreateCopyDeletingPathExtension removes the extension.");
  if (r) CFRelease (r);
  CFRelease (u);

  u = mk ("http://host/dir/file");
  r = CFURLCreateCopyAppendingPathExtension (NULL, u, CFSTR("txt"));
  PASS_CF(urleq (r, "http://host/dir/file.txt"),
    "CFURLCreateCopyAppendingPathExtension adds an extension.");
  if (r) CFRelease (r);
  r = CFURLCreateCopyAppendingPathComponent (NULL, u, CFSTR("sub"), false);
  PASS_CF(urleq (r, "http://host/dir/file/sub"),
    "CFURLCreateCopyAppendingPathComponent appends a component.");
  if (r) CFRelease (r);
  r = CFURLCreateCopyAppendingPathComponent (NULL, u, CFSTR("sub"), true);
  PASS_CF(urleq (r, "http://host/dir/file/sub/"),
    "CFURLCreateCopyAppendingPathComponent adds a trailing slash for a directory.");
  if (r) CFRelease (r);
  CFRelease (u);

  return 0;
}
