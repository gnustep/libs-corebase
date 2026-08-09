#include "CoreFoundation/CFAttributedString.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFDictionary.h"

#include "../CFTesting.h"

int
main (void)
{
  CFRange lr;
  CFTypeRef v;
  CFDictionaryRef d;
  CFMutableAttributedStringRef m;

  m = CFAttributedStringCreateMutable (NULL, 0);
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("Hello World"));
  /* (0,5) = {color, shared}, (5,11) = {weight, shared}. */
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 5), CFSTR ("color"),
                                  CFSTR ("red"));
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 11), CFSTR ("shared"),
                                  CFSTR ("x"));
  CFAttributedStringSetAttribute (m, CFRangeMake (5, 6), CFSTR ("weight"),
                                  CFSTR ("bold"));

  v = CFAttributedStringGetAttributeAndLongestEffectiveRange (
        m, 2, CFSTR ("shared"), CFRangeMake (0, 11), &lr);
  PASS_CFEQ(v, CFSTR ("x"), "The longest-range shared value is returned.");
  PASS_CF(lr.location == 0 && lr.length == 11,
          "shared has the same value across both runs.");

  v = CFAttributedStringGetAttributeAndLongestEffectiveRange (
        m, 2, CFSTR ("color"), CFRangeMake (0, 11), &lr);
  PASS_CFEQ(v, CFSTR ("red"), "The longest-range color value is returned.");
  PASS_CF(lr.location == 0 && lr.length == 5,
          "color has the same value only over (0,5).");

  d = CFAttributedStringGetAttributesAndLongestEffectiveRange (
        m, 8, CFRangeMake (0, 11), &lr);
  PASS_CF(d != NULL && CFDictionaryGetCount (d) == 2,
          "The longest-range attributes at 8 are the second run's two.");
  PASS_CF(lr.location == 5 && lr.length == 6,
          "The full attribute set stays equal only over (5,6).");

  v = CFAttributedStringGetAttributeAndLongestEffectiveRange (
        m, 2, CFSTR ("shared"), CFRangeMake (0, 3), &lr);
  PASS_CF(lr.location == 0 && lr.length == 3,
          "The longest range is clamped to the search range.");

  CFRelease (m);

  return 0;
}
