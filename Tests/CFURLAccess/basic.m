#include "CoreFoundation/CFURLAccess.h"
#include "../CFTesting.h"
#include <string.h>

const char *test = "test data!\n";

int main (void)
{
  const char *bytes;
  CFURLRef url;
  CFDataRef data;
  CFDictionaryRef dict;
  CFArrayRef props;
  
  url = CFURLCreateWithFileSystemPath (NULL, CFSTR("test.txt"),
    kCFURLPOSIXPathStyle, false);
  data = CFDataCreate (NULL, (const UInt8*)test, strlen(test));
  
  PASS (CFURLWriteDataAndPropertiesToResource (url, data, NULL, NULL),
    "Data was successfully written to test file.");
  
  CFRelease (data);
  
  PASS (CFURLCreateDataAndPropertiesFromResource (NULL, url, &data, NULL, NULL,
    NULL), "File was successfully read.");
  
  bytes = CFDataGetBytePtr (data);
  PASS (strncmp (bytes, test, strlen(test)) == 0,
    "Content read is the same the content that was written.");
  
  CFRelease (data);
  
  PASS (CFURLDestroyResource(url, NULL), "File was successfully deleted.");
  
  CFRelease (url);
  
  return 0;
}