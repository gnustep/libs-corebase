#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <math.h>

int main (void)
{
  CFStringRef str1;
  
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%a %A %f %F %g %G"),
				   -INFINITY, INFINITY, INFINITY, -INFINITY,
				   -INFINITY, INFINITY);
  PASS_CFEQ (str1, CFSTR ("-inf INF inf -INF -inf INF"),
			  "Formatting infinity works.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%a %A %f %F %g %G"),
				   -NAN, NAN, NAN, -NAN, -NAN, NAN);
  PASS_CFEQ (str1, CFSTR ("-nan NAN nan -NAN -nan NAN"),
			  "Formatting not-a-number works.");
  CFRelease (str1);

  return 0;
}
