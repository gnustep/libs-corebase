#include "../CFTesting.h"
#include "CoreFoundation/CFPropertyList.h"
#include "CoreFoundation/CFDictionary.h"

const UInt8 data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

const UInt8 proplist[] = "\357\273\277{\n"
  "\tstring = \"Quotable String.\";\n"
  "\tdata = <0102030405060708090A0B0C0D0E0F>;\n"
  "\tarray = (\n"
  "\t\t<0102030405060708090A0B0C0D0E0F>,\n"
  "\t\t\"Quotable String.\"\n"
  "\t);\n"
  "}\n";

int main (void)
{
  CFPropertyListRef plist;
  CFPropertyListRef readPlist;
  CFDataRef plData;
  CFErrorRef err;
  CFPropertyListFormat fmt;
  CFIndex count;
  const void *keys[3];
  const void *values[3];

  keys[0] = CFSTR ("data");
  keys[1] = CFSTR ("string");
  keys[2] = CFSTR ("array");
  values[0] = CFDataCreate (NULL, data, 15);
  values[1] = CFSTR ("Quotable String.");
  values[2] = CFArrayCreate (NULL, values, 2, &kCFTypeArrayCallBacks);

  plist = CFDictionaryCreate (NULL, keys, values, 3,
                              &kCFCopyStringDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);

#if GNUSTEP
  plData = CFPropertyListCreateData (NULL, plist,
                                     kCFPropertyListOpenStepFormat, 0, NULL);
  PASS_CF (memcmp (CFDataGetBytePtr (plData), proplist,
                   CFDataGetLength (plData)) == 0,
           "OpenStep property list written correctly.");
#endif

  plData = CFDataCreate (NULL, proplist, sizeof (proplist) -1);
  fmt = 0;
  err = NULL;
  readPlist = CFPropertyListCreateWithData (NULL, plData,
                                            kCFPropertyListImmutable, &fmt,
                                            &err);
  PASS_CF (err == NULL, "No errors encountered parsing valid property list.");

  count = CFDictionaryGetCount (readPlist);
  PASS_CF (count == 3,
           "Correct number of objects parsed into OpenStep property list.");

  PASS_CF (CFPropertyListIsValid (readPlist, kCFPropertyListOpenStepFormat),
           "Valid OpenStep property list returned.");

  PASS_CFEQ (readPlist, plist, "OpenStep property list read correctly.");

  CFRelease (plData);
  CFRelease (readPlist);
  CFRelease (plist);

  return 0;
}
