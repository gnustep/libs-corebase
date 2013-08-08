#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#import <Foundation/NSString.h>
#include <string.h>

int main (void)
{
  // Letter 'Å'
  UniChar notNormalized[] = { 0x212B, 0 };
  UniChar normalized[] = { 0x0041, 0x030A, 0 };
  CFMutableStringRef mut;
  UniChar* processed;
  NSMutableString* nsmut;

  mut = CFStringCreateMutable (NULL, 0);
  CFStringAppendCharacters (mut, notNormalized, 1);

  CFStringNormalize(mut, kCFStringNormalizationFormD);
  processed = CFStringGetCharactersPtr(mut);

  printf("first char: %04x\n", *processed);

  PASS_CF (memcmp(processed, normalized, 4) == 0,
    "CFStringNormalize works");
  CFRelease(mut);

  nsmut = [[NSString stringWithCharacters: notNormalized
                                   length: 1] mutableCopy];
  CFStringNormalize(nsmut, kCFStringNormalizationFormD);

  PASS_CF ([nsmut length] == 2, "Correct char count");
  PASS_CF ([nsmut characterAtIndex: 0] == normalized[0],
    "CFStringNormalize works on an NSString");

  [nsmut release];

  return 0;
}

