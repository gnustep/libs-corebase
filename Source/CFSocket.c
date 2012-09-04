/* CFSocket.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Author: Stefan Bidigaray <stefanbidi@gmail.com>
   Date: September, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFSocket.h"
#include "GSPrivate.h"

static CFTypeID _kCFSocketTypeID = 0;

struct __CFSocket
{
  CFRuntimeBase    parent;
  CFSocketNativeHandle _socket;
  CFOptionFlags    _callbackTypes;
  CFSocketCallBack _callback;
  CFSocketContext  _context;
};

CFTypeID
CFSocketGetTypeID (void)
{
  return _kCFSocketTypeID;
}

CFSocketRef
CFSocketCreate (CFAllocatorRef allocator, SInt32 protocolFamily,
                SInt32 socketType, SInt32 protocol,
                CFOptionFlags callBackTypes, CFSocketCallBack callback,
                const CFSocketContext *context)
{
  return NULL;
}

CFSocketRef
CFSocketCreateConnectedToSocketSignature (CFAllocatorRef allocator,
                                          const CFSocketSignature *signature,
                                          CFOptionFlags callBackTypes,
                                          CFSocketCallBack callback,
                                          const CFSocketContext *context,
                                          CFTimeInterval timeout)
{
  return NULL;
}

CFSocketRef
CFSocketCreateWithNative (CFAllocatorRef allocator, CFSocketNativeHandle sock,
                          CFOptionFlags callBackTypes,
                          CFSocketCallBack callback,
                          const CFSocketContext *context)
{
  return NULL;
}

CFSocketRef
CFSocketCreateWithSocketSignature (CFAllocatorRef allocator,
                                   const CFSocketSignature *signature,
                                   CFOptionFlags callBackTypes,
                                   CFSocketCallBack callback,
                                   const CFSocketContext *context)
{
  return NULL;
}

CFDataRef
CFSocketCopyAddress (CFSocketRef s)
{
  return NULL;
}

CFDataRef
CFSocketCopyPeerAddress (CFSocketRef s)
{
  return NULL;
}

void
CFSocketGetContext (CFSocketRef s, CFSocketContext *context)
{
  return;
}

CFSocketNativeHandle
CFSocketGetNative (CFSocketRef s)
{
  return 0;
}

CFSocketError
CFSocketSetAddress (CFSocketRef s, CFDataRef address)
{
  return kCFSocketError;
}

void
CFSocketDisableCallBacks (CFSocketRef s, CFOptionFlags callBackTypes)
{
  return;
}

void
CFSocketEnableCallBacks (CFSocketRef s, CFOptionFlags callBackTypes)
{
  return;
}

CFOptionFlags
CFSocketGetSocketFlags (CFSocketRef s)
{
  return;
}

void
CFSocketSetSocketFlags (CFSocketRef s, CFOptionFlags flags)
{
  return;
}

CFSocketError
CFSocketConnectToAddress (CFSocketRef s, CFDataRef address,
                          CFTimeInterval timeout)
{
  return kCFSocketError;
}

CFRunLoopSourceRef
CFSocketCreateRunLoopSource (CFAllocatorRef allocator, CFSocketRef s,
                             CFIndex order)
{
  return NULL;
}

void
CFSocketInvalidate (CFSocketRef s)
{
  return;
}

Boolean
CFSocketIsValid (CFSocketRef s)
{
  return false;
}

CFSocketError
CFSocketSendData (CFSocketRef s, CFDataRef address, CFDataRef data,
                  CFTimeInterval timeout)
{
  return kCFSocketError;
}

CFSocketError
CFSocketCopyRegisteredSocketSignature (const CFSocketSignature *nameServerSignature,
                                       CFTimeInterval timeout,
                                       CFStringRef name,
                                       CFSocketSignature *signature,
                                       CFDataRef *nameServerAddress)
{
  return kCFSocketError;
}

CFSocketError
CFSocketCopyRegisteredValue (const CFSocketSignature *nameServerSignature,
                             CFTimeInterval timeout,
                             CFStringRef name,
                             CFPropertyListRef *value,
                             CFDataRef *nameServerAddress)
{
  return kCFSocketError;
}

UInt16
CFSocketGetDefaultNameRegistryPortNumber (void)
{
  return 0;
}

CFSocketError
CFSocketRegisterSocketSignature (const CFSocketSignature *nameServerSignature,
                                 CFTimeInterval timeout,
                                 CFStringRef name,
                                 const CFSocketSignature *signature)
{
  return kCFSocketError;
}

CFSocketError
CFSocketRegisterValue (const CFSocketSignature *nameServerSignature,
                       CFTimeInterval timeout,
                       CFStringRef name,
                       CFPropertyListRef value)
{
  return kCFSocketError;
}

void
CFSocketSetDefaultNameRegistryPortNumber (UInt16 port)
{
  return;
}

CFSocketError
CFSocketUnregister (const CFSocketSignature *nameServerSignature,
                    CFTimeInterval timeout, CFStringRef name)
{
  return kCFSocketError;
}

