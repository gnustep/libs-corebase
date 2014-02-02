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
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFError.h"

#ifndef _WIN32
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <errno.h>
#	include <netdb.h>
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

enum _CFWriteStreamType
{
    _kCFWriteStreamDescriptor, /* file or socket */
    _kCFWriteStreamExpandableBuffer, /* CFAllocatorRef allocated buffer */
    _kCFWriteStreamFixedBuffer /* Fixed buffer with a given length */
};

struct __CFWriteStream
{
    CFRuntimeBase               parent;
    enum _CFWriteStreamType     type;

    /* reference to related read stream for socket streams */
    CFReadStreamRef             readStream;

    Boolean open, closed;
    CFErrorRef error;

    /* callbacks when used with a runloop */
    CFOptionFlags               streamEvents;
    CFWriteStreamClientCallBack clientCB;
    
    union
    {
        struct
        {
            int fd; // FIXME: support Windows?
            Boolean append, isSocket, failed;
            CFURLRef url;
        };
        struct
        {
            CFAllocatorRef bufferAllocator;
            UInt8          *buffer;
            CFIndex        bufferCapacity;
            CFIndex        position;
        };
    };    
};

#define CFWRITESTREAM_SIZE (sizeof(struct __CFWriteStream) - sizeof(CFRuntimeBase))

static void
CFWriteStreamFinalize (CFTypeRef cf)
{
  CFWriteStreamRef s = (CFWriteStreamRef)cf;
  
  CFWriteStreamClose(s);

  if (s->error)
    CFRelease(s->error);

  if (s->type == _kCFWriteStreamDescriptor && s->url != NULL)
    CFRelease(s->url);
}

static void
CFWriteStreamSetError(CFWriteStreamRef stream, int error)
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

void
CFStreamInitialize (void)
{
  _kCFWriteStreamTypeID = _CFRuntimeRegisterClass (&CFWriteStreamClass);
}

CFTypeID
CFWriteStreamGetTypeID (void)
{
  return _kCFWriteStreamTypeID;
}

CFTypeID
CFReadStreamGetTypeID (void)
{
  return 0;
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
CFStreamCreatePairWithSocket (CFAllocatorRef alloc, CFSocketNativeHandle sock,
                              CFReadStreamRef *readStream,
                              CFWriteStreamRef *writeStream)
{
  
  *writeStream = (CFWriteStreamRef)
        _CFRuntimeCreateInstance (alloc, _kCFWriteStreamTypeID,
                                  CFWRITESTREAM_SIZE, 0);

  (*writeStream)->type = _kCFWriteStreamDescriptor;
  (*writeStream)->isSocket = true;
  (*writeStream)->fd = dup(sock);

  // TODO: read stream
}

void
CFStreamCreatePairWithSocketToHost (CFAllocatorRef alloc, CFStringRef host,
                                    UInt32 port, CFReadStreamRef *readStream,
                                    CFWriteStreamRef *writeStream)
{
  
}



Boolean
CFWriteStreamCanAcceptBytes (CFWriteStreamRef stream)
{
  return true;
}

void
CFWriteStreamClose (CFWriteStreamRef stream)
{

  if (stream->closed)
    return;
  
  switch (stream->type)
    {
      case _kCFWriteStreamDescriptor:
        if (stream->fd != -1)
          {
            close(stream->fd);
            stream->fd = -1;
          }
        break;
      case _kCFWriteStreamExpandableBuffer:
        if (stream->buffer != NULL)
          {
            CFAllocatorDeallocate(stream->bufferAllocator, stream->buffer);
            stream->buffer = NULL;
          }
      break;
      case _kCFWriteStreamFixedBuffer:
        /* Deallocated by whoever allocated the buffer for us */
        stream->buffer = NULL;
        break;
  };

  stream->closed = true;
  stream->open = false;
}

CFErrorRef
CFWriteStreamCopyError (CFWriteStreamRef stream)
{
  return (CFErrorRef) CFRetain(stream->error);
}

CFTypeRef
CFWriteStreamCopyProperty (CFWriteStreamRef stream, CFStringRef propertyName)
{
  if (stream->type != _kCFWriteStreamDescriptor
      && CFEqual(propertyName, kCFStreamPropertyDataWritten))
    {
      return CFDataCreate(NULL, stream->buffer, stream->position);
    }

  if (stream->type != _kCFWriteStreamDescriptor)
    {
      CFWriteStreamSetError(stream, EINVAL);
      return NULL;
    }

  if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
    {
      long long offset = 0;

      if (!stream->open)
        {
          CFWriteStreamSetError(stream, EBADF);
          return NULL;
        }
          
      if (!stream->isSocket)
        offset = lseek(stream->fd, 0, SEEK_CUR);

      if (offset == (off_t)-1)
        {
          CFWriteStreamSetError(stream, errno);
          return NULL;
        }

      return CFNumberCreate(NULL, kCFNumberLongLongType, &offset);
    }

  if (stream->isSocket)
    {
      if (CFEqual(propertyName, kCFStreamPropertySocketNativeHandle))
        {
          return CFDataCreate(NULL, (UInt8*) &stream->fd, sizeof(stream->fd));
        }
      else if (CFEqual(propertyName, kCFStreamPropertySocketRemoteHostName)
            || CFEqual(propertyName, kCFStreamPropertySocketRemotePortNumber))
       {
         char hbuf[NI_MAXHOST];
         struct sockaddr* sa = NULL;
         socklen_t len = 0;

         getpeername(stream->fd, sa, &len);
         
         if (errno != ENOBUFS)
           {
             CFWriteStreamSetError(stream, errno);
             return NULL;
           }

         sa = (struct sockaddr*) malloc(len);

         if (getpeername(stream->fd, sa, &len) == -1)
           {
             free(sa);

             CFWriteStreamSetError(stream, errno);
             return NULL;
           }

         if (CFEqual(propertyName, kCFStreamPropertySocketRemoteHostName))
           {
             if (getnameinfo(sa, len, hbuf, sizeof(hbuf), NULL, 0, 0) == -1)
               {
                 free(sa);

                 CFWriteStreamSetError(stream, errno);
                 return NULL;
               }

             free(sa);
             return CFStringCreateWithCString(NULL, hbuf, kCFStringEncodingISOLatin1);
           }
         else
           {
             int port = 0;

             if (sa->sa_family == AF_INET)
               port = ntohs( ((struct sockaddr_in*) sa)->sin_port );
             else if (sa->sa_family == AF_INET6)
               port = ntohs( ((struct sockaddr_in6*) sa)->sin6_port );

             free(sa);
             return CFNumberCreate(NULL, kCFNumberIntType, &port);
           }
       }
    }

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
                                                   CFWRITESTREAM_SIZE, 0);
  new->type = _kCFWriteStreamExpandableBuffer;
  new->bufferAllocator = bufferAllocator;

  return new;
}

CFWriteStreamRef
CFWriteStreamCreateWithBuffer (CFAllocatorRef alloc, UInt8 *buffer,
                               CFIndex bufferCapacity)
{
  CFWriteStreamRef new;

  if (buffer == NULL && bufferCapacity > 0)
    return NULL;

  new = (CFWriteStreamRef)_CFRuntimeCreateInstance (alloc, _kCFWriteStreamTypeID,
                                                   CFWRITESTREAM_SIZE, 0);
  new->type = _kCFWriteStreamFixedBuffer;
  new->buffer = buffer;
  new->bufferCapacity = bufferCapacity;

  return new;
}

CFWriteStreamRef
CFWriteStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  CFWriteStreamRef new;
  CFStringRef scheme;
  CFStringRef schemeFile = CFSTR("file");

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
                                                   CFWRITESTREAM_SIZE, 0);
  new->type = _kCFWriteStreamDescriptor;
  new->url = (CFURLRef) CFRetain(fileURL);

  return new;
}

CFStreamError
CFWriteStreamGetError (CFWriteStreamRef stream)
{
  if (!stream->error)
    {
      CFStreamError error = { kCFStreamErrorDomainPOSIX, 0 };
      return error;
    }
  else
    {
      CFStreamError error;
      CFStringRef domain;

      domain = CFErrorGetDomain(stream->error);

      if (CFEqual(domain, kCFErrorDomainPOSIX))
        error.domain = kCFStreamErrorDomainPOSIX;
      else
        error.domain = kCFStreamErrorDomainCustom;

      error.error = CFErrorGetCode(stream->error);
      return error;
    }
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

Boolean
CFWriteStreamOpen (CFWriteStreamRef stream)
{
  if (stream->closed)
    {
      CFWriteStreamSetError(stream, EBADF);
      return false;
    }

  if (stream->type == _kCFWriteStreamDescriptor)
    {
      // TODO: Windows?
      CFStringRef path = CFURLCopyFileSystemPath(stream->url,
                                                 kCFURLPOSIXPathStyle);
      int flags = O_WRONLY | O_CREAT;

      if (stream->append)
        flags |= O_APPEND;

      stream->fd = open(CFStringGetCStringPtr(path, kCFStringEncodingUTF8),
                        flags, 0666);

      if (stream->fd == -1)
      {
        CFWriteStreamSetError(stream, errno);
        return false;
      }
    }

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

Boolean
CFWriteStreamSetProperty (CFWriteStreamRef stream, CFStringRef propertyName,
                          CFTypeRef propertyValue)
{
  if (stream->type == _kCFWriteStreamDescriptor)
    {
      if (CFEqual(propertyName, kCFStreamPropertyAppendToFile))
        {
          if (stream->open)
            {
              CFWriteStreamSetError(stream, EINVAL);
              return false;
            }
          if (CFGetTypeID(propertyValue) != CFBooleanGetTypeID())
            {
              CFWriteStreamSetError(stream, EINVAL);
              return false;
            }

          stream->append = propertyValue == kCFBooleanTrue;
          return true;
        }
      else if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset))
        {
          if (!stream->open)
            {
              CFWriteStreamSetError(stream, EINVAL);
              return false;
            }
          if (CFGetTypeID(propertyValue) != CFNumberGetTypeID())
            {
              CFWriteStreamSetError(stream, EINVAL);
              return false;
            }

          long long offset;

          if (!CFNumberGetValue((CFNumberRef) propertyValue,
                                kCFNumberLongLongType, &offset))
            {
              CFWriteStreamSetError(stream, EINVAL);
              return false;
            }
          
          if (lseek(stream->fd, offset, SEEK_SET) == (off_t)-1)
            {
              CFWriteStreamSetError(stream, errno);
              return false;
            }

          return true;
        }
    }

  CFWriteStreamSetError(stream, EINVAL);
  return false;
}

void
CFWriteStreamUnscheduleFromRunLoop (CFWriteStreamRef stream,
                                    CFRunLoopRef runLoop,
                                    CFStringRef runLoopMode)
{
  
}

CFIndex
CFWriteStreamWrite (CFWriteStreamRef stream, const UInt8 *buffer,
                    CFIndex bufferLength)
{
  if (!stream->open)
    return -1;

  if (stream->type == _kCFWriteStreamDescriptor)
    {
      int wr;

      wr = write(stream->fd, buffer, bufferLength);

      return wr;
    }
  else if (stream->type == _kCFWriteStreamFixedBuffer)
    {
      CFIndex bufSpace = stream->bufferCapacity - stream->position;
      
      if (bufSpace <= 0)
        return 0;

      if (bufferLength > bufSpace)
        bufferLength = bufSpace;

      memcpy(stream->buffer + stream->position, buffer, bufferLength);
      stream->position += bufferLength;

      return bufferLength;
    }
  else if (stream->type == _kCFWriteStreamExpandableBuffer)
    {
      CFIndex bufSpace = stream->bufferCapacity - stream->position;

      if (bufSpace <= 0)
        {
          CFIndex cap = stream->bufferCapacity;
          UInt8 *newbuf;

          while (cap - stream->position < bufferLength)
            cap *= 2;

          newbuf = (UInt8*) CFAllocatorReallocate(stream->bufferAllocator,
                                                  stream->buffer, cap, 0);

          if (!newbuf)
            return -1;

          stream->buffer = newbuf;
          stream->bufferCapacity = cap;
        }

       memcpy(stream->buffer + stream->position, buffer, bufferLength);
       stream->position += bufferLength;

       return bufferLength;
    }
  return -1;
}



void
CFReadStreamClose (CFReadStreamRef stream)
{
  
}

CFErrorRef
CFReadStreamCopyError (CFReadStreamRef stream)
{
  return NULL;
}

CFTypeRef
CFReadStreamCopyProperty (CFReadStreamRef stream, CFStringRef propertyName)
{
  return NULL;
}

CFReadStreamRef
CFReadStreamCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
                                   CFIndex length,
                                   CFAllocatorRef bytesDeallocator)
{
  return NULL;
}

CFReadStreamRef
CFReadStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  return NULL;
}

const UInt8 *
CFReadStreamGetBuffer (CFReadStreamRef stream, CFIndex maxBytesToRead,
                       CFIndex *numBytesRead)
{
  return NULL;
}

CFStreamError
CFReadStreamGetError (CFReadStreamRef stream)
{
  CFStreamError error = { 0, 0 };
  return error;
}

CFStreamStatus
CFReadStreamGetStatus (CFReadStreamRef stream)
{
  return kCFStreamStatusError;
}

Boolean
CFReadStreamHasBytesAvailable (CFReadStreamRef stream)
{
  return false;
}

Boolean
CFReadStreamOpen (CFReadStreamRef stream)
{
  return false;
}

CFIndex
CFReadStreamRead (CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength)
{
  return 0;
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

Boolean
CFReadStreamSetProperty (CFReadStreamRef stream, CFStringRef propertyName,
                         CFTypeRef propertyValue)
{
  return false;
}

void
CFReadStreamUnscheduleFromRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                   CFStringRef runLoopMode)
{
  
}

