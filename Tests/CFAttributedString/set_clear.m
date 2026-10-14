#include "CoreFoundation/CFAttributedString.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFDictionary.h"

#include "../CFTesting.h"

static CFDictionaryRef
oneAttr (CFStringRef k, CFStringRef v)
{
  const void *kk = k;
  const void *vv = v;
  return CFDictionaryCreate (NULL, &kk, &vv, 1,
                             &kCFTypeDictionaryKeyCallBacks,
                             &kCFTypeDictionaryValueCallBacks);
}

int
main (void)
{
  CFRange er;
  CFDictionaryRef d;
  CFDictionaryRef repl;
  CFTypeRef v;
  CFMutableAttributedStringRef m;

  m = CFAttributedStringCreateMutable (NULL, 0);
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("abc"));
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 3), CFSTR ("x"),
                                  CFSTR ("1"));

  repl = oneAttr (CFSTR ("y"), CFSTR ("2"));
  CFAttributedStringSetAttributes (m, CFRangeMake (0, 3), repl, true);

  d = CFAttributedStringGetAttributes (m, 1, &er);
  PASS_CF(CFDictionaryGetValue (d, CFSTR ("x")) == NULL,
          "Clearing other attributes removes the old attribute.");
  v = CFDictionaryGetValue (d, CFSTR ("y"));
  PASS_CF(v != NULL && CFEqual (v, CFSTR ("2")),
          "Clearing other attributes sets the new attribute.");
  PASS_CF(CFDictionaryGetCount (d) == 1, "Only the new attribute remains.");

  CFRelease (repl);
  CFRelease (m);

  return 0;
}
