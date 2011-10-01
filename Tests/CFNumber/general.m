#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

int main (void)
{
  int i32 = 5;
  short i16 = 6;
  char i8 = 7;
  float f = 3.4;
  Float64 f64;
  CFNumberRef num1, num2;
  
  num1 = CFNumberCreate (NULL, kCFNumberIntType, &i32);
  PASS(num1 != NULL, "CFNumberCreate() returns non-NULL");
  PASS(CFNumberGetType(num1) == kCFNumberSInt32Type,
    "Integer is converted to SInt32.");
  
  num2 = CFNumberCreate (NULL, kCFNumberCharType, &i8);
  PASS(CFNumberGetType(num1) == kCFNumberSInt32Type,
    "Char is converted to SInt32.");
  
  PASS(CFNumberGetValue(num1, kCFNumberSInt8Type, &i8),
    "SInt32 converted to SInt8.");
  PASS(i8 == 5, "SInt32 converted to SInt8 correctly.");
  
  PASS(CFNumberGetValue(num2, kCFNumberSInt32Type, &i32),
    "SInt8 converted to SInt32.");
  PASS(i32 == 7, "SInt8 converted to SInt32 correctly.");
  
  CFRelease (num1);
  CFRelease (num2);
  
  return 0;
}
