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
  double d;
  
  loc = CFLocaleCreate (NULL, CFSTR("en_US"));
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterNoStyle);
  int_8 = 16;
  str = CFNumberFormatterCreateStringWithValue (NULL, nf,
    kCFNumberSInt8Type, &int_8);
  PASS_CFEQ(str, CFSTR("16"), "SInt8 formatted correctly");
  CFRelease (str);
  
  num = CFNumberCreate (NULL, kCFNumberSInt8Type, &int_8);
  CFNumberFormatterSetProperty (nf, kCFNumberFormatterFormatWidth, num);
  str = CFNumberFormatterCreateStringWithValue (NULL, nf,
    kCFNumberSInt8Type, &int_8);
  PASS_CFEQ(str, CFSTR("**************16"), "SInt8 formatted correctly");
  CFRelease(nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterCurrencyStyle);
  int_16 = 300;
  PASS_CFEQ(CFNumberFormatterCreateStringWithValue (NULL, nf,
    kCFNumberSInt16Type, &int_16), CFSTR("$300.00"),
    "SInt16 formatted correctly");
  CFRelease(nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterScientificStyle);
  int_32 = 68456;
  PASS_CFEQ(CFNumberFormatterCreateStringWithValue (NULL, nf,
    kCFNumberSInt32Type, &int_32), CFSTR("6.8456E4"),
    "SInt32 formatted correctly");
  CFRelease(nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterDecimalStyle);
  int_64 = 315000000;
  PASS_CFEQ(CFNumberFormatterCreateStringWithValue (NULL, nf,
    kCFNumberSInt64Type, &int_64), CFSTR("315,000,000"),
    "SInt64 formatted correctly");
  CFRelease(nf);
  
  nf = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterPercentStyle);
  d = 1.234;
  PASS_CFEQ(CFNumberFormatterCreateStringWithValue (NULL, nf,
    kCFNumberDoubleType, &d), CFSTR("123%"),
    "Double formatted correctly");
  
  return 0;
}

