#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

int main (void)
{
  signed int i32 = -5;
  signed short i16 = 6;
  signed char i8 = 7;
  float f = 3.4;
  Float64 f64;
  CFNumberRef num1, num2;
  
  num1 = CFNumberCreate (NULL, kCFNumberIntType, &i32);
  PASS_CF(num1 != NULL, "CFNumberCreate() returns non-NULL");
  PASS_CF(CFNumberGetType(num1) == kCFNumberSInt32Type,
    "Integer is converted to SInt32(%d).", (int)CFNumberGetType(num1));
  
  num2 = CFNumberCreate (NULL, kCFNumberCharType, &i8);
#ifdef __APPLE__
  PASS_CF(CFNumberGetType(num2) == kCFNumberSInt8Type,
    "Char is converted to Apple SInt8 type (%d).", (int)CFNumberGetType(num2));
#else
  PASS_CF(CFNumberGetType(num2) == kCFNumberSInt32Type,
    "Char is converted to SInt32(%d).", (int)CFNumberGetType(num2));
#endif
  
  PASS_CF(CFNumberGetValue(num1, kCFNumberSInt8Type, &i8),
    "Integer converted to SInt8.");
  PASS_CF(i8 == -5, "SInt32 converted to SInt8 (%d) correctly.", i8);
  
  PASS_CF(CFNumberGetValue(num2, kCFNumberSInt32Type, &i32),
    "Char converted to SInt32.");
  PASS_CF(i32 == 7, "SInt8 converted to SInt32 (%d)correctly.", i32);
  
  CFRelease (num1);
  CFRelease (num2);
  
  num1 = CFNumberCreate (NULL, kCFNumberShortType, &i16);
#ifdef __APPLE__
  PASS_CF(CFNumberGetType(num1) == kCFNumberSInt16Type,
    "Short is converted to Apple SInt16 type (%d).", (int)CFNumberGetType(num1));
#else
  PASS_CF(CFNumberGetType(num1) == kCFNumberSInt32Type,
    "Short is converted to SInt32(%d).", (int)CFNumberGetType(num1));
#endif
  
  num2 = CFNumberCreate (NULL, kCFNumberFloatType, &f);
#ifdef __APPLE__
  PASS_CF(CFNumberGetType(num2) == kCFNumberFloat32Type,
    "Float is converted to Apple Float32 type (%d).", (int)CFNumberGetType(num2));
#else
  PASS_CF(CFNumberGetType(num2) == kCFNumberFloat64Type,
    "Float is converted to Float64(%d).", (int)CFNumberGetType(num2));
#endif
  
  PASS_CF(CFNumberGetValue(num1, kCFNumberFloatType, &f),
    "Short converted to float.");
  PASS_CF(f == 6.0, "Short converted to float (%f) correctly.", f);
  
  /* Lossy conversion so returns false... */
  PASS_CF(CFNumberGetValue(num2, kCFNumberSInt16Type, &i16) == false,
    "Float converted to short.");
  PASS_CF(i16 == 3, "Float converted to short (%d) correctly.", i16);
  
  CFRelease (num1);
  CFRelease (num2);
  
  return 0;
}
