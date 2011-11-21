/* CFStream.h
   
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

#ifndef __COREFOUNDATION_CFSTREAM_H__
#define __COREFOUNDATION_CFSTREAM_H__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFURL.h>

typedef struct CFWriteStream *CFWriteStreamRef;
typedef struct CFReadStream *CFReadStreamRef;



#if MAC_OS_X_VERSION_10_1 <= MAC_OS_X_VERSION_MAX_ALLOWED

const CFStringRef kCFStreamPropertyDataWritten;
const CFStringRef kCFStreamPropertySocketNativeHandle;
const CFStringRef kCFStreamPropertySocketRemoteHostName;
const CFStringRef kCFStreamPropertySocketRemotePortNumber;
#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
const CFStringRef kCFStreamPropertyAppendToFile;
#endif
#if MAC_OS_X_VERSION_10_3 <= MAC_OS_X_VERSION_MAX_ALLOWED
const CFStringRef kCFStreamPropertyFileCurrentOffset;
#endif

struct CFStreamClientContext
{
  CFIndex version;
  void *info;
  CFAllocatorRetainCallBack retain;
  CFAllocatorReleaseCallBack release;
  CFAllocatorCopyDescriptionCallBack copyDescription;
};
typedef struct CFStreamClientContext CFStreamClientContext;

enum _CFStreamStatus
{
  kCFStreamStatusNotOpen = 0,
  kCFStreamStatusOpening,
  kCFStreamStatusOpen,
  kCFStreamStatusReading,
  kCFStreamStatusWriting,
  kCFStreamStatusAtEnd,
  kCFStreamStatusClosed,
  kCFStreamStatusError
};
typedef enum _CFStreamStatus CFStreamStatus;

// Deprecated
enum _CFStreamErrorDomain
{
  kCFStreamErrorDomainCustom = -1,
  kCFStreamErrorDomainPOSIX = 1,
  kCFStreamErrorDomainMacOSStatus
};
typedef enum _CFStreamErrorDomain CFStreamErrorDomain;


struct _CFStreamError
{
  CFStreamErrorDomain domain;
  SInt32 error;
};
typedef struct _CFStreamError CFStreamError;

enum _CFStreamEventType
{
  kCFStreamEventNone = 0,
  kCFStreamEventOpenCompleted = 1,
  kCFStreamEventHasBytesAvailable = 2,
  kCFStreamEventCanAcceptBytes = 4,
  kCFStreamEventErrorOccurred = 8,
  kCFStreamEventEndEncountered = 16
};
typedef enum _CFStreamEventType CFStreamEventType;

typedef void (*CFWriteStreamClientCallBack) (CFWriteStreamRef stream,
  CFStreamEventType eventType, void *clientCallBackInfo);

typedef void (*CFReadStreamClientCallBack) (CFReadStreamRef stream,
  CFStreamEventType eventType, void *clientCallBackInfo);



//
// NSStream functions
//
/* FIXME: need CFSocket suff...
void
CFStreamCreatePairWithSocket (CFAllocatorRef alloc, CFSocketNativeHandle sock,
                              CFReadStreamRef *readStream,
                              CFWriteStreamRef *writeStream);
*/
void
CFStreamCreatePairWithSocketToHost (CFAllocatorRef alloc, CFStringRef host,
                                    UInt32 port, CFReadStreamRef *readStream,
                                    CFWriteStreamRef *writeStream);

//
// Creating a Write Stream
//
CFWriteStreamRef
CFWriteStreamCreateWithAllocatedBuffers (CFAllocatorRef alloc,
                                         CFAllocatorRef bufferAllocator);

CFWriteStreamRef
CFWriteStreamCreateWithBuffer (CFAllocatorRef alloc, UInt8 *buffer,
                               CFIndex bufferCapacity);

CFWriteStreamRef
CFWriteStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL);

//
// Opening and Closing a Write Stream
//
void
CFWriteStreamClose (CFWriteStreamRef stream);

Boolean
CFWriteStreamOpen (CFWriteStreamRef stream);

//
// Writing to a Stream
//
CFIndex
CFWriteStreamWrite (CFWriteStreamRef stream, const UInt8 *buffer,
                    CFIndex bufferLength);

//
// Scheduling a Write Stream
//
void
CFWriteStreamScheduleWithRunLoop (CFWriteStreamRef stream,
                                  CFRunLoopRef runLoop,
                                  CFStringRef runLoopMode);

void
CFWriteStreamUnscheduleFromRunLoop (CFWriteStreamRef stream,
                                    CFRunLoopRef runLoop,
                                    CFStringRef runLoopMode);

//
// Examining Write Stream Properties
//
Boolean
CFWriteStreamCanAcceptBytes (CFWriteStreamRef stream);

CFTypeRef
CFWriteStreamCopyProperty (CFWriteStreamRef stream, CFStringRef propertyName);

// Deprecated function
CFStreamError
CFWriteStreamGetError (CFWriteStreamRef stream);

CFStreamStatus
CFWriteStreamGetStatus (CFWriteStreamRef stream);

//
// Setting Write Stream Properties
//
Boolean
CFWriteStreamSetClient (CFWriteStreamRef stream, CFOptionFlags streamEvents,
                        CFWriteStreamClientCallBack clientCB,
                        CFStreamClientContext *clientContext);

//
// Getting the CFWriteStream Type ID
//
CFTypeID
CFWriteStreamGetTypeID (void);

//
// Creating a Read Stream
//
CFReadStreamRef
CFReadStreamCreateWithBytesNoCopy (CFAllocatorRef alloc, const UInt8 *bytes,
                                   CFIndex length, CFAllocatorRef bytesDeallocator);

CFReadStreamRef
CFReadStreamCreateWithFile (CFAllocatorRef alloc, CFURLRef fileURL);

//
// Opening and Closing a Read Stream
//
void
CFReadStreamClose (CFReadStreamRef stream);

Boolean
CFReadStreamOpen (CFReadStreamRef stream);

//
// Reading from a Read Stream
//
CFIndex
CFReadStreamRead (CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength);

//
// Scheduling a Read Stream
//
void
CFReadStreamScheduleWithRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                 CFStringRef runLoopMode);

void
CFReadStreamUnscheduleFromRunLoop (CFReadStreamRef stream, CFRunLoopRef runLoop,
                                   CFStringRef runLoopMode);

//
// Examining Stream Properties
//
CFTypeRef
CFReadStreamCopyProperty (CFReadStreamRef stream, CFStringRef propertyName);

const UInt8 *
CFReadStreamGetBuffer (CFReadStreamRef stream, CFIndex maxBytesToRead,
                       CFIndex *numBytesRead);

// Deprecated function
CFStreamError
CFReadStreamGetError (CFReadStreamRef stream);


CFStreamStatus
CFReadStreamGetStatus (CFReadStreamRef stream);

Boolean
CFReadStreamHasBytesAvailable (CFReadStreamRef stream);

//
// Setting Stream Properties
//
Boolean
CFReadStreamSetClient (CFReadStreamRef stream, CFOptionFlags streamEvents,
                       CFReadStreamClientCallBack clientCB,
                       CFStreamClientContext *clientContext);

//
// Getting the CFReadStream Type ID
//
CFTypeID
CFReadStreamGetTypeID (void);



#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
/* FIXME: need CFSocket stuff
void
CFStreamCreatePairWithPeerSocketSignature (CFAllocatorRef alloc,
                                           const CFSocketSignature *signature,
                                           CFReadStreamRef *readStream,
                                           CFWriteStreamRef *writeStream);
*/
Boolean
CFWriteStreamSetProperty (CFWriteStreamRef stream, CFStringRef propertyName,
                          CFTypeRef propertyValue);

Boolean
CFReadStreamSetProperty (CFReadStreamRef stream, CFStringRef propertyName,
                         CFTypeRef propertyValue);
#endif

#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED
void
CFStreamCreateBoundPair (CFAllocatorRef alloc, CFReadStreamRef *readStream,
                         CFWriteStreamRef *writeStream, CFIndex transferBufferSize);

CFErrorRef
CFWriteStreamCopyError (CFWriteStreamRef stream);

CFErrorRef
CFReadStreamCopyError (CFReadStreamRef stream);
#endif

#endif /* MAC_OS_X_VERSION_10_1 */

#endif /* __COREFOUNDATION_CFSTREAM_H__ */
