/* CFStream.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: August, 2012

   Written by: Lubos Dolezel
   Date: January, 2014
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFStream.h"
#include "GSPrivate.h"
#include "GSObjCRuntime.h"
#include "GSMemory.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFStreamPriv.h"

#ifndef _WIN32
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <errno.h>
#endif
#include <stdlib.h>

CONST_STRING_DECL(kCFStreamPropertyDataWritten, "kCFStreamPropertyDataWritten");
CONST_STRING_DECL(kCFStreamPropertySocketNativeHandle,
  "kCFStreamPropertySocketNativeHandle");
CONST_STRING_DECL(kCFStreamPropertySocketRemoteHostName,
  "kCFStreamPropertySocketRemoteHostName");
CONST_STRING_DECL(kCFStreamPropertySocketRemotePortNumber,
  "kCFStreamPropertySocketRemotePortNumber");
CONST_STRING_DECL(kCFStreamPropertyAppendToFile,
  "kCFStreamPropertyAppendToFile");
CONST_STRING_DECL(kCFStreamPropertyFileCurrentOffset,
  "kCFStreamPropertyFileCurrentOffset");

static CFTypeID _kCFWriteStreamTypeID = 0;
static CFTypeID _kCFReadStreamTypeID = 0;

static void
CFWriteStreamFDFinalize(CFWriteStreamRef s);
static void
CFWriteStreamFDClose(CFWriteStreamRef s);
static void
CFWriteStreamBufferClose(CFWriteStreamRef s);
static CFTypeRef
CFWriteStreamFDCopyProperty (CFWriteStreamRef s, CFStringRef propertyName);
static CFTypeRef
CFWriteStreamBufferCopyProperty (CFWriteStreamRef s, CFStringRef propertyName);
static Boolean
CFWriteStreamFDSetProperty (CFWriteStreamRef s, CFStringRef propertyName,
                          CFTypeRef propertyValue);
static Boolean
CFWriteStreamBufferSetProperty (CFWriteStreamRef s, CFStringRef propertyName,
                          CFTypeRef propertyValue);
static CFIndex
CFWriteStreamFDWrite (CFWriteStreamRef s, const UInt8 *buffer,
                    CFIndex bufferLength);
static CFIndex
CFWriteStreamBufferWrite (CFWriteStreamRef s, const UInt8 *buffer,
                    CFIndex bufferLength);
static Boolean
CFWriteStreamFDOpen (CFWriteStreamRef s);
static Boolean
CFWriteStreamBufferOpen (CFWriteStreamRef s);



struct CFWriteStreamFD
{
    struct __CFWriteStream parent;
    int                    fd; // FIXME: support Windows?
    Boolean                append;
    CFURLRef               url;
};

static const struct CFWriteStreamImpl CFWriteStreamFDImpl = {
    CFWriteStreamFDClose,
    CFWriteStreamFDFinalize,
    CFWriteStreamFDOpen,
    CFWriteStreamFDWrite,
    CFWriteStreamFDCopyProperty,
    CFWriteStreamFDSetProperty,
    NULL
};

static const struct CFWriteStreamImpl CFWriteStreamBufferImpl = {
    CFWriteStreamBufferClose,
    NULL,
    CFWriteStreamBufferOpen,
    CFWriteStreamBufferWrite,
    CFWriteStreamBufferCopyProperty,
    CFWriteStreamBufferSetProperty,
    NULL
};

struct CFWriteStreamBuffer
{
    struct __CFWriteStream parent;
    CFAllocatorRef         bufferAllocator;
    UInt8                  *buffer;
    CFIndex                bufferCapacity;
    CFIndex                position;
};

static void
CFReadStreamFDClose (CFReadStreamRef s);
static void
CFReadStreamBufferClose (CFReadStreamRef s);
static const UInt8 *
CFReadStreamBufferGetBuffer (CFReadStreamRef s, CFIndex maxBytesToRead,
                       CFIndex *numBytesRead);
static Boolean
CFReadStreamBufferHasBytesAvailable (CFReadStreamRef s);
static Boolean
CFReadStreamFDOpen (CFReadStreamRef s);
static Boolean
CFReadStreamBufferOpen (CFReadStreamRef s);
static CFIndex
CFReadStreamFDRead (CFReadStreamRef s, UInt8 *buffer, CFIndex bufferLength);
static CFIndex
CFReadStreamBufferRead (CFReadStreamRef s, UInt8 *buffer, CFIndex bufferLength);
static Boolean
CFReadStreamFDSetProperty (CFReadStreamRef s, CFStringRef propertyName,
                         CFTypeRef propertyValue);
static Boolean
CFReadStreamBufferSetProperty (CFReadStreamRef s, CFStringRef propertyName,
                         CFTypeRef propertyValue);
static CFTypeRef
CFReadStreamFDCopyProperty (CFReadStreamRef s, CFStringRef propertyName);
static void
CFReadStreamFDFinalize(CFReadStreamRef s);

static const struct CFReadStreamImpl CFReadStreamFDImpl = {
    CFReadStreamFDClose,
    CFReadStreamFDFinalize,
    CFReadStreamFDOpen,
    CFReadStreamFDRead,
    CFReadStreamFDCopyProperty,
    CFReadStreamFDSetProperty,
    NULL,
    NULL
};

static const struct CFReadStreamImpl CFReadStreamBufferImpl = {
    CFReadStreamBufferClose,
    NULL,
    CFReadStreamBufferOpen,
    CFReadStreamBufferRead,
    NULL,
    CFReadStreamBufferSetProperty,
    CFReadStreamBufferGetBuffer,
    CFReadStreamBufferHasBytesAvailable
};

struct CFReadStreamFD
{
    struct __CFReadStream parent;
    int      fd;
    CFURLRef url;
};

struct CFReadStreamBuffer
{
    struct __CFReadStream parent;
    CFAllocatorRef        bufferAllocator;
    const UInt8           *buffer;
    CFIndex               bufferCapacity;
    CFIndex               position;
};

#define CFWRITESTREAMFD_SIZE (sizeof(struct CFWriteStreamFD) - sizeof(CFRuntimeBase))
#define CFWRITESTREAMBUFFER_SIZE (sizeof(struct CFWriteStreamBuffer) - sizeof(CFRuntimeBase))
#define CFREADSTREAMFD_SIZE (sizeof(struct CFReadStreamFD) - sizeof(CFRuntimeBase))
#define CFREADSTREAMBUFFER_SIZE (sizeof(struct CFReadStreamBuffer) - sizeof(CFRuntimeBase))

static void
CFWriteStreamFinalize (CFTypeRef cf)
{
  CFWriteStreamRef s = (CFWriteStreamRef)cf;
  
  CFWriteStreamClose(s);

  if (s->error)
    CFRelease(s->error);

  if (s->impl.finalize != NULL)
    s->impl.finalize(s);
}

static void
CFWriteStreamFDFinalize(CFWriteStreamRef stream)
{
  struct CFWriteStreamFD* s = (struct CFWriteStreamFD*) stream;
  if (s->url != NULL)
    CFRelease(s->url);
}

void
CFWriteStreamSetError(CFWriteStreamRef stream, int error)
{
  if (stream->error)
    CFRelease(stream->error);
  stream->error = CFErrorCreate(NULL, kCFErrorDomainPOSIX, error, NULL);
}

void
CFReadStreamSetError(CFReadStreamRef stream, int error)
{
  if (stream->error)
    CFRelease(stream->error);
  stream->error = CFErrorCreate(NULL, kCFErrorDomainPOSIX, error, NULL);
}

static CFRuntimeClass CFWriteStreamClass =
{
  0,
  "CFWriteStream",
  NULL,
  NULL,
  CFWriteStreamFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

static void
CFReadStreamFDFinalize(CFReadStreamRef s)
{
  struct CFReadStreamFD* ss = (struct CFReadStreamFD*) s;

  if (ss->url != NULL)
    CFRelease(ss->url);
}

static void
CFReadStreamFinalize (CFTypeRef cf)
{
  CFReadStreamRef s = (CFReadStreamRef)cf;
  
  CFReadStreamClose(s);

  if (s->error)
    CFRelease(s->error);
  
  if (s->impl.finalize)
    s->impl.finalize(s);
}

static CFRuntimeClass CFReadStreamClass =
{
  0,
  "CFReadStream",
  NULL,
  NULL,
  CFReadStreamFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void
CFStreamInitialize (void)
{
  _kCFWriteStreamTypeID = _CFRuntimeRegisterClass (&CFWriteStreamClass);
  _kCFReadStreamTypeID = _CFRuntimeRegisterClass (&CFReadStreamClass);
}

CFTypeID
CFWriteStreamGetTypeID (void)
{
  return _kCFWriteStreamTypeID;
}

CFTypeID
CFReadStreamGetTypeID (void)
{
  return _kCFReadStreamTypeID;
}

void
CFStreamCreateBoundPair (CFAllocatorRef alloc, CFReadStreamRef *readStream,
                         CFWriteStreamRef *writeStream,
                         CFIndex transferBufferSize)
{
  // FIXME
}

/*
void
CFStreamCreatePairWithPeerSocketSignature (CFAllocatorRef alloc,
                                           const CFSocketSignature *signature,
                                           CFReadStreamRef *readStream,
                                           CFWriteStreamRef *writeStream)
{
  // FIXME
}
*/

void
CFStreamCreatePairWithSocketToHost (CFAllocatorRef alloc, CFStringRef host,
                                    UInt32 port, CFReadStreamRef *readStream,
                                    CFWriteStreamRef *writeStream)
{
  
}



Boolean
CFWriteStreamCanAcceptBytes (CFWriteStreamRef stream)
{
  CF_OBJC_FUNCDISPATCHV(_kCFWriteStreamTypeID, Boolean, stream,
                        "hasSpaceAvailable");

  if (stream->impl.acceptBytes != NULL)
    return stream->impl.acceptBytes(stream);
  return true;
}

static void
CFWriteStreamFDClose(CFWriteStreamRef s)
{
  struct CFWriteStreamFD* stream = (struct CFWriteStreamFD*) s;
  if (stream->fd != -1)
    {
      close(stream->fd);
      stream->fd = -1;
    }
}

static void
CFWriteStreamBufferClose(CFWriteStreamRef s)
{
  struct CFWriteStreamBuffer* stream =
    (struct CFWriteStreamBuffer*) s;

  if (stream->buffer != NULL && stream->bufferAllocator != NULL)
    {
      CFAllocatorDeallocate(stream->bufferAllocator, stream->buffer);
    }

  stream->buffer = NULL;
}

void
CFWriteStreamClose (CFWriteStreamRef stream)
{
  if (stream->closed)
    return;

  if (stream->impl.close != NULL)
    stream->impl.close(stream);

  stream->closed = true;
  stream->open = false;
}

CFErrorRef
CFWriteStreamCopyError (CFWriteStreamRef stream)
{
  return (CFErrorRef) CFRetain(stream->error);
}

static CFTypeRef
CFWriteStreamBufferCopyProperty (CFWriteStreamRef s, CFStringRef propertyName)
{
  struct CFWriteStreamBuffer* stream = (struct CFWriteStreamBuffer*) s;

  if (!s->open)
    {
      CFWriteStreamSetError(s, EBADF);
      return NULL;
    }

  if (CFEqual(propertyName, kCFStreamPropertyDataWritten))
    {
      return CFDataCreate(NULL, stream->buffer, stream->position);
    }
  else if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {
      return CFNumberCreate(NULL, kCFNumberCFIndexType, &stream->position);
    }

  // unknown property
  CFWriteStreamSetError(s, EINVAL);
  return NULL;
}

static CFTypeRef
CFWriteStreamFDCopyProperty (CFWriteStreamRef s, CFStringRef propertyName)
{
  struct CFWriteStreamFD* stream = (struct CFWriteStreamFD*) s;

  if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {
      long long offset = 0;

      if (!s->open)
        {
          CFWriteStreamSetError(s, EBADF);
          return NULL;
        }
          
      offset = lseek(stream->fd, 0, SEEK_CUR);

      if (offset == (off_t)-1)
        {
          CFWriteStreamSetError(s, errno);
          return NULL;
        }

      return CFNumberCreate(NULL, kCFNumberLongLongType, &offset);
    }

  // unknown property
  CFWriteStreamSetError(s, EINVAL);
  return NULL;
}

CFTypeRef
CFWriteStreamCopyProperty (CFWriteStreamRef stream, CFStringRef propertyName)
{
  if (stream->impl.copyProperty != NULL)
    return stream->impl.copyProperty(stream, propertyName);

  // unknown property
  CFWriteStreamSetError(stream, EINVAL);
  return NULL;
}

CFWriteStreamRef
CFWriteStreamCreateWithAllocatedBuffers (CFAllocatorRef alloc,
                                         CFAllocatorRef bufferAllocator)
{
  CFWriteStreamRef new;

  new = (CFWriteStreamRef)_CFRuntimeCreateInstance (alloc, _kCFWriteStreamTypeID,
                                                   CFWRITESTREAMBUFFER_SIZE, 0);
  GSMemoryCopy(&new->impl, &CFWriteStreamBufferImpl, sizeof(CFWriteStreamBufferImpl));

  ((struct CFWriteStreamBuffer *)new)->bufferAllocator = bufferAllocator;

  return new;
}

CFWriteStreamRef
CFWriteStreamCreateWithBuffer (CFAllocatorRef alloc, UInt8 *buffer,
                               CFIndex bufferCapacity)
{
  CFWriteStreamRef new;
  struct CFWriteStreamBuffer *sbuf;

  if (buffer == NULL && bufferCapacity > 0)
    return NULL;

  new = (CFWriteStreamRef)_CFRuntimeCreateInstance (alloc, _kCFWriteStreamTypeID,
                                                   CFWRITESTREAMBUFFER_SIZE, 0);
  sbuf = ((struct CFWriteStreamBuffer *)new);
  GSMemoryCopy(&new->impl, &CFWriteStreamBufferImpl, sizeof(CFWriteStreamBufferImpl));
  sbuf->buffer = buffer;
  sbuf->bufferCapacity = bufferCapacity;
  sbuf->bufferAllocator = kCFAllocatorNull;

  return new;
}

CFWriteStreamRef
CFWriteStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  CFWriteStreamRef new;
  CFStringRef scheme;
  CFStringRef schemeFile = CFSTR("file");
  struct CFWriteStreamFD *sfd;

  if (fileURL == NULL)
    return NULL;

  scheme = CFURLCopyScheme(fileURL);
  if (!CFEqual(schemeFile, scheme))
    {
      CFRelease(scheme);
      return NULL;
    }
  CFRelease(scheme);

  new = (CFWriteStreamRef)_CFRuntimeCreateInstance (alloc, _kCFWriteStreamTypeID,
                                                   CFWRITESTREAMFD_SIZE, 0);

  sfd = ((struct CFWriteStreamFD *)new);
  GSMemoryCopy(&new->impl, &CFWriteStreamFDImpl, sizeof(CFWriteStreamFDImpl));
  sfd->url = (CFURLRef) CFRetain(fileURL);
  sfd->fd = -1;

  return new;
}


static CFStreamError
CFStreamGetError (CFErrorRef inError)
{
  if (!inError)
    {
      CFStreamError error = { kCFStreamErrorDomainPOSIX, 0 };
      return error;
    }
  else
    {
      CFStreamError error;
      CFStringRef domain;

      domain = CFErrorGetDomain(inError);

      if (CFEqual(domain, kCFErrorDomainPOSIX))
        error.domain = kCFStreamErrorDomainPOSIX;
      else
        error.domain = kCFStreamErrorDomainCustom;

      error.error = CFErrorGetCode(inError);
      return error;
    }
}

CFStreamError
CFWriteStreamGetError (CFWriteStreamRef stream)
{
  return CFStreamGetError(stream->error);
}

CFStreamStatus
CFWriteStreamGetStatus (CFWriteStreamRef stream)
{
  if (stream->closed)
    return kCFStreamStatusClosed;
  else if (stream->failed)
    return kCFStreamStatusError;
  else if (stream->open)
    return kCFStreamStatusOpen;
  else
    return kCFStreamStatusNotOpen;
}

static Boolean
CFWriteStreamFDOpen (CFWriteStreamRef s)
{
  struct CFWriteStreamFD* stream = (struct CFWriteStreamFD*) s;
  CFStringRef path = CFURLCopyFileSystemPath(stream->url,
                                                 kCFURLPOSIXPathStyle);
  int flags = O_WRONLY | O_CREAT;

  if (stream->append)
    flags |= O_APPEND;

#ifdef O_LARGEFILE
  flags |= O_LARGEFILE;
#endif

  stream->fd = open(CFStringGetCStringPtr(path, kCFStringEncodingUTF8),
                    flags, 0666);

  CFRelease(path);
  if (stream->fd == -1)
    {
      CFWriteStreamSetError(s, errno);
      return false;
    }

  return true;
}

static Boolean
CFWriteStreamBufferOpen (CFWriteStreamRef s)
{
  return true;
}

Boolean
CFWriteStreamOpen (CFWriteStreamRef stream)
{
  if (stream->closed || stream->open)
    {
      CFWriteStreamSetError(stream, EBADF);
      return false;
    }

  if (!stream->impl.open(stream))
    return false;

  stream->open = true;
  return true;
}

void
CFWriteStreamScheduleWithRunLoop (CFWriteStreamRef stream,
                                  CFRunLoopRef runLoop,
                                  CFStringRef runLoopMode)
{
  ;
}

Boolean
CFWriteStreamSetClient (CFWriteStreamRef stream, CFOptionFlags streamEvents,
                        CFWriteStreamClientCallBack clientCB,
                        CFStreamClientContext *clientContext)
{
  return false;
}

static Boolean
CFWriteStreamFDSetProperty (CFWriteStreamRef s, CFStringRef propertyName,
                          CFTypeRef propertyValue)
{
  struct CFWriteStreamFD* stream = (struct CFWriteStreamFD*) s;

  if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {
      if (!s->open)
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }
      if (CFGetTypeID(propertyValue) != CFNumberGetTypeID())
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }

      long long offset;

      if (!CFNumberGetValue((CFNumberRef) propertyValue,
                            kCFNumberLongLongType, &offset))
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }
          
      if (lseek(stream->fd, offset, SEEK_SET) == (off_t)-1)
        {
          CFWriteStreamSetError(s, errno);
          return false;
        }

      return true;
    }

  else if (CFEqual(propertyName, kCFStreamPropertyAppendToFile))
    {
      if (s->open)
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }
      if (CFGetTypeID(propertyValue) != CFBooleanGetTypeID())
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }

      stream->append = propertyValue == kCFBooleanTrue;
      return true;
    }
/*  else if (stream->isSocket
           && CFEqual(propertyName, kCFStreamPropertyShouldCloseNativeSocket))
    {
      stream->closeOriginal = propertyValue == kCFBooleanTrue;
      if (stream->readStream)
        stream->readStream->closeOriginal = stream->closeOriginal;
      return true;
    }*/

  CFWriteStreamSetError(s, EINVAL);
  return false;
}

static Boolean
CFWriteStreamBufferSetProperty (CFWriteStreamRef s, CFStringRef propertyName,
                          CFTypeRef propertyValue)
{
  struct CFWriteStreamBuffer* stream = (struct CFWriteStreamBuffer*) s;
  
  if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {
      if (!s->open)
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }
      if (CFGetTypeID(propertyValue) != CFNumberGetTypeID())
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }

      if (!CFNumberGetValue((CFNumberRef) propertyValue,
                            kCFNumberCFIndexType, &stream->position))
        {
          CFWriteStreamSetError(s, EINVAL);
          return false;
        }
          
      return true;
    }

  CFWriteStreamSetError(s, EINVAL);
  return false;
}

Boolean
CFWriteStreamSetProperty (CFWriteStreamRef stream, CFStringRef propertyName,
                          CFTypeRef propertyValue)
{
  return stream->impl.setProperty(stream, propertyName, propertyValue);
}

void
CFWriteStreamUnscheduleFromRunLoop (CFWriteStreamRef stream,
                                    CFRunLoopRef runLoop,
                                    CFStringRef runLoopMode)
{
  
}

static CFIndex
CFWriteStreamFDWrite (CFWriteStreamRef s, const UInt8 *buffer,
                    CFIndex bufferLength)
{
  struct CFWriteStreamFD* stream = (struct CFWriteStreamFD*) s;
  int wr;

  wr = write(stream->fd, buffer, bufferLength);
  if (wr == -1)
    CFWriteStreamSetError(s, errno);

  return wr;
}

static CFIndex
CFWriteStreamBufferWrite (CFWriteStreamRef s, const UInt8 *buffer,
                    CFIndex bufferLength)
{
  struct CFWriteStreamBuffer* stream = (struct CFWriteStreamBuffer*) s;

  CFIndex bufSpace = stream->bufferCapacity - stream->position;
      
  if (bufSpace < bufferLength)
    {
      if (stream->bufferAllocator != kCFAllocatorNull)
        {
          CFIndex cap = stream->bufferCapacity;
          UInt8 *newbuf;

          if (cap == 0)
            cap = 4;

          while (cap - stream->position < bufferLength)
            cap *= 2;

          newbuf = (UInt8*) CFAllocatorReallocate(stream->bufferAllocator,
                                                  stream->buffer, cap, 0);

          if (!newbuf)
            return -1;

          stream->buffer = newbuf;
          stream->bufferCapacity = cap;
        }
      else
        bufferLength = bufSpace;
    }

  GSMemoryCopy(stream->buffer + stream->position, buffer, bufferLength);
  stream->position += bufferLength;

  return bufferLength;
}

CFIndex
CFWriteStreamWrite (CFWriteStreamRef stream, const UInt8 *buffer,
                    CFIndex bufferLength)
{
  CF_OBJC_FUNCDISPATCHV(_kCFWriteStreamTypeID, CFIndex, stream,
                        "write:maxLength:", buffer, bufferLength);

  if (!stream->open)
    return -1;

  return stream->impl.write(stream, buffer, bufferLength);
}

static void
CFReadStreamFDClose (CFReadStreamRef s)
{
  struct CFReadStreamFD* stream = (struct CFReadStreamFD*) s;
  if (stream->fd != -1)
    {
      close(stream->fd);
      stream->fd = -1;
    }
}

static void
CFReadStreamBufferClose (CFReadStreamRef s)
{
  struct CFReadStreamBuffer* stream = (struct CFReadStreamBuffer*) s;

  if (stream->buffer != NULL)
    {
      CFAllocatorDeallocate(stream->bufferAllocator, (void*)stream->buffer);
      stream->buffer = NULL;
    }
}

void
CFReadStreamClose (CFReadStreamRef stream)
{
  if (stream->closed)
    return;

  if (stream->impl.close)
    stream->impl.close(stream);

  stream->closed = true;
  stream->open = false;
}

CFErrorRef
CFReadStreamCopyError (CFReadStreamRef stream)
{
  if (!stream->error)
    return NULL;

  return (CFErrorRef) CFRetain(stream->error);
}

static CFTypeRef
CFReadStreamFDCopyProperty (CFReadStreamRef s, CFStringRef propertyName)
{
  // struct CFReadStreamFD* stream = (struct CFReadStreamFD*) s;

  CFReadStreamSetError(s, EINVAL);
  return NULL;
}

CFTypeRef
CFReadStreamCopyProperty (CFReadStreamRef stream, CFStringRef propertyName)
{
  if (stream->impl.copyProperty != NULL)
    return stream->impl.copyProperty(stream, propertyName);

  // unknown property
  CFReadStreamSetError(stream, EINVAL);
  return NULL;
}

CFReadStreamRef
CFReadStreamCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
                                   CFIndex length,
                                   CFAllocatorRef bytesDeallocator)
{
  CFReadStreamRef new;
  struct CFReadStreamBuffer* sbuf;

  if (bytes == NULL && length > 0)
    return NULL;

  new = (CFReadStreamRef)_CFRuntimeCreateInstance (alloc, _kCFReadStreamTypeID,
                                                   CFREADSTREAMBUFFER_SIZE, 0);

  GSMemoryCopy(&new->impl, &CFReadStreamBufferImpl, sizeof(CFReadStreamBufferImpl));
  sbuf = (struct CFReadStreamBuffer*) new;
  sbuf->buffer = bytes;
  sbuf->bufferAllocator = bytesDeallocator;
  sbuf->bufferCapacity = length;

  return new;
}

CFReadStreamRef
CFReadStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  CFReadStreamRef new;
  CFStringRef scheme;
  CFStringRef schemeFile = CFSTR("file");
  struct CFReadStreamFD* sfd;

  if (fileURL == NULL)
    return NULL;

  scheme = CFURLCopyScheme(fileURL);
  if (!CFEqual(schemeFile, scheme))
    {
      CFRelease(scheme);
      return NULL;
    }
  CFRelease(scheme);

  new = (CFReadStreamRef)_CFRuntimeCreateInstance (alloc, _kCFReadStreamTypeID,
                                                   CFREADSTREAMFD_SIZE, 0);
  GSMemoryCopy(&new->impl, &CFReadStreamFDImpl, sizeof(CFReadStreamFDImpl));
  sfd = (struct CFReadStreamFD*) new;
  sfd->url = (CFURLRef) CFRetain(fileURL);
  sfd->fd = -1;

  return new;
}

static const UInt8 *
CFReadStreamBufferGetBuffer (CFReadStreamRef s, CFIndex maxBytesToRead,
                       CFIndex *numBytesRead)
{
  struct CFReadStreamBuffer* stream;
  const UInt8 *retval;

  stream = (struct CFReadStreamBuffer*) s;

  if (!numBytesRead)
    return NULL;

  if (maxBytesToRead < stream->bufferCapacity - stream->position)
    *numBytesRead = maxBytesToRead;
  else
    *numBytesRead = stream->bufferCapacity - stream->position;

  retval = stream->buffer + stream->position;
  stream->position += *numBytesRead;

  return retval;
}

const UInt8 *
CFReadStreamGetBuffer (CFReadStreamRef stream, CFIndex maxBytesToRead,
                       CFIndex *numBytesRead)
{
  if (CF_IS_OBJC(_kCFReadStreamTypeID, stream))
    {
      const UInt8* buffer;
      Boolean rv;
      
      *numBytesRead = maxBytesToRead;

      CF_OBJC_CALLV(Boolean, rv, stream, "getBuffer:length:",
                    &buffer, numBytesRead);
      if (!rv)
        buffer = NULL;

      return buffer;
    }

  if (stream->impl.getBuffer == NULL)
    return NULL;
  return stream->impl.getBuffer(stream, maxBytesToRead, numBytesRead);
}

CFStreamError
CFReadStreamGetError (CFReadStreamRef stream)
{
  return CFStreamGetError(stream->error);
}

CFStreamStatus
CFReadStreamGetStatus (CFReadStreamRef stream)
{
  if (stream->closed)
    return kCFStreamStatusClosed;
  else if (stream->failed)
    return kCFStreamStatusError;
  else if (stream->open)
    return kCFStreamStatusOpen;
  else
    return kCFStreamStatusNotOpen;
}

static Boolean
CFReadStreamBufferHasBytesAvailable (CFReadStreamRef s)
{
  struct CFReadStreamBuffer* stream;

  stream = (struct CFReadStreamBuffer*) s;
  return stream->bufferCapacity > stream->position;
}

Boolean
CFReadStreamHasBytesAvailable (CFReadStreamRef stream)
{
  CF_OBJC_FUNCDISPATCHV(_kCFReadStreamTypeID, Boolean, stream,
                        "hasBytesAvailable");

  if (stream->impl.hasBytes != NULL)
    return stream->impl.hasBytes(stream);
  return true;
}

static Boolean
CFReadStreamFDOpen (CFReadStreamRef s)
{
  struct CFReadStreamFD* stream = (struct CFReadStreamFD*) s;

  CFStringRef path = CFURLCopyFileSystemPath(stream->url,
                                                 kCFURLPOSIXPathStyle);
  int flags = O_RDONLY;

#ifdef O_LARGEFILE
  flags |= O_LARGEFILE;
#endif

  stream->fd = open(CFStringGetCStringPtr(path, kCFStringEncodingUTF8),
                    flags);

  CFRelease(path);
  if (stream->fd == -1)
    {
      CFReadStreamSetError(s, errno);
      return false;
    }

  return true;
}

static Boolean
CFReadStreamBufferOpen (CFReadStreamRef s)
{
  return true;
}

Boolean
CFReadStreamOpen (CFReadStreamRef stream)
{
  if (stream->open || stream->closed)
    {
      CFReadStreamSetError(stream, EBADF);
      return false;
    }

  if (!stream->impl.open(stream))
    return false;

  stream->open = true;
  return true;
}

static CFIndex
CFReadStreamFDRead (CFReadStreamRef s, UInt8 *buffer, CFIndex bufferLength)
{
  int rd;
  struct CFReadStreamFD* stream = (struct CFReadStreamFD*) s;

  rd = read(stream->fd, buffer, bufferLength);

  if (rd == -1)
    CFReadStreamSetError(s, errno);

  return rd;
}

static CFIndex
CFReadStreamBufferRead (CFReadStreamRef s, UInt8 *buffer, CFIndex bufferLength)
{
  struct CFReadStreamBuffer* stream = (struct CFReadStreamBuffer*) s;

  if (stream->position >= stream->bufferCapacity)
    return 0;

  if (stream->bufferCapacity - stream->position < bufferLength)
    bufferLength = stream->bufferCapacity - stream->position;

  GSMemoryCopy(buffer, stream->buffer + stream->position, bufferLength);
  stream->position += bufferLength;

  return bufferLength;
}

CFIndex
CFReadStreamRead (CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength)
{
  CF_OBJC_FUNCDISPATCHV (_kCFReadStreamTypeID, CFIndex, stream,
                         "read:maxLength:",
                         buffer, bufferLength);

  if (!stream->open)
    {
      CFReadStreamSetError(stream, EBADF);
      return -1;
    }

  return stream->impl.read(stream, buffer, bufferLength);
}

void
CFReadStreamScheduleWithRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                 CFStringRef runLoopMode)
{
  
}

Boolean
CFReadStreamSetClient (CFReadStreamRef stream, CFOptionFlags streamEvents,
                       CFReadStreamClientCallBack clientCB,
                       CFStreamClientContext *clientContext)
{
  return false;
}

static Boolean
CFReadStreamFDSetProperty (CFReadStreamRef s, CFStringRef propertyName,
                         CFTypeRef propertyValue)
{
  struct CFReadStreamFD* stream = (struct CFReadStreamFD*) s;

  if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {

      if (!s->open)
        {
          CFReadStreamSetError(s, EINVAL);
          return false;
        }
      if (CFGetTypeID(propertyValue) != CFNumberGetTypeID())
        {
          CFReadStreamSetError(s, EINVAL);
          return false;
        }

      long long offset;

      if (!CFNumberGetValue((CFNumberRef) propertyValue,
                            kCFNumberLongLongType, &offset))
        {
          CFReadStreamSetError(s, EINVAL);
          return false;
        }

      if (lseek(stream->fd, offset, SEEK_SET) == (off_t)-1)
        {
          CFReadStreamSetError(s, errno);
          return false;
        }
  
      return true;
    }

  CFReadStreamSetError(s, EINVAL);
  return false;
}

static Boolean
CFReadStreamBufferSetProperty (CFReadStreamRef s, CFStringRef propertyName,
                         CFTypeRef propertyValue)
{
  struct CFReadStreamBuffer* stream = (struct CFReadStreamBuffer*) s;

  if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {
      if (!s->open)
        {
          CFReadStreamSetError(s, EINVAL);
          return false;
        }
      if (CFGetTypeID(propertyValue) != CFNumberGetTypeID())
        {
          CFReadStreamSetError(s, EINVAL);
          return false;
        }

      CFIndex offset;

      if (!CFNumberGetValue((CFNumberRef) propertyValue,
                            kCFNumberCFIndexType, &offset))
        {
          CFReadStreamSetError(s, EINVAL);
          return false;
        }

      stream->position = offset;
      return true;
    }

  CFReadStreamSetError(s, EINVAL);
  return false;
}

Boolean
CFReadStreamSetProperty (CFReadStreamRef stream, CFStringRef propertyName,
                         CFTypeRef propertyValue)
{
  if (stream->impl.setProperty != NULL)
    stream->impl.setProperty(stream, propertyName, propertyValue);

  CFReadStreamSetError(stream, EINVAL);
  return false;
}

void
CFReadStreamUnscheduleFromRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                   CFStringRef runLoopMode)
{
  
}

