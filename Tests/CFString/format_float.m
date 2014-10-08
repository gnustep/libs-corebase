#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%a %A %f %F %g %G"),
				   -1.0/0.0, 1.0/0.0, 1.0/0.0, -1.0/0.0,
				   -1.0/0.0, 1.0/0.0);
  PASS_CFEQ (str1, CFSTR ("-inf INF inf -INF -inf INF"),
			  "Formatting infinity works.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%a %A %f %F %g %G"),
				   -(0.0/0.0), 0.0/0.0, 0.0/0.0, -(0.0/0.0),
				   -(0.0/0.0), 0.0/0.0);
  PASS_CFEQ (str1, CFSTR ("-nan NAN nan -NAN -nan NAN"),
			  "Formatting not-a-number works.");
  CFRelease (str1);

  return 0;
}
