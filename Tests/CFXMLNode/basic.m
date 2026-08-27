#include <CoreFoundation/CFXMLNode.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFArray.h>

#include "../CFTesting.h"

int main (void)
{
  CFStringRef keys[1] = { CFSTR ("id") };
  CFStringRef vals[1] = { CFSTR ("1") };
  CFDictionaryRef attrs;
  CFArrayRef order;
  CFXMLNodeRef text;
  CFXMLNodeRef v2;
  CFXMLNodeRef comment;
  CFXMLNodeRef elem;
  CFXMLNodeRef elemCopy;
  CFXMLNodeRef pi;
  const CFXMLElementInfo *ei;
  const CFXMLProcessingInstructionInfo *pip;
  CFXMLElementInfo einfo;
  CFXMLProcessingInstructionInfo piinfo;

  text = CFXMLNodeCreate (NULL, kCFXMLNodeTypeText, CFSTR ("hello"), NULL,
                          kCFXMLNodeCurrentVersion);
  PASS_CF(text != NULL, "Created a text node.");
  PASS_CF(CFGetTypeID (text) == CFXMLNodeGetTypeID (),
          "A node has the CFXMLNode type ID.");
  PASS_CFEQ(CFXMLNodeGetString (text), CFSTR ("hello"),
            "The text node keeps its string.");
  PASS_CF(CFXMLNodeGetInfoPtr (text) == NULL,
          "A text node has no info pointer.");
  PASS_CF(CFXMLNodeGetVersion (text) == 1, "The node keeps its version.");

  v2 = CFXMLNodeCreate (NULL, kCFXMLNodeTypeText, CFSTR ("x"), NULL, 2);
  PASS_CF(CFXMLNodeGetVersion (v2) == 2,
          "The node keeps a non-default version.");
  CFRelease (v2);

  comment = CFXMLNodeCreate (NULL, kCFXMLNodeTypeComment, CFSTR ("cmt"), NULL,
                             1);
  PASS_CF(CFXMLNodeGetInfoPtr (comment) == NULL,
          "A comment node has no info pointer.");
  CFRelease (comment);

  attrs = CFDictionaryCreate (NULL, (const void **)keys, (const void **)vals, 1,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  order = CFArrayCreate (NULL, (const void **)keys, 1, &kCFTypeArrayCallBacks);
  einfo.attributes = attrs;
  einfo.attributeOrder = order;
  einfo.isEmpty = false;
  elem = CFXMLNodeCreate (NULL, kCFXMLNodeTypeElement, CFSTR ("tag"), &einfo, 1);
  PASS_CFEQ(CFXMLNodeGetString (elem), CFSTR ("tag"),
            "The element node keeps its name.");
  ei = (const CFXMLElementInfo *)CFXMLNodeGetInfoPtr (elem);
  PASS_CF(ei != NULL, "An element node has an info pointer.");
  PASS_CF(ei != NULL && ei->attributes != NULL
          && CFDictionaryGetCount (ei->attributes) == 1,
          "The element keeps its attributes.");
  PASS_CF(ei != NULL && ei->attributes != attrs,
          "The element copies the attributes dictionary.");
  PASS_CF(ei != NULL && ei->attributeOrder != NULL
          && CFArrayGetCount (ei->attributeOrder) == 1,
          "The element keeps its attribute order.");
  PASS_CF(ei != NULL && ei->isEmpty == false,
          "The element keeps its isEmpty flag.");

  elemCopy = CFXMLNodeCreateCopy (NULL, elem);
  PASS_CF(CFEqual (elem, elemCopy), "An element node equals its copy.");
  CFRelease (elemCopy);

  piinfo.dataString = CFSTR ("pidata");
  pi = CFXMLNodeCreate (NULL, kCFXMLNodeTypeProcessingInstruction,
                        CFSTR ("pitarget"), &piinfo, 1);
  pip = (const CFXMLProcessingInstructionInfo *)CFXMLNodeGetInfoPtr (pi);
  PASS_CF(pip != NULL, "A processing instruction has an info pointer.");
  PASS_CFEQ(pip->dataString, CFSTR ("pidata"),
            "The processing instruction keeps its data string.");
  CFRelease (pi);

  CFRelease (elem);
  CFRelease (attrs);
  CFRelease (order);
  CFRelease (text);

  return 0;
}
