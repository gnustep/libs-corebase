#import <Foundation/NSString.h>
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <string.h>

/* NSCFString lengthOfBytesUsingEncoding: and getCString:maxLength:encoding:. */

int main (void)
{
  NSString *s =
    (NSString *) CFStringCreateWithCString (NULL, "hello",
                                            kCFStringEncodingASCII);
  char buf[16];
  BOOL ok;

  PASS_CF([s lengthOfBytesUsingEncoding: NSASCIIStringEncoding] == 5,
    "lengthOfBytesUsingEncoding: returns the byte count.");

  ok = [s getCString: buf maxLength: sizeof(buf) encoding: NSASCIIStringEncoding];
  PASS_CF(ok == YES, "getCString:maxLength:encoding: succeeds for ASCII.");
  PASS_CF(strcmp(buf, "hello") == 0, "getCString fills the buffer correctly.");

  CFRelease ((CFStringRef) s);
  return 0;
}
