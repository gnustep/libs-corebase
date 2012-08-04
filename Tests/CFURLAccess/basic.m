#include "CoreFoundation/CFURLAccess.h"
#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"
#include <string.h>

const char *test = "test data!\n";

int main (void)
{
  const char *bytes;
  CFURLRef dir;
  CFURLRef file1;
  CFURLRef file2;
  CFDataRef data;
  CFDictionaryRef dict;
  CFArrayRef contents;
  CFNumberRef num;
  CFIndex fileLength;
  CFIndex count;
  
  dir = CFURLCreateWithFileSystemPath (NULL, CFSTR("TestDir"),
    kCFURLPOSIXPathStyle, true);
  PASS_CF(CFURLWriteDataAndPropertiesToResource (dir, NULL, NULL, NULL),
    "Directory was successfully created.");
  
  file1 = CFURLCreateWithFileSystemPath (NULL, CFSTR("TestDir/file1.txt"),
    kCFURLPOSIXPathStyle, false);
  data = CFDataCreate (NULL, (const UInt8*)test, strlen(test));
  
  PASS_CF(CFURLWriteDataAndPropertiesToResource (file1, data, NULL, NULL),
    "Data was successfully written to test file.");
  
  CFRelease (data);
  
  file2 = CFURLCreateWithFileSystemPath (NULL, CFSTR("TestDir/file2.txt"),
    kCFURLPOSIXPathStyle, false);
  data = CFDataCreate (NULL, NULL, 0);
  
  PASS_CF(CFURLWriteDataAndPropertiesToResource (file2, data, NULL, NULL),
    "Empty file was successfully created.");
  
  CFRelease (data);
  
  /* Lets try to delete the directory with files inside, this should fail. */
  PASS_CF(!CFURLDestroyResource(dir, NULL), "Could not delete directory.");
  
  PASS_CF(CFURLCreateDataAndPropertiesFromResource (NULL, file1, &data, &dict,
    NULL, NULL), "File was successfully read.");
  num = CFDictionaryGetValue (dict, kCFURLFileLength);
  CFNumberGetValue (num, kCFNumberCFIndexType, &fileLength);
  PASS_CF(fileLength == 11, "Properties correctly read.");
  
  CFRelease (dict);
  
  PASS_CF(CFURLCreateDataAndPropertiesFromResource (NULL, dir, NULL, &dict,
    NULL, NULL), "Directory was successfully read.");
  contents = CFDictionaryGetValue (dict, kCFURLFileDirectoryContents);
  count = CFArrayGetCount (contents);
  PASS_CF(count == 2, "There are %d items in the directory.", (int)count);
  PASS_CF(CFArrayContainsValue (contents, CFRangeMake (0, count), CFSTR("file1.txt")),
    "Directory has file1.txt");
  PASS_CF(CFArrayContainsValue (contents, CFRangeMake (0, count), CFSTR("file2.txt")),
    "Directory has file2.txt");
  
  CFRelease (dict);
  
  bytes = (const char*)CFDataGetBytePtr (data);
  PASS_CF(strncmp (bytes, test, strlen(test)) == 0,
    "Content read is the same the content that was written.");
  
  CFRelease (data);
  
  PASS_CF(CFURLDestroyResource(file1, NULL), "File1 was successfully deleted.");
  PASS_CF(CFURLDestroyResource(file2, NULL), "File2 was successfully deleted.");
  PASS_CF(CFURLDestroyResource(dir, NULL), "Directory was successfully deleted.");
  
  CFRelease (file1);
  CFRelease (file2);
  CFRelease (dir);
  
  return 0;
}