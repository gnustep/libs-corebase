#include "CoreFoundation/CFAttributedString.h"
#include "../CFTesting.h"

int main (void)
{
  const void *keys[3];
  const void *values[3];
  CFDictionaryRef attrib1;
  CFDictionaryRef attrib2;
  CFDictionaryRef attrib3;
  CFDictionaryRef curAttrib;
  CFAttributedStringRef str;
  CFMutableAttributedStringRef mstr;
  CFRange r;
  
  keys[0] = CFSTR("Attrib 1");
  values[0] = CFSTR("Attribute");
  attrib1 = CFDictionaryCreate (NULL, keys, values, 1,
                                &kCFCopyStringDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
  str = CFAttributedStringCreate (NULL, CFSTR("This is a string!"),
                                  (CFDictionaryRef)attrib1);
  
  mstr = CFAttributedStringCreateMutableCopy (NULL, 0, str);
  PASS_CF(mstr != NULL, "Mutable attributed string was created.");
  
  curAttrib = CFAttributedStringGetAttributes (str, 10, &r);
  PASS_CF(r.location == 0 && r.length == 17, "Attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  PASS_CFEQ (curAttrib, attrib1, "First set of attributes are the same.");
  
  /* This will be like a wedge between the previous attribute */
  keys[1] = CFSTR("Attrib 2");
  values[1] = CFSTR("Another Attribute");
  attrib2 = CFDictionaryCreateMutable (NULL, 2,
                                &kCFCopyStringDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
  CFDictionaryAddValue (attrib2, keys[1], values[1]);
  CFAttributedStringSetAttributes (mstr, CFRangeMake (5, 10), attrib2, false);
  CFDictionaryAddValue (attrib2, keys[0], values[0]);
  
  curAttrib = CFAttributedStringGetAttributes (mstr, 10, &r);
  PASS_CF(r.location == 5 && r.length == 10, "Attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  PASS_CFEQ (curAttrib, attrib2, "Second set of attributes are the same.");
  curAttrib = CFAttributedStringGetAttributes (mstr, 16, &r);
  PASS_CF(r.location == 15 && r.length == 2, "Attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  PASS_CFEQ (curAttrib, attrib1, "Previous attributes are adjusted.");
  
  /* This will insert a new attribute and delete the last one */
  keys[0] = CFSTR("Attrib 3");
  values[0] = CFSTR("attribute #3");
  attrib3 = CFDictionaryCreate (NULL, keys, values, 1,
                                &kCFCopyStringDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
  CFAttributedStringSetAttributes (mstr, CFRangeMake (10, 7), attrib3, true);
  curAttrib = CFAttributedStringGetAttributes (mstr, 11, &r);
  PASS_CF(r.location == 10 && r.length == 7, "Attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  PASS_CFEQ (curAttrib, attrib3, "Third set of attributes are the same.");
  
  
  
  CFRelease (attrib1);
  CFRelease (attrib2);
  CFRelease (attrib3);
  CFRelease (mstr);
  CFRelease (str);
  
  return 0;
}

