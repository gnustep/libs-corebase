#include "CoreFoundation/CFAttributedString.h"
#include "../CFTesting.h"

int main (void)
{
  CFAttributedStringRef str;
  CFDictionaryRef attribs;
  CFDictionaryRef attrOut;
  CFRange r;
  
  attribs = CFDictionaryCreateMutable (NULL, 0,
                                       &kCFCopyStringDictionaryKeyCallBacks,
                                       &kCFTypeDictionaryValueCallBacks);
  CFDictionaryAddValue (attribs, CFSTR("Attrib 1"), CFSTR("Attribute"));
  CFDictionaryAddValue (attribs, CFSTR("Attrib 2"), CFSTR("Attribute2"));
  
  str = CFAttributedStringCreate (NULL, CFSTR("This is a string!"), attribs);
  PASS (str != NULL
        && CFEqual(CFAttributedStringGetString(str), CFSTR("This is a string!"))
        && CFAttributedStringGetLength(str) == 17,
        "Attributed string was correctly created.");
  
  attrOut = CFAttributedStringGetAttributes (str, 2, &r);
  PASS (r.location == 0 && r.length == 17, "Attribute range is (%d, %d).",
        (int)r.location, (int)r.length);
  
  CFRelease (str);
  
  return 0;
}