#include <CoreFoundation/CFXMLNode.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFArray.h>

#include "../CFTesting.h"

int main (void)
{
  CFXMLNodeRef text;
  CFXMLNodeRef comment;
  CFXMLNodeRef elem;
  CFXMLNodeRef pi;
  CFXMLNodeRef copy;
  CFDictionaryRef attrs;
  CFArrayRef order;
  CFXMLElementInfo einfo;
  CFXMLProcessingInstructionInfo piinfo;

  text = CFXMLNodeCreate (NULL, kCFXMLNodeTypeText, CFSTR ("t"), NULL, 1);
  PASS_CF(CFXMLNodeGetTypeCode (text) == kCFXMLNodeTypeText,
          "A text node reports the text type code.");

  comment = CFXMLNodeCreate (NULL, kCFXMLNodeTypeComment, CFSTR ("c"), NULL, 1);
  PASS_CF(CFXMLNodeGetTypeCode (comment) == kCFXMLNodeTypeComment,
          "A comment node reports the comment type code.");

  attrs = CFDictionaryCreate (NULL, NULL, NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  order = CFArrayCreate (NULL, NULL, 0, &kCFTypeArrayCallBacks);
  einfo.attributes = attrs;
  einfo.attributeOrder = order;
  einfo.isEmpty = true;
  elem = CFXMLNodeCreate (NULL, kCFXMLNodeTypeElement, CFSTR ("e"), &einfo, 1);
  PASS_CF(CFXMLNodeGetTypeCode (elem) == kCFXMLNodeTypeElement,
          "An element node reports the element type code.");

  piinfo.dataString = CFSTR ("d");
  pi = CFXMLNodeCreate (NULL, kCFXMLNodeTypeProcessingInstruction,
                        CFSTR ("p"), &piinfo, 1);
  PASS_CF(CFXMLNodeGetTypeCode (pi) == kCFXMLNodeTypeProcessingInstruction,
          "A processing instruction reports the processing instruction type code.");

  copy = CFXMLNodeCreateCopy (NULL, text);
  PASS_CF(CFXMLNodeGetTypeCode (copy) == kCFXMLNodeTypeText,
          "A copied node preserves its type code.");

  CFRelease (text);
  CFRelease (comment);
  CFRelease (elem);
  CFRelease (attrs);
  CFRelease (order);
  CFRelease (pi);
  CFRelease (copy);

  return 0;
}
