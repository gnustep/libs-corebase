#ifndef __CFRuntime_h_GNUSTEP_COREBASE_INCLUDE
#define __CFRuntime_h_GNUSTEP_COREBASE_INCLUDE

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>

enum
{
  _kCFRuntimeNotATypeID = 0,
  _kCFRuntimeScannedObject = (1UL << 0),
  _kCFRuntimeResourcefulObject = (1UL << 2)
};

typedef struct __CFRuntimeClass CFRuntimeClass;
struct __CFRuntimeClass
{
  CFIndex version;
  const char *className;
  void (*init)(CFTypeRef cf);
  CFTypeRef (*copy)(CFAllocatorRef allocator, CFTypeRef cf);
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  void (*finalize)(CFTypeRef cf);
#else
  void (*dealloc)(CFTypeRef cf);
#endif
  Boolean (*equal)(CFTypeRef cf1, CFTypeRef cf2);
  CFHashCode (*hash)(CFTypeRef cf);
  CFStringRef (*copyFormattingDesc)(CFTypeRef cf, CFDictionaryRef formatOptions);
  CFStringRef (*copyDebugDesc)(CFTypeRef cf);
#if 0 // FIXME: OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
#define CF_RECLAIM_AVAILABLE 1
  void (*reclaim)(CFTypeRef cf);
#endif
};



CFTypeID
_CFRuntimeRegisterClass (const CFRuntimeClass * const cls);

const CFRuntimeClass *
_CFRuntimeGetClassWithTypeID (CFTypeID typeID);

void
_CFRuntimeUnregisterClassWithTypeID (CFTypeID typeID);



typedef struct __CFRuntimeBase CFRuntimeBase;
struct __CFRuntimeBase
{
  void *_isa;
  CFTypeID _typeid;
};

CFTypeRef
_CFRuntimeCreateInstance (CFAllocatorRef allocator, CFTypeID typeID,
                          CFIndex extraBytes, unsigned char *category);

void
_CFRuntimeSetInstanceTypeID (CFTypeRef cf, CFTypeID typeID);

void
_CFRuntimeInitStaticInstance (void *memory, CFTypeID typeID);
#define CF_HAS_INIT_STATIC_INSTANCE 0 // FIXME

#endif /* __CFRuntime_h_GNUSTEP_COREBASE_INCLUDE */
