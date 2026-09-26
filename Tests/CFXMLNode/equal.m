#include <CoreFoundation/CFXMLNode.h>
#include <CoreFoundation/CFString.h>

#include "../CFTesting.h"

int main (void)
{
  CFXMLNodeRef t1;
  CFXMLNodeRef t2;
  CFXMLNodeRef t3;
  CFXMLNodeRef c1;
  CFXMLNodeRef c2;

  t1 = CFXMLNodeCreate (NULL, kCFXMLNodeTypeText, CFSTR ("hello"), NULL, 1);
  t2 = CFXMLNodeCreateCopy (NULL, t1);
  PASS_CF(CFEqual (t1, t2), "Two identical text nodes are equal.");

  t3 = CFXMLNodeCreate (NULL, kCFXMLNodeTypeText, CFSTR ("world"), NULL, 1);
  PASS_CF(!CFEqual (t1, t3), "Text nodes with different strings are not equal.");

  c1 = CFXMLNodeCreate (NULL, kCFXMLNodeTypeComment, CFSTR ("note"), NULL, 1);
  c2 = CFXMLNodeCreateCopy (NULL, c1);
  PASS_CF(CFEqual (c1, c2), "Two identical comment nodes are equal.");

  PASS_CF(!CFEqual (t1, c1),
          "A text node and a comment node are not equal.");

  CFRelease (t1);
  CFRelease (t2);
  CFRelease (t3);
  CFRelease (c1);
  CFRelease (c2);

  return 0;
}
