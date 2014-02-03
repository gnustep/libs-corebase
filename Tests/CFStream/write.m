#include "CoreFoundation/CFStream.h"
#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"
#include <string.h>

void BufferTest1();
void BufferTest2();

int main (void)
{
  BufferTest1();
  
  return 0;
}

void BufferTest1()
{
  CFWriteStreamRef stream;
  CFDataRef data;
  const char* test1 = "Hello WorldHelloWorld!!!";
  const char* test2 = "Abcdefghijklmn";

  stream = CFWriteStreamCreateWithAllocatedBuffers(NULL, NULL);

  PASS_CF(CFWriteStreamOpen(stream), "CFWriteStreamOpen() succeeds");
  
  PASS_CF(strlen(test1) == CFWriteStreamWrite(stream, test1, strlen(test1)), "CFWriteStreamWrite() writes into buffer");
  PASS_CF(strlen(test2) == CFWriteStreamWrite(stream, test2, strlen(test2)), "CFWriteStreamWrite() writes into buffer");
  
  data = CFWriteStreamCopyProperty(stream, kCFStreamPropertyDataWritten);
  
  PASS_CF(data != NULL, "CFWriteStreamCopyProperty returns written data");
  
  PASS_CF(strlen(test1)+strlen(test2) == CFDataGetLength(data), "Written data have correct length");
  PASS_CF(memcmp(test1, CFDataGetBytePtr(data), strlen(test1)) == 0, "Written data contain correct data");
  PASS_CF(memcmp(test2, CFDataGetBytePtr(data) + strlen(test1), strlen(test2)) == 0, "Written data contain correct data");
  
  CFRelease(data);
  CFRelease(stream);
}

void BufferTest2()
{
  CFWriteStreamRef stream;
  UInt8 mybuffer[10];
  const char* test1 = "Hello WorldHelloWorld!!!";

  stream = CFWriteStreamCreateWithBuffer(NULL, mybuffer, sizeof(mybuffer));

  PASS_CF(CFWriteStreamOpen(stream), "CFWriteStreamOpen() succeeds");
  
  PASS_CF(sizeof(mybuffer) == CFWriteStreamWrite(stream, test1, strlen(test1)), "CFWriteStreamWrite() writes into buffer");
  
  PASS_CF(memcmp(test1, mybuffer, sizeof(mybuffer)) == 0, "Written data contain correct data");
  
  CFRelease(stream);
}
