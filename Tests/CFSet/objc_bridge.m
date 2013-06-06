#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#include "CoreFoundation/CFSet.h"
#include "../CFTesting.h"

int main (void)
{
  NSSet* nsset = [[NSSet alloc] initWithObjects: @"Hello", @"world", @"!", nil];
  CFSetRef copy;

  PASS_CF ([nsset count] == CFSetGetCount((CFSetRef) nsset),
    "CFSetGetCount works with an NSSet instance");

  PASS_CF ([nsset member: @"world"] == CFSetGetValue((CFSetRef) nsset, @"world"),
    "CFSetGetValue works with an NSSet instance");

  copy = CFSetCreateCopy(NULL, (CFSetRef) nsset);

  PASS_CF ([nsset count] == CFSetGetCount(copy),
    "CFSetCreateCopy resulting count matches");

  CFRelease((CFSetRef) nsset);
  CFRelease(copy);

  return 0;
}


