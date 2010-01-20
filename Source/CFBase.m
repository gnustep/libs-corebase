/* CFBase.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#include <Foundation/Foundation.h>
#include <stddef.h>
#include <stdlib.h>

#include "CoreFoundation/CFBase.h"

//
// CFAllocator
//
/* FIXME: taken from NSObject.m verbatum for compatibility */
#ifdef ALIGN
#undef ALIGN
#endif
#define	ALIGN __alignof__(double)

/*
 *	Define a structure to hold information that is held locally
 *	(before the start) in each object.
 */
typedef struct obj_layout_unpadded {
    NSUInteger	retained;
    NSZone	*zone;
} unp;
#define	UNP sizeof(unp)

/*
 *	Now do the REAL version - using the other version to determine
 *	what padding (if any) is required to get the alignment of the
 *	structure correct.
 */
struct obj_layout {
    NSUInteger	retained;
    NSZone	*zone;
    char	padding[ALIGN - ((UNP % ALIGN) ? (UNP % ALIGN) : ALIGN)];
};
typedef	struct obj_layout *obj;



//
// kCFAllocatorMalloc
//
static void * malloc_malloc (CFAllocatorRef alloc, size_t size)
{
  return malloc (size);
}

static void * malloc_realloc (CFAllocatorRef alloc, void *ptr, size_t size)
{
  return realloc (ptr, size);
}

static void malloc_free (CFAllocatorRef alloc, void *ptr)
{
  free (ptr);
}

static void malloc_recycle (CFAllocatorRef alloc)
{
  // Cannot recycle allocator/zone.
}

static BOOL malloc_check (CFAllocatorRef alloc)
{
  return NO;
}

static BOOL malloc_lookup (CFAllocatorRef alloc, void *ptr)
{
  return NO;
}

static struct NSZoneStats malloc_stats (NSZone *alloc)
{
  /* FIXME: I'm not sure how to implement this */
  return (struct NSZoneStats){0, 0, 0, 0, 0};
}

static NSZone _kCFAllocatorMalloc =
{
  malloc_malloc,
  malloc_realloc,
  malloc_free,
  malloc_recycle,
  malloc_check,
  malloc_lookup,
  malloc_stats,
  0,
  @"kCFAllocatorMalloc",
  NULL
};

//
// kCFAllocatorNull
//
static void * null_malloc (CFAllocatorRef alloc, size_t size)
{
  return ;
}

static void * null_realloc (CFAllocatorRef alloc, void *ptr, size_t size)
{
  return NULL;
}

static void null_free (CFAllocatorRef alloc, void *ptr)
{
}

static void null_recycle (CFAllocatorRef alloc)
{
}

static BOOL null_check (CFAllocatorRef alloc)
{
  return NO;
}

static BOOL null_lookup (CFAllocatorRef alloc, void *ptr)
{
  return NO;
}

static struct NSZoneStats null_stats (NSZone *alloc)
{
  return (struct NSZoneStats){0, 0, 0, 0, 0};
}

static NSZone _kCFAllocatorNull =
{
  null_malloc,
  null_realloc,
  null_free,
  null_recycle,
  null_check,
  null_lookup,
  null_stats,
  0,
  @"kCFAllocatorNull",
  NULL
};

const CFAllocatorRef kCFAllocatorDefault = NULL;
/* FIXME
const CFAllocatorRef kCFAllocatorSystemDefault = &default_zone; */
const CFAllocatorRef kCFAllocatorMalloc = &_kCFAllocatorMalloc;
#if 0 // FIXME: OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
const CFAllocatorRef kCFAllocatorMallocZone = &default_zone;
#endif
const CFAllocatorRef kCFAllocatorNull = &_kCFAllocatorNull;
const CFAllocatorRef kCFAllocatorUseContext = (CFAllocatorRef)0x01;

// this will hold the default zone if set with CFAllocatorSetDefault ()
CFAllocatorRef __kCFAllocatorDefault = NULL;



/* FIXME: The next few functions were adapted from what's in NSObject.m.  It
   would really help if parts of those functions were made public. */
CFAllocatorRef CFAllocatorCreate(CFAllocatorRef allocator, CFAllocatorContext *context)
{
  /* FIXME: Creating Allocators in CF is completely different from ObjC */
  return NULL;
}

void *CFAllocatorAllocate(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint)
{
  void *new;
  int  newSize;
  
  newSize = size + sizeof(struct obj_layout);
  if (allocator == 0)
    {
      allocator = CFAllocatorGetDefault();
    }
  
  new = NSZoneMalloc(allocator, size);
  if (new != NULL)
    {
      memset (new, 0, size);
      ((obj)new)->zone = allocator;
      new = (void *)&((obj)new)[1];
    }
  
  return new;

}

void CFAllocatorDeallocate(CFAllocatorRef allocator, void *ptr)
{
  if (ptr != NULL)
    {
      obj o = &((obj)ptr)[-1];
      NSZoneFree(allocator, o);
    }
  
  return;
}

CFIndex CFAllocatorGetPreferredSizeForSize(CFAllocatorRef allocator, CFIndex size, CFOptionFlags hint)
{
  return 0;  /* FIXME */
}

void *CFAllocatorReallocate(CFAllocatorRef allocator, void *ptr, CFIndex newsize, CFOptionFlags hint)
{
  void *new;
  CFIndex size;
  
  if (allocator == NULL)
    {
      allocator = CFAllocatorGetDefault();
    }
  
  size = size + sizeof(struct obj_layout);
  new = NSZoneRealloc (allocator, &((obj)ptr)[-1], size);
  new = (void *)&((obj)new)[1];
  
  return new;
}

CFAllocatorRef CFAllocatorGetDefault(void)
{
  return NSDefaultMallocZone();
}

void CFAllocatorSetDefault(CFAllocatorRef allocator)
{
}

void CFAllocatorGetContext(CFAllocatorRef allocator, CFAllocatorContext *context)
{
  context = NULL;
}

CFTypeID CFAllocatorGetTypeID(void)
{
  return 0;
}



//
// CFType Functions
//
CFStringRef CFCopyDescription (CFTypeRef cf)
{
  return (CFStringRef)CFRetain([(id)cf description]);
}

CFStringRef CFCopyTypeIDDescription (CFTypeID typeID)
{
  return (CFStringRef)[(Class)typeID description];
}

Boolean CFEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return [(id)cf1 isEqual: (id)cf2];
}

CFAllocatorRef CFGetAllocator (CFTypeRef cf)
{
  return (CFAllocatorRef)[(id)cf zone];
}

CFIndex CFGetRetainCount (CFTypeRef cf)
{
  return [(id)cf retainCount];
}

CFTypeID CFGetTypeID (CFTypeRef cf)
{
  return (CFTypeID)[(id)cf class];
}

CFHashCode CFHash (CFTypeRef cf)
{
  return [(id)cf hash];
}

CFTypeRef CFMakeCollectable (CFTypeRef cf)
{
  return NULL;
}

void CFRelease (CFTypeRef cf)
{
  RELEASE((id)cf);
}

CFTypeRef CFRetain (CFTypeRef cf)
{
  return (CFTypeRef)RETAIN((id)cf);
}



//
// CFNull
//
/* FIXME: need to initialize this variable somewhere. */
CFNullRef kCFNull;

CFTypeID CFNullGetTypeID (void)
{
  return (CFTypeID)[NSNull class];
}
