#include "CoreFoundation/CFAttributedString.h"
#include "../CFTesting.h"

int main (void)
{
  CFAttributedStringRef str;
  CFAttributedStringRef str2;
  CFMutableDictionaryRef attribs;
  CFDictionaryRef attrOut;
  CFRange r;
  
  attribs = CFDictionaryCreateMutable (NULL, 0,
                                       &kCFCopyStringDictionaryKeyCallBacks,
                                       &kCFTypeDictionaryValueCallBacks);
  CFDictionaryAddValue (attribs, CFSTR("Attrib 1"), CFSTR("Attribute"));
  CFDictionaryAddValue (attribs, CFSTR("Attrib 2"), CFSTR("Attribute2"));
  
  str = CFAttributedStringCreate (NULL, CFSTR("This is a string!"),
                                  (CFDictionaryRef)attribs);
  PASS_CF(str != NULL
        && CFEqual(CFAttributedStringGetString(str), CFSTR("This is a string!"))
        && CFAttributedStringGetLength(str) == 17,
        "Attributed string was correctly created.");
  
  attrOut = CFAttributedStringGetAttributes (str, 2, &r);
  PASS_CF(r.location == 0 && r.length == 17, "Attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  PASS_CFEQ (attrOut, attribs, "Attributes are the same.");
  
  str2 = CFAttributedStringCreateCopy (NULL, str);
  PASS_CFEQ (str, str2, "Copied attributed string is equal.");
  
  attrOut = CFAttributedStringGetAttributes (str, 10, &r);
  PASS_CF(r.location == 0 && r.length == 17, "Copied attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  PASS_CFEQ (attrOut, attribs, "Copied attributes are the same.");
  
  CFRelease (attribs);
  CFRelease (str);
  //CFRelease (str2);
  
  return 0;
}
