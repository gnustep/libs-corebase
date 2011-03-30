#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "../CFTesting.h"

int main (void)
{
  CFNumberRef num;
  CFLocaleRef loc;
  CFStringRef str;
  CFNumberFormatterRef nf;
  SInt8 int_8;
  SInt16 int_16;
  SInt32 int_32;
  SInt64 int_64;
  int    sint;
  double d;
  CFRange range;
  
  loc = CFLocaleCreate (NULL, CFSTR("en_US"));
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterNoStyle);
  str = CFSTR("16");
  PASS(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt8Type, &int_8) == true, "Got SInt8 value");
  PASS(int_8 == 16, "SInt8 value is '16'");
  
  str = CFSTR("300.0");
  PASS(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt16Type, &int_16) == true, "Got SInt16 value");
  PASS(int_16 == 300, "SInt16 value is '300'");
  CFRelease(nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterCurrencyStyle);
  str = CFSTR("$68456.50");
  PASS(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt32Type, &int_32) == true, "Got SInt32 value");
  PASS(int_32 == 68456, "SInt32 value is '68456'");
  CFRelease (nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterScientificStyle);
  str = CFSTR("3.15e8");
  PASS(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt64Type, &int_64) == true, "Got SInt64 value");
  PASS(int_64 == 315000000, "SInt64 value is '315000000'");
  CFRelease (nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterPercentStyle);
  str = CFSTR("123.4%");
  range = CFRangeMake (1, 5);
  PASS(CFNumberFormatterGetValueFromString (nf, str, &range,
    kCFNumberDoubleType, &d) == true, "Got double value");
  PASS(d == .234, "Double value is '.234'");
  PASS((range.location == 1 && range.length == 5) , "Parsed complete length");
  CFRelease (nf);
  
  return 0;
}
