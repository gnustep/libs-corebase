#include "CoreFoundation/CFStream.h"
#include "../CFTesting.h"
#include <string.h>

void BufferTest1();

int main (void)
{
  BufferTest1();
  
  return 0;
}

void BufferTest1()
{
  CFReadStreamRef stream;
  const char* data = "lknvidufvbdfuvdvjbfdjkvbfdb jkd eaduyebiub";
  UInt8 buf[10];
  UInt8 buf2[50];
  CFIndex rd;

  stream = CFReadStreamCreateWithBytesNoCopy(NULL, data, strlen(data), kCFAllocatorNull);

  PASS_CF(CFReadStreamOpen(stream), "CFReadStreamOpen() succeeds");

  PASS_CF(sizeof(buf) == CFReadStreamRead(stream, buf, sizeof(buf)), "CFReadStreamRead() reads data");
  PASS_CF(memcmp(buf, data, sizeof(buf)) == 0, "CFReadStreamRead() reads correct data");

  rd = CFReadStreamRead(stream, buf2, sizeof(buf2));
  PASS_CF(strlen(data) - sizeof(buf) == rd, "CFReadStreamRead() reads data");
  PASS_CF(memcmp(buf2, data+sizeof(buf), rd) == 0, "CFReadStreamRead() reads correct data");

  CFRelease(stream);
}
