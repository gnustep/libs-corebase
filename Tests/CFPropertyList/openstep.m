#include "../CFTesting.h"
#include "CoreFoundation/CFPropertyList.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include <string.h>

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

  {
    /* A string longer than the internal 1024-character buffer is parsed
       from a separately allocated buffer; this must not read past the end
       of it or leak it. */
    UInt8 big[1200];
    CFDataRef bigData;
    CFPropertyListRef bigPlist;

    memset (big, 'a', 1100);
    bigData = CFDataCreate (NULL, big, 1100);
    bigPlist = CFPropertyListCreateWithData (NULL, bigData,
                                             kCFPropertyListImmutable, NULL,
                                             NULL);
    PASS_CF (bigPlist != NULL
             && CFGetTypeID (bigPlist) == CFStringGetTypeID (),
             "A string larger than the internal buffer parses.");
    if (bigPlist)
      CFRelease (bigPlist);
    CFRelease (bigData);
  }

  {
    /* A "//" comment with no trailing newline that fills the buffer must
       be stopped at the end of the data, not read past it. */
    UInt8 c[1200];
    CFDataRef cData;
    CFPropertyListRef cPlist;

    c[0] = '/';
    c[1] = '/';
    memset (c + 2, ' ', 1100);
    cData = CFDataCreate (NULL, c, 1102);
    cPlist = CFPropertyListCreateWithData (NULL, cData,
                                           kCFPropertyListImmutable, NULL,
                                           NULL);
    PASS_CF (cPlist == NULL,
             "A comment-only property list returns NULL without overreading.");
    if (cPlist)
      CFRelease (cPlist);
    CFRelease (cData);
  }

  return 0;
}
