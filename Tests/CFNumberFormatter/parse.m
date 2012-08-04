#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "../CFTesting.h"

int main (void)
{
  CFLocaleRef loc;
  CFStringRef str;
  CFNumberFormatterRef nf;
  SInt8 int_8;
  SInt16 int_16;
  SInt32 int_32;
  SInt64 int_64;
  double d;
  CFRange range;
  
  loc = CFLocaleCreate (NULL, CFSTR("en_US"));
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterNoStyle);
  str = CFSTR("16");
  PASS_CF(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt8Type, &int_8) == true, "Got SInt8 value");
  PASS_CF(int_8 == 16, "SInt8 value is '%d'", (int)int_8);
  
  str = CFSTR("300.0");
  PASS_CF(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt16Type, &int_16) == true, "Got SInt16 value");
  PASS_CF(int_16 == 300, "SInt16 value is '%d'", (int)int_16);
  CFRelease(nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterCurrencyStyle);
  str = CFSTR("$68456.50");
  PASS_CF(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt32Type, &int_32) == true, "Got SInt32 value");
  PASS_CF(int_32 == 68456, "SInt32 value is '%d'", (int)int_32);
  CFRelease (nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterScientificStyle);
  str = CFSTR("3.15E+8");
  PASS_CF(CFNumberFormatterGetValueFromString (nf, str, NULL,
    kCFNumberSInt64Type, &int_64) == true, "Got SInt64 value");
  PASS_CF(int_64 == 315000000, "SInt64 value is '%lld'", (long long)int_64);
  CFRelease (nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterPercentStyle);
  str = CFSTR("123.4%");
  range = CFRangeMake (1, 5);
  PASS_CF(CFNumberFormatterGetValueFromString (nf, str, &range,
    kCFNumberDoubleType, &d) == true, "Got double value");
  PASS_CF(d == 0.234, "Double value is '%g'", d);
  PASS_CF((range.location == 1 && range.length == 5) , "Parsed complete length");
  CFRelease (nf);
  
  return 0;
}

