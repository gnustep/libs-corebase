#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include "../CFTesting.h"

const UInt8 rawdata[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
const SInt32 i32 = 10;

int main (void)
{
  CFMutableDictionaryRef dict;
  CFMutableArrayRef array;
  CFNumberRef num;
  CFDateRef date;
  CFDataRef data;
  
  date = CFDateCreate (NULL, 1.0);
  data = CFDataCreate (NULL, rawdata, sizeof(rawdata));
  num = CFNumberCreate (NULL, kCFNumberSInt32Type, &i32);
  dict = CFDictionaryCreateMutable (NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                    &kCFTypeDictionaryValueCallBacks);
  CFDictionaryAddValue (dict, date, data);
  CFDictionaryAddValue (dict, num, date);
  
  PASS_CF(CFPropertyListIsValid (dict, kCFPropertyListXMLFormat_v1_0) == false,
          "Dictionary with non-CFString keys is not a valid property list.");
  
  CFDictionaryRemoveValue (dict, date);
  CFDictionaryRemoveValue (dict, num);
  CFDictionaryAddValue (dict, CFSTR("data"), data);
  CFDictionaryAddValue (dict, CFSTR("date"), date);
  CFDictionaryAddValue (dict, CFSTR("number"), num);
  
  PASS_CF(CFPropertyListIsValid (dict, kCFPropertyListXMLFormat_v1_0),
          "Dictionary with CFString keys is a valid property list.");
  PASS_CF(CFPropertyListIsValid (dict, kCFPropertyListOpenStepFormat) == false,
          "Dictionary with CFDate and CFNumber values is not a valid OpenStep"
          " property list.");
  
  CFDictionaryRemoveValue (dict, CFSTR("date"));
  CFDictionaryRemoveValue (dict, CFSTR("number"));
  
  PASS_CF(CFPropertyListIsValid (dict, kCFPropertyListOpenStepFormat),
          "Dictionary with CFData only is a valid OpenStep property list.");
  
  array =  CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
  CFArrayAppendValue (array, (const void*)date);
  CFArrayAppendValue (array, (const void*)data);
  CFArrayAppendValue (array, (const void*)num);
  CFArrayAppendValue (array, (const void*)dict);
  
  PASS_CF(CFPropertyListIsValid (array, kCFPropertyListXMLFormat_v1_0),
          "Array with CFType objects is a valid XML property list.");
  
  CFDictionaryAddValue (dict, CFSTR("array"), array);
  PASS_CF(CFPropertyListIsValid (dict, kCFPropertyListXMLFormat_v1_0) == false,
          "Property list with recursion is not a valid property list.");
  
  CFRelease (dict);
  CFRelease (array);
  CFRelease (data);
  CFRelease (date);
  CFRelease (num);
  
  return 0;
}
