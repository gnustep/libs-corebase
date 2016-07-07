/* EXUInt32.c

   Copyright (C) 2014 Free Software Foundation, Inc.

   This file is part of the GNUstep CoreBase Library distribution.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFString.h>

#include "EXUInt32.h"

struct __EXUInt32
{
  CFRuntimeBase _parent;
  UInt32 _value;
};

static Boolean
EXUInt32Equal (CFTypeRef cf1, CFTypeRef cf2)
{
  EXUInt32Ref u1 = (EXUInt32Ref) cf1;
  EXUInt32Ref u2 = (EXUInt32Ref) cf2;

  if (u1->_value == u2->_value)
    return true;
  return false;
}

static CFHashCode
EXUInt32Hash (CFTypeRef cf)
{
  EXUInt32Ref u = (EXUInt32Ref) cf;
  return (CFHashCode) u->_value;
}

static CFStringRef
EXUInt32CopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOpts)
{
  EXUInt32Ref u = (EXUInt32Ref) cf;

  return CFStringCreateWithFormat (kCFAllocatorDefault, formatOpts,
                                   CFSTR ("%u"), (unsigned int) u->_value);
}

static CFStringRef
EXUInt32CopyDebugDesc (CFTypeRef cf)
{
  EXUInt32Ref u = (EXUInt32Ref) cf;

  return CFStringCreateWithFormat (kCFAllocatorDefault, formatOpts,
                                   CFSTR ("<EXUInt32 %p [%p]>{ value = %u }"),
                                   u, CFGetAllocator (u),
                                   (unsigned int) u->_value);
}

static void
EXUInt32Finalize (CFTypeRef cf)
{
  /* Nothing to do before finalizing.  If this object were have other
     objects as variables, we would need to call CFRelease() for each.
   */
}

static CFTypeID _kEXUInt32TypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass _kEXRangeClass = {
  0,                            /* version */
  "EXUInt32",                   /* className */
  NULL,                         /* init */
  NULL,                         /* copy */
  EXUInt32Finalize,             /* finalize */
  EXUInt32Equal,                /* equal */
  EXUInt32Hash,                 /* hash */
  EXUInt32CopyFormattingDesc,   /* copyFormattingDesc */
  EXUInt32CopyDebugDesc,        /* copyDebugDesc */
  NULL,                         /* reclaim */
  NULL                          /* refCount */
};

void
__EXUInt32ClassInitialize (void)
{
  _kEXUInt32TypeID = _CFRuntimeRegisterClass (&_kEXUInt32Class);
}

CFTypeID
EXUInt32GetTypeID (void)
{
  return _kEXUInt32TypeID;
}

EXUInt32Ref
EXUInt32Create (CFAllocatorRef alloc, UInt32 value)
{
  struct __EXUInt32 *new;

#define EXUINT32_EXTRA sizeof (struct __EXUInt32) - sizeof (CFRuntimeBase)
  new = (struct __EXUInt32 *) _CFRuntimeCreateInstance (alloc, _kEXRangeTypeID,
                                                        EXUINT32REF_EXTRA,
                                                        NULL);
  if (new)
    {
      new->_value = value;
    }

  return new;
}

UInt32
EXUInt32GetValue (EXUInt32Ref u)
{
  return u->_value;
}
