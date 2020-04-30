#import <Foundation/NSValue.h>
#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

static signed int sints[] = {INT_MIN, INT_MAX, 0, -1, 1};
static signed short sshorts[] = {SHRT_MIN, SHRT_MAX, 0, -1, 1};
static signed char schars[] = {CHAR_MIN, CHAR_MAX, 0, -1, 1};
static unsigned int uints[] = {UINT_MAX, 0, 1};
static unsigned short ushorts[] = {USHRT_MAX, 0, 1};
static unsigned char uchars[] = {UCHAR_MAX, 0, 1};
static float floats[] = {FLT_MIN, FLT_MAX, FLT_EPSILON, 0, -3.4, 3.4};
static double doubles[] = {DBL_MIN, DBL_MAX, DBL_EPSILON, 0, -3.4, 3.4};

void testNSonCF(void);
void testCFonNS(void);

int main(void)
{
  testNSonCF();
  testCFonNS();
  return 0;
}

void testNSonCF(void)
{
  int i;
  
  for (i = 0; i < sizeof(sints)/sizeof(*sints); i++) {
    signed int in = sints[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberIntType, &in);
    signed int out = [(NSNumber*)cfnum intValue];
    PASS_CF(in == out,
      "-intValue = %d works on CFNumber(kCFNumberIntType) %d", out, in);
  }
  
  for (i = 0; i < sizeof(sshorts)/sizeof(*sshorts); i++) {
    signed short in = sshorts[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberShortType, &in);
    signed short out = [(NSNumber*)cfnum shortValue];
    PASS_CF(in == out,
      "-shortValue = %d works on CFNumber(kCFNumberShortType) %d", out, in);
  }
  
  for (i = 0; i < sizeof(schars)/sizeof(*schars); i++) {
    signed char in = schars[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberCharType, &in);
    signed char out = [(NSNumber*)cfnum charValue];
    PASS_CF(in == out,
      "-charValue = %d works on CFNumber(kCFNumberCharType) %d", out, in);
  }
  
  for (i = 0; i < sizeof(uints)/sizeof(*uints); i++) {
    unsigned int in = uints[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberIntType, &in);
    unsigned int out = [(NSNumber*)cfnum unsignedIntValue];
    PASS_CF(in == out,
      "-unsignedIntValue = %u works on CFNumber(kCFNumberIntType) %u", out, in);
  }
  
  for (i = 0; i < sizeof(ushorts)/sizeof(*ushorts); i++) {
    unsigned short in = ushorts[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberShortType, &in);
    unsigned short out = [(NSNumber*)cfnum unsignedShortValue];
    PASS_CF(in == out,
      "-unsignedShortValue = %u works on CFNumber(kCFNumberShortType) %u", out, in);
  }
  
  for (i = 0; i < sizeof(uchars)/sizeof(*uchars); i++) {
    unsigned char in = uchars[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberCharType, &in);
    unsigned char out = [(NSNumber*)cfnum unsignedCharValue];
    PASS_CF(in == out,
      "-unsignedCharValue = %u works on CFNumber(kCFNumberCharType) %u", out, in);
  }
  
  for (i = 0; i < sizeof(floats)/sizeof(*floats); i++) {
    float in = floats[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberFloatType, &in);
    float out = [(NSNumber*)cfnum floatValue];
    PASS_CF(in == out,
      "-floatValue = %f works on CFNumber(kCFNumberFloatType) %f", out, in);
  }
  
  for (i = 0; i < sizeof(doubles)/sizeof(*doubles); i++) {
    double in = doubles[i];
    CFNumberRef cfnum = CFNumberCreate(NULL, kCFNumberDoubleType, &in);
    double out = [(NSNumber*)cfnum doubleValue];
    PASS_CF(in == out,
      "-doubleValue = %lf works on CFNumber(kCFNumberDoubleType) %lf", out, in);
  }
}

void testCFonNS(void)
{
  int i;
  
  for (i = 0; i < sizeof(sints)/sizeof(*sints); i++) {
    signed int in = sints[i];
    signed int out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithInt:in],
      kCFNumberIntType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberIntType) = %d works on signed NSNumber %d",
      out, in);
  }

  for (i = 0; i < sizeof(sshorts)/sizeof(*sshorts); i++) {
    signed short in = sshorts[i];
    signed short out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithShort:in],
      kCFNumberShortType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberShortType) = %d works on signed NSNumber %d",
      out, in);
  }

  for (i = 0; i < sizeof(schars)/sizeof(*schars); i++) {
    signed char in = schars[i];
    signed char out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithChar:in],
      kCFNumberCharType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberCharType) = %d works on signed NSNumber %d",
      out, in);
  }

  for (i = 0; i < sizeof(uints)/sizeof(*uints); i++) {
    unsigned int in = uints[i];
    unsigned int out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithUnsignedInt:in],
      kCFNumberIntType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberIntType) = %u works on unsigned NSNumber %u",
      out, in);
  }

  for (i = 0; i < sizeof(ushorts)/sizeof(*ushorts); i++) {
    unsigned short in = ushorts[i];
    unsigned short out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithUnsignedShort:in],
      kCFNumberShortType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberShortType) = %u works on unsigned NSNumber %u",
      out, in);
  }

  for (i = 0; i < sizeof(uchars)/sizeof(*uchars); i++) {
    unsigned char in = uchars[i];
    unsigned char out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithUnsignedChar:in],
      kCFNumberCharType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberCharType) = %u works on unsigned NSNumber %u",
      out, in);
  }
  
  for (i = 0; i < sizeof(floats)/sizeof(*floats); i++) {
    float in = floats[i];
    float out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithFloat:in],
      kCFNumberFloatType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberFloatType) = %f works on NSNumber %f",
      out, in);
  }
  
  for (i = 0; i < sizeof(doubles)/sizeof(*doubles); i++) {
    double in = doubles[i];
    double out;
    CFNumberGetValue((CFNumberRef)[NSNumber numberWithDouble:in],
      kCFNumberDoubleType, &out);
    PASS_CF(in == out,
      "CFNumberGetValue(kCFNumberDoubleType) = %lf works on NSNumber %lf",
      out, in);
  }
}

