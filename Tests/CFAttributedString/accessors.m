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
  CFTypeRef v;
  CFDictionaryRef a;
  CFAttributedStringRef s;
  CFAttributedStringRef s2;
  CFMutableAttributedStringRef m;
  CFMutableAttributedStringRef mc;

  a = oneAttr (CFSTR ("color"), CFSTR ("red"));
  s = CFAttributedStringCreate (NULL, CFSTR ("Hello"), a);
  PASS_CF(s != NULL, "Created an attributed string.");
  PASS_CF(CFGetTypeID (s) == CFAttributedStringGetTypeID (),
          "It has the attributed string type ID.");
  PASS_CFEQ(CFAttributedStringGetString (s), CFSTR ("Hello"),
            "It keeps its string.");
  PASS_CF(CFAttributedStringGetLength (s) == 5, "It reports its length.");

  d = CFAttributedStringGetAttributes (s, 2, &er);
  PASS_CF(er.location == 0 && er.length == 5,
          "A single run covers the whole string.");
  PASS_CFEQ(d, a, "The attributes match.");
  v = CFAttributedStringGetAttribute (s, 2, CFSTR ("color"), &er);
  PASS_CFEQ(v, CFSTR ("red"), "GetAttribute returns the value.");
  PASS_CF(er.location == 0 && er.length == 5,
          "GetAttribute reports the effective range.");
  PASS_CF(CFAttributedStringGetAttribute (s, 2, CFSTR ("missing"), NULL) == NULL,
          "A missing attribute is NULL.");

  s2 = CFAttributedStringCreateCopy (NULL, s);
  PASS_CF(CFEqual (s, s2), "A copy is equal to the original.");

  mc = CFAttributedStringCreateMutableCopy (NULL, 0, s);
  PASS_CF(mc != NULL, "Created a mutable copy.");
  PASS_CFEQ(CFAttributedStringGetString (mc), CFSTR ("Hello"),
            "The mutable copy keeps the string.");
  PASS_CFEQ(CFAttributedStringGetAttribute (mc, 2, CFSTR ("color"), NULL),
            CFSTR ("red"), "The mutable copy keeps the attributes.");
  CFRelease (mc);
  CFRelease (s2);
  CFRelease (s);
  CFRelease (a);

  m = CFAttributedStringCreateMutable (NULL, 0);
  PASS_CF(m != NULL, "Created a mutable attributed string.");
  PASS_CF(CFAttributedStringGetLength (m) == 0, "It starts empty.");
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("Hello World"));
  PASS_CF(CFAttributedStringGetLength (m) == 11,
          "Replacing the string sets its length.");
  PASS_CFEQ(CFAttributedStringGetString (m), CFSTR ("Hello World"),
            "Replacing the string sets its characters.");
  d = CFAttributedStringGetAttributes (m, 5, &er);
  PASS_CF(d != NULL && CFDictionaryGetCount (d) == 0
          && er.location == 0 && er.length == 11,
          "A fresh mutable string has one empty attribute run.");
  CFAttributedStringReplaceString (m, CFRangeMake (0, 5), CFSTR ("Goodbye"));
  PASS_CFEQ(CFAttributedStringGetString (m), CFSTR ("Goodbye World"),
            "Replacing a sub-range edits the characters.");
  CFRelease (m);

  return 0;
}
