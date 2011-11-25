/* CFStream.m
   
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

#import <Foundation/NSStream.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSURL.h>

#include "CoreFoundation/CFStream.h"

void
_CFStreamClose (CFTypeRef stream)
{
  [(NSStream *)stream close];
}

CFErrorRef
_CFStreamCopyError (CFTypeRef stream)
{
  return (CFErrorRef)CFRetain([(NSStream *)stream streamError]);
}

CFTypeRef
_CFStreamCopyProperty (CFTypeRef stream, CFStringRef propertyName)
{
  return CFRetain([(NSStream *)stream propertyForKey: (NSString *)propertyName]);
}

Boolean
_CFStreamSetProperty (CFTypeRef stream, CFStringRef propertyName,
                      CFTypeRef propertyValue)
{
  return (Boolean)[(NSStream *)stream setProperty: (id)propertyValue
                                           forKey: (NSString *)propertyName];
}

CFStreamError
_CFStreamGetError (CFTypeRef stream)
{
  // FIXME
  return (CFStreamError){0, 0};
}

CFStreamStatus
_CFStreamGetStatus (CFTypeRef stream)
{
  return [(NSStream *)stream streamStatus];
}

Boolean
_CFStreamOpen (CFTypeRef stream)
{
  [(NSStream *)stream open];
  return (_CFStreamGetStatus (stream) == kCFStreamStatusOpen ? TRUE : FALSE);
}

void
_CFStreamScheduleInRunLoop (CFTypeRef stream, CFRunLoopRef runLoop,
                              CFStringRef runLoopMode)
{
  [(NSStream *)stream scheduleInRunLoop: (NSRunLoop *)runLoop
                                forMode: (NSString *)runLoopMode];
}

void
_CFStreamUnscheduleFromRunLoop (CFTypeRef stream, CFRunLoopRef runLoop,
                               CFStringRef runLoopMode)
{
  [(NSStream *)stream removeFromRunLoop: (NSRunLoop *)runLoop
                                forMode: (NSString *)runLoopMode];
}



void
CFStreamCreateBoundPair (CFAllocatorRef alloc, CFReadStreamRef *readStream,
                         CFWriteStreamRef *writeStream, CFIndex transferBufferSize)
{
  // FIXME
}

/*void
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
  [NSStream getStreamsToHost: [NSHost hostWithName: (NSString *)host]
                        port: (NSUInteger)port
                 inputStream: (NSInputStream **)readStream
                outputStream: (NSOutputStream **)writeStream];
}



Boolean
CFWriteStreamCanAcceptBytes (CFWriteStreamRef stream)
{
  return (Boolean)[(NSOutputStream *)stream hasSpaceAvailable];
}

void
CFWriteStreamClose (CFWriteStreamRef stream)
{
  _CFStreamClose (stream);
}

CFErrorRef
CFWriteStreamCopyError (CFWriteStreamRef stream)
{
  return _CFStreamCopyError (stream);
}

CFTypeRef
CFWriteStreamCopyProperty (CFWriteStreamRef stream, CFStringRef propertyName)
{
  return _CFStreamCopyProperty (stream, propertyName);
}

CFWriteStreamRef
CFWriteStreamCreateWithAllocatedBuffers (CFAllocatorRef alloc,
                                         CFAllocatorRef bufferAllocator)
{
  // FIXME: ???
  return NULL;
}

CFWriteStreamRef
CFWriteStreamCreateWithBuffer (CFAllocatorRef alloc, UInt8 *buffer,
                               CFIndex bufferCapacity)
{
  return (CFWriteStreamRef)[[NSOutputStream alloc]
                             initToBuffer: buffer
                                 capacity: (NSUInteger)bufferCapacity];
}

CFWriteStreamRef
CFWriteStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  // FIXME: there's nothing in -base to set the append option at a later time.
  return (CFWriteStreamRef)[[NSOutputStream alloc]
                             initToFileAtPath: [(NSURL *)fileURL absoluteString]
                                       append: NO];
}

CFStreamError
CFWriteStreamGetError (CFWriteStreamRef stream)
{
  return _CFStreamGetError (stream);
}

CFStreamStatus
CFWriteStreamGetStatus (CFWriteStreamRef stream)
{
  return _CFStreamGetStatus (stream);
}

CFTypeID
CFWriteStreamGetTypeID (void)
{
  return (CFTypeID)[NSOutputStream class];
}

Boolean
CFWriteStreamOpen (CFWriteStreamRef stream)
{
  return _CFStreamOpen (stream);
}

void
CFWriteStreamScheduleWithRunLoop (CFWriteStreamRef stream,
                                  CFRunLoopRef runLoop,
                                  CFStringRef runLoopMode)
{
  _CFStreamScheduleInRunLoop (stream, runLoop, runLoopMode);
}

Boolean
CFWriteStreamSetClient (CFWriteStreamRef stream, CFOptionFlags streamEvents,
                        CFWriteStreamClientCallBack clientCB,
                        CFStreamClientContext *clientContext)
{
  // FIXME
  return FALSE;
}

Boolean
CFWriteStreamSetProperty (CFWriteStreamRef stream, CFStringRef propertyName,
                          CFTypeRef propertyValue)
{
  return _CFStreamSetProperty (stream, propertyName, propertyValue);
}

void
CFWriteStreamUnscheduleFromRunLoop (CFWriteStreamRef stream,
                                    CFRunLoopRef runLoop,
                                    CFStringRef runLoopMode)
{
  _CFStreamUnscheduleFromRunLoop (stream, runLoop, runLoopMode);
}

CFIndex
CFWriteStreamWrite (CFWriteStreamRef stream, const UInt8 *buffer,
                    CFIndex bufferLength)
{
  return (CFIndex)[(NSOutputStream *)stream write: buffer
                                        maxLength: (NSUInteger)bufferLength];
}



void
CFReadStreamClose (CFReadStreamRef stream)
{
  [(NSInputStream *)stream close];
}

CFErrorRef
CFReadStreamCopyError (CFReadStreamRef stream)
{
  return (CFErrorRef)[(NSInputStream *)stream streamError];
}

CFTypeRef
CFReadStreamCopyProperty (CFReadStreamRef stream, CFStringRef propertyName)
{
  return _CFStreamCopyProperty (stream, propertyName);
}

CFReadStreamRef
CFReadStreamCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
                                   CFIndex length, CFAllocatorRef bytesDeallocator)
{
  // FIXME
  return NULL;
}

CFReadStreamRef
CFReadStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL)
{
  return (CFReadStreamRef)[[NSInputStream alloc]
                           initWithFileAtPath: [(NSURL *)fileURL absoluteString]];
}

const UInt8 *
CFReadStreamGetBuffer (CFReadStreamRef stream, CFIndex maxBytesToRead,
                       CFIndex *numBytesRead)
{
  // FIXME: docs are  bit confusing
  return NULL;
}

CFStreamError
CFReadStreamGetError (CFReadStreamRef stream)
{
  return _CFStreamGetError (stream);
}

CFStreamStatus
CFReadStreamGetStatus (CFReadStreamRef stream)
{
  return (CFStreamStatus)[(NSInputStream *)stream streamStatus];
}

CFTypeID
CFReadStreamGetTypeID (void)
{
  return (CFTypeID)[NSInputStream class];
}

Boolean
CFReadStreamHasBytesAvailable (CFReadStreamRef stream)
{
  return [(NSInputStream *)stream hasBytesAvailable];
}

Boolean
CFReadStreamOpen (CFReadStreamRef stream)
{
  return _CFStreamOpen (stream);
}

CFIndex
CFReadStreamRead (CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength)
{
  return (CFIndex)[(NSInputStream *)stream read: buffer
                                      maxLength: (NSUInteger)bufferLength];
}

void
CFReadStreamScheduleWithRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                 CFStringRef runLoopMode)
{
  _CFStreamScheduleInRunLoop (stream, runLoop, runLoopMode);
}

Boolean
CFReadStreamSetClient (CFReadStreamRef stream, CFOptionFlags streamEvents,
                       CFReadStreamClientCallBack clientCB,
                       CFStreamClientContext *clientContext)
{
  // FIXME
  return FALSE;
}

Boolean
CFReadStreamSetProperty (CFReadStreamRef stream, CFStringRef propertyName,
                         CFTypeRef propertyValue)
{
  return _CFStreamSetProperty (stream, propertyName, propertyValue);
}

void
CFReadStreamUnscheduleFromRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                   CFStringRef runLoopMode)
{
  _CFStreamUnscheduleFromRunLoop (stream, runLoop, runLoopMode);
}
