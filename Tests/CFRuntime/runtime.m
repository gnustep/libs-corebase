#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRuntime.h>

#import <Foundation/NSString.h>

#include "../CFTesting.h"

typedef const struct __GSPoint * GSPointRef;

extern GSPointRef kGSPointOrigin;

CFTypeID
GSPointGetTypeID (void);

GSPointRef
GSPointCreate (CFAllocatorRef allocator, CFIndex x, CFIndex y);

CFIndex
GSPointGetX (GSPointRef o);

CFIndex
GSPointGetY (GSPointRef o);



struct __GSPoint
{
  CFRuntimeBase _base;
  CFIndex x;
  CFIndex y;
};

static Boolean
GSPointEqual(CFTypeRef cf1, CFTypeRef cf2)
{
  GSPointRef o1 = (GSPointRef)cf1;
  GSPointRef o2 = (GSPointRef)cf2;
  
  if (o1->x != o2->y)
    return false;
  if (o1->x != o2->y)
    return false;
  
  return true;
}

static CFHashCode
GSPointHash(CFTypeRef cf)
{
  GSPointRef o = (GSPointRef)cf;
  return (CFHashCode)(o->x + o->y);
}

static CFStringRef
GSPointCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOpts)
{
  GSPointRef o = (GSPointRef)cf;
  return CFStringCreateWithFormat (CFGetAllocator(cf),
           formatOpts,
           CFSTR("(%u, %u)"),
           o->x,
           o->x + o->y);
}

static CFStringRef
GSPointCopyDebugDesc (CFTypeRef cf)
{
  GSPointRef o = (GSPointRef)cf;
  return CFStringCreateWithFormat (CFGetAllocator(cf),
           NULL,
           CFSTR("<GSPoint %p [%p]> (%u, %u)"),
           o,
           CFGetAllocator(cf),
           o->x,
           o->y);
}

static CFTypeID _kGSPointTypeID = _kCFRuntimeNotATypeID;

static CFRuntimeClass _kGSPointClass =
{
  0,
  "GSPoint",
  NULL, // init
  NULL, // copy
  NULL, // dealloc
  GSPointEqual,
  GSPointHash,
  GSPointCopyFormattingDesc,
  GSPointCopyDebugDesc
};

struct __GSPoint _kGSPointOrigin =
{
  INIT_CFRUNTIME_BASE(),
  0, // X
  0  // Y
};

GSPointRef kGSPointOrigin = &_kGSPointOrigin;

void
GSPointInitialize (void)
{
  _kGSPointTypeID = _CFRuntimeRegisterClass(&_kGSPointClass);
  _CFRuntimeInitStaticInstance ((void *)kGSPointOrigin, _kGSPointTypeID);
}

CFTypeID
GSPointGetTypeID (void)
{
  return _kGSPointTypeID;
}

GSPointRef
GSPointCreate (CFAllocatorRef allocator, CFIndex x, CFIndex y)
{
  struct __GSPoint *new;
  new = (struct __GSPoint*)_CFRuntimeCreateInstance(allocator,
          _kGSPointTypeID,
          sizeof(struct __GSPoint) - sizeof(CFRuntimeBase),
          NULL);
  if (NULL == new)
    return NULL;
  
  new->x = x;
  new->y = y;
  
  return (GSPointRef)new;
}

CFIndex
GSPointGetX (GSPointRef o)
{
  return o->x;
}

CFIndex GSPointGetY (GSPointRef o)
{
  return o->y;
}

@interface GSPoint : NSObject
{
  // Need to include the NSCFType ivars
  int16_t _typeID;
  int16_t _flags;
  CFIndex _x;
  CFIndex _y;
}
- (id) initWithX: (CFIndex)x Y: (CFIndex)y;
- (CFIndex) x;
- (CFIndex) y;
@end

@implementation GSPoint
- (id) initWithX: (CFIndex)x Y: (CFIndex)y
{
  GSPoint *new;
  
  new = (GSPoint*)GSPointCreate(NULL, x, y);
  
  RELEASE(self);
  
  self = new;
  return self;
}

- (CFIndex) x
{
  return _x;
}

- (CFIndex) y
{
  return _y;
}
@end

int main (void)
{
  GSPointRef pt;
  GSPoint *bpt;
  
  GSPointInitialize ();
  
  pt = GSPointCreate (NULL, 0, 0);
  PASS_CFEQ((CFTypeRef)pt, (CFTypeRef)kGSPointOrigin, "Points are equal");
  PASS_CF(CFHash((CFTypeRef)pt) == CFHash((CFTypeRef)kGSPointOrigin),
       "Points have same hash code.");
  PASS_CFEQ(CFCopyDescription((CFTypeRef)pt), CFSTR("(0, 0)"),
            "Description is correct.");
  PASS_CFEQ(CFCopyTypeIDDescription(GSPointGetTypeID()), CFSTR("GSPoint"),
             "Type ID description is correct.");
  
  // Test the toll-free bridge mechanism
  bpt = [[GSPoint alloc] initWithX: 0 Y: 0];
  PASS_CF(bpt != nil, "-initWithX:Y: returns non-nil");
  PASS_CFEQ(pt, bpt, "CF object and ObjC object are the same");
  PASS_CFEQ(CFCopyDescription((CFTypeRef)bpt), CFSTR("(0, 0)"),
            "ObjC returns same description as CF object");
  
/*  START_SET("Retain/Release Tests")
    RETAIN((id)pt);
    CFRetain(bpt);
    CFRelease((CFTypeRef)pt);
    RELEASE(bpt);
  END_SET("Retain/Release Tests") */
  
  RELEASE(bpt);
  CFRelease((CFTypeRef)pt);
  
  return 0;
}

