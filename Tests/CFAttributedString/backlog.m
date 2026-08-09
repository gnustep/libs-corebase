#include "CoreFoundation/CFAttributedString.h"
#include "CoreFoundation/CFString.h"

#include "../CFTesting.h"

static Boolean
attrIs (CFAttributedStringRef s, CFIndex i, CFStringRef key, CFStringRef want)
{
  CFTypeRef v = CFAttributedStringGetAttribute (s, i, key, NULL);
  if (want == NULL)
    return v == NULL;
  return v != NULL && CFEqual (v, want);
}

int
main (void)
{
  CFMutableAttributedStringRef m;
  CFAttributedStringRef sub;

  /* CreateWithSubstring: "Hello World" with (0,5)=color and (6,5)=weight;
   * the substring (3,5) is "lo Wo" with color over its first two characters,
   * a bare space, then weight over its last two.
   */
  m = CFAttributedStringCreateMutable (NULL, 0);
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("Hello World"));
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 5), CFSTR ("color"),
                                  CFSTR ("red"));
  CFAttributedStringSetAttribute (m, CFRangeMake (6, 5), CFSTR ("weight"),
                                  CFSTR ("bold"));
  sub = CFAttributedStringCreateWithSubstring (NULL, m, CFRangeMake (3, 5));
  PASS_CFEQ(CFAttributedStringGetString (sub), CFSTR ("lo Wo"),
            "The substring holds the requested characters.");
  PASS_CF(attrIs (sub, 0, CFSTR ("color"), CFSTR ("red"))
          && attrIs (sub, 1, CFSTR ("color"), CFSTR ("red")),
          "The source color is remapped over the substring's first run.");
  PASS_CF(attrIs (sub, 2, CFSTR ("color"), NULL)
          && attrIs (sub, 2, CFSTR ("weight"), NULL),
          "The gap between the source runs stays bare.");
  PASS_CF(attrIs (sub, 3, CFSTR ("weight"), CFSTR ("bold"))
          && attrIs (sub, 4, CFSTR ("weight"), CFSTR ("bold")),
          "The source weight is remapped over the substring's last run.");
  PASS_CF(attrIs (sub, 3, CFSTR ("color"), NULL),
          "The substring does not carry color past the source run.");
  CFRelease (sub);
  CFRelease (m);

  /* RemoveAttribute clears a single key across the range and leaves the
   * others in place.
   */
  m = CFAttributedStringCreateMutable (NULL, 0);
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("Hello World"));
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 11), CFSTR ("color"),
                                  CFSTR ("red"));
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 5), CFSTR ("weight"),
                                  CFSTR ("bold"));
  CFAttributedStringRemoveAttribute (m, CFRangeMake (0, 11), CFSTR ("color"));
  PASS_CF(attrIs (m, 0, CFSTR ("color"), NULL)
          && attrIs (m, 5, CFSTR ("color"), NULL)
          && attrIs (m, 10, CFSTR ("color"), NULL),
          "color is removed across the whole range.");
  PASS_CF(attrIs (m, 0, CFSTR ("weight"), CFSTR ("bold"))
          && attrIs (m, 4, CFSTR ("weight"), CFSTR ("bold")),
          "weight is left untouched by the color removal.");
  PASS_CF(attrIs (m, 5, CFSTR ("weight"), NULL),
          "The run that only carried color is now bare.");
  CFRelease (m);

  /* ReplaceAttributedString swaps in the replacement's characters and its
   * attributes, and keeps the surrounding attributes.
   */
  m = CFAttributedStringCreateMutable (NULL, 0);
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("Hello World"));
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 11), CFSTR ("color"),
                                  CFSTR ("red"));
  {
    CFMutableAttributedStringRef repl = CFAttributedStringCreateMutable (NULL, 0);
    CFAttributedStringReplaceString (repl, CFRangeMake (0, 0), CFSTR ("HI"));
    CFAttributedStringSetAttribute (repl, CFRangeMake (0, 2), CFSTR ("weight"),
                                    CFSTR ("bold"));
    CFAttributedStringReplaceAttributedString (m, CFRangeMake (0, 5), repl);
    CFRelease (repl);
  }
  PASS_CFEQ(CFAttributedStringGetString (m), CFSTR ("HI World"),
            "The replacement characters are spliced in.");
  PASS_CF(attrIs (m, 0, CFSTR ("weight"), CFSTR ("bold"))
          && attrIs (m, 1, CFSTR ("weight"), CFSTR ("bold")),
          "The replacement's own attributes cover the inserted text.");
  PASS_CF(attrIs (m, 0, CFSTR ("color"), NULL),
          "The inserted text does not inherit the surrounding color.");
  PASS_CF(attrIs (m, 2, CFSTR ("color"), CFSTR ("red"))
          && attrIs (m, 7, CFSTR ("color"), CFSTR ("red")),
          "The surviving text keeps its original color.");
  CFRelease (m);

  return 0;
}
