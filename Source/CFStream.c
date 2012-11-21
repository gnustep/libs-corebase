/* CFStream.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: August, 2012
   
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

CFTypeID
CFWriteStreamGetTypeID (void)
{
  return 0;
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

void
CFStreamCreatePairWithSocket (CFAllocatorRef alloc, CFSocketNativeHandle sock,
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
  return false;
}

void
CFWriteStreamClose (CFWriteStreamRef stream)
{
  
}

CFErrorRef
CFWriteStreamCopyError (CFWriteStreamRef stream)
{
  return NULL;
}

CFTypeRef
CFWriteStreamCopyProperty (CFWriteStreamRef stream, CFStringRef propertyName)
{
  return NULL;
}

CFWriteStreamRef
CFWriteStreamCreateWithAllocatedBuffers (CFAllocatorRef alloc,
                                         CFAllocatorRef bufferAllocator)
{
  return NULL;
}

CFWriteStreamRef
CFWriteStreamCreateWithBuffer (CFAllocatorRef alloc, UInt8 *buffer,
                               CFIndex bufferCapacity)
{
  return NULL;
}

CFWriteStreamRef
CFWriteStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  return NULL;
}

CFStreamError
CFWriteStreamGetError (CFWriteStreamRef stream)
{
  CFStreamError error = { 0, 0 };
  return error;
}

CFStreamStatus
CFWriteStreamGetStatus (CFWriteStreamRef stream)
{
  return kCFStreamStatusError;
}

Boolean
CFWriteStreamOpen (CFWriteStreamRef stream)
{
  return false;
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
  return 0;
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

