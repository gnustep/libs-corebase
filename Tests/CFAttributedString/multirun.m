#include "CoreFoundation/CFAttributedString.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFDictionary.h"

#include "../CFTesting.h"

int
main (void)
{
  CFRange er;
  CFDictionaryRef d;
  CFMutableAttributedStringRef m;

  m = CFAttributedStringCreateMutable (NULL, 0);
  CFAttributedStringReplaceString (m, CFRangeMake (0, 0), CFSTR ("Hello World"));

  /* Two runs: (0,5) = {color}, (5,11) = {weight}. */
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 5), CFSTR ("color"),
                                  CFSTR ("red"));
  CFAttributedStringSetAttribute (m, CFRangeMake (5, 6), CFSTR ("weight"),
                                  CFSTR ("bold"));

  d = CFAttributedStringGetAttributes (m, 2, &er);
  PASS_CF(d != NULL && CFDictionaryGetCount (d) == 1
          && er.location == 0 && er.length == 5,
          "The first run's attributes cover (0,5).");
  PASS_CFEQ(CFAttributedStringGetAttribute (m, 2, CFSTR ("color"), NULL),
            CFSTR ("red"), "The first run has the color attribute.");
  d = CFAttributedStringGetAttributes (m, 8, &er);
  PASS_CF(d != NULL && CFDictionaryGetCount (d) == 1
          && er.location == 5 && er.length == 6,
          "The second run's attributes cover (5,6).");
  PASS_CFEQ(CFAttributedStringGetAttribute (m, 8, CFSTR ("weight"), NULL),
            CFSTR ("bold"), "The second run has the weight attribute.");
  PASS_CF(CFAttributedStringGetAttribute (m, 2, CFSTR ("weight"), NULL) == NULL,
          "weight is absent from the first run.");
  PASS_CF(CFAttributedStringGetAttribute (m, 8, CFSTR ("color"), NULL) == NULL,
          "color is absent from the second run.");
  d = CFAttributedStringGetAttributes (m, 5, &er);
  PASS_CF(er.location == 5 && er.length == 6,
          "A boundary index resolves to the second run.");

  /* Setting an attribute over both runs keeps each run's own attributes. */
  CFAttributedStringSetAttribute (m, CFRangeMake (0, 11), CFSTR ("lang"),
                                  CFSTR ("en"));
  d = CFAttributedStringGetAttributes (m, 2, &er);
  PASS_CF(CFDictionaryGetCount (d) == 2
          && CFDictionaryGetValue (d, CFSTR ("color")) != NULL
          && CFDictionaryGetValue (d, CFSTR ("lang")) != NULL
          && er.location == 0 && er.length == 5,
          "A spanning attribute merges into the first run.");
  d = CFAttributedStringGetAttributes (m, 8, &er);
  PASS_CF(CFDictionaryGetCount (d) == 2
          && CFDictionaryGetValue (d, CFSTR ("weight")) != NULL
          && CFDictionaryGetValue (d, CFSTR ("lang")) != NULL
          && er.location == 5 && er.length == 6,
          "A spanning attribute merges into the second run.");
  CFRelease (m);

  return 0;
}
