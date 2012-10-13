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

#include <math.h>

#if defined(_WIN32)
#include <winsock2.h>
typedef socklen_t int;
#define EINPROGRESS WSAEINPROGRESS

CF_INLINE int
CFSocketGetLastError (void)
{
  return WSAGetLastError ();
}
#else
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#define closesocket(x) close(x)

CF_INLINE int
CFSocketGetLastError (void)
{
  return errno;
}
#endif

struct __CFSocket
{
  CFRuntimeBase    parent;
  GSMutex          _lock;
  CFSocketNativeHandle _socket;
  CFOptionFlags    _opts;
  CFOptionFlags    _cbTypes;
  CFSocketCallBack _callback;
  CFSocketContext  _ctx;
  CFDataRef        _address;
  CFDataRef        _peerAddress;
};

static CFTypeID _kCFSocketTypeID = 0;
static CFMutableDictionaryRef _kCFSocketObjects = NULL;
static GSMutex _kCFSocketObjectsLock;

static void
CFSocketFinalize (CFTypeRef cf)
{
  CFSocketRef s = (CFSocketRef)cf;
  
  closesocket (s->_socket);
  if (s->_address)
    CFRelease (s->_address);
  if (s->_peerAddress)
    CFRelease (s->_peerAddress);
}

static CFRuntimeClass CFSocketClass =
{
  0,
  "CFSocket",
  NULL,
  NULL,
  CFSocketFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void
CFSocketInitialize (void)
{
  _kCFSocketTypeID = _CFRuntimeRegisterClass (&CFSocketClass);
  GSMutexInitialize (&_kCFSocketObjectsLock);
}




CFTypeID
CFSocketGetTypeID (void)
{
  return _kCFSocketTypeID;
}

#define CFSOCKET_SIZE sizeof(struct __CFSocket) - sizeof(CFRuntimeBase)

CFSocketRef
CFSocketCreateWithNative (CFAllocatorRef alloc, CFSocketNativeHandle sock,
                          CFOptionFlags cbTypes,
                          CFSocketCallBack callback,
                          const CFSocketContext *ctx)
{
  CFSocketRef new = NULL;
  
  GSMutexLock (&_kCFSocketObjectsLock);
  
  if (_kCFSocketObjects == NULL)
    {
      _kCFSocketObjects = CFDictionaryCreateMutable (kCFAllocatorSystemDefault,
                            0, NULL,  &kCFTypeDictionaryValueCallBacks);
    }
  if (CFDictionaryGetValueIfPresent (_kCFSocketObjects,
                                     (const void*)(uintptr_t)sock,
                                     (const void**)&new))
    {
      GSMutexUnlock (&_kCFSocketObjectsLock);
      return new;
    }
  
  if (new == NULL)
    {
      new = (CFSocketRef)_CFRuntimeCreateInstance (alloc, _kCFSocketTypeID,
                                                   CFSOCKET_SIZE, 0);
      if (new != NULL)
        {
          new->_socket = sock;
          new->_cbTypes = cbTypes;
          new->_callback = callback;
          if (ctx != NULL)
            {
              if (ctx->info != NULL)
                new->_ctx.info = (void*)(ctx->retain ? ctx->retain (ctx->info)
                                                     : ctx->info);
              new->_ctx.retain = ctx->retain;
              new->_ctx.release = ctx->release;
              new->_ctx.copyDescription = ctx->copyDescription;
            }
          CFDictionaryAddValue (_kCFSocketObjects,
                                (const void*)(uintptr_t)sock, new);
        }
    }
  
  GSMutexUnlock (&_kCFSocketObjectsLock);
  
  return new;
}

#if defined(_WIN32)
static Boolean _kWinsockInitialized = false;
#define WINSOCK_MAJOR_VERSION 2
#define WINSOCK_MINOR_VERSION 2
#endif

CFSocketRef
CFSocketCreate (CFAllocatorRef alloc, SInt32 protocolFamily,
                SInt32 socketType, SInt32 protocol,
                CFOptionFlags cbTypes, CFSocketCallBack callback,
                const CFSocketContext *ctx)
{
  CFSocketRef new = NULL;
  CFSocketNativeHandle s;
#if defined(_WIN32)
  if (_kWinsockInitialized == false)
    {
      WORD winsockVersionRequested;
      WSADATA winsockData;
      int winsockErr;
      
      _kWinsockInitialized = true;
      /* High-byte = minor && low-byte = major */
      winsockVersionRequested = MAKEWORD(WINSOCK_MAJOR_VERSION,
                                         WINSOCK_MINOR_VERSION);
      winsockErr = WSAStartup (winsockVersionRequested, &winsockData);
      if (winsockErr != 0)
        return NULL;
      if (LOBYTE(wsaData.wVersion) != WINSOCK_MAJOR_VERSION
          || HIBYTE(wsaData.wVersion) != WINSOCK_MINOR_VERSION)
        {
          WSACleanup ();
          _kWinsockInitialized = false;
          return NULL;
        }
    }
#endif
  
  s = socket (protocolFamily, socketType, protocol);
  if (s != INVALID_SOCKET)
    new = CFSocketCreateWithNative (alloc, s, cbTypes, callback, ctx);
  
  return new;
}

CFSocketRef
CFSocketCreateWithSocketSignature (CFAllocatorRef alloc,
                                   const CFSocketSignature *sign,
                                   CFOptionFlags cbTypes,
                                   CFSocketCallBack callback,
                                   const CFSocketContext *ctx)
{
  CFSocketRef new;
  new = CFSocketCreate (alloc, sign->protocolFamily, sign->socketType,
                        sign->protocol, cbTypes, callback, ctx);
  if (new)
    {
      CFSocketError err;
      err = CFSocketSetAddress (new, sign->address);
      if (err != kCFSocketSuccess)
        {
          CFRelease (new);
          new = NULL;
        }
    }
  
  return new;
}

CFSocketRef
CFSocketCreateConnectedToSocketSignature (CFAllocatorRef alloc,
                                          const CFSocketSignature *sign,
                                          CFOptionFlags cbTypes,
                                          CFSocketCallBack callback,
                                          const CFSocketContext *ctx,
                                          CFTimeInterval timeout)
{
  CFSocketRef new;
  new = CFSocketCreate (alloc, sign->protocolFamily, sign->socketType,
                        sign->protocol, cbTypes, callback, ctx);
  if (new)
    {
      CFSocketError err;
      err = CFSocketConnectToAddress (new, sign->address, timeout);
      if (err != kCFSocketSuccess)
        {
          CFRelease (new);
          new = NULL;
        }
    }
  
  return new;
}

CFDataRef
CFSocketCopyAddress (CFSocketRef s)
{
  CFDataRef ret = NULL;
  
  GSMutexLock (&s->_lock);
  if (s->_address == NULL)
    {
      struct sockaddr addr;
      socklen_t addrlen;
      getsockname (s->_socket, &addr, &addrlen);
      s->_address = CFDataCreate (CFGetAllocator (s), (const UInt8*)&addr,
                                  (CFIndex)addrlen);
    }
  if (s->_address != NULL)
    ret = CFRetain (s->_address);
  GSMutexUnlock (&s->_lock);
  
  return ret;
}

CFDataRef
CFSocketCopyPeerAddress (CFSocketRef s)
{
  CFDataRef ret = NULL;
  
  GSMutexLock (&s->_lock);
  if (s->_address == NULL)
    {
      struct sockaddr addr;
      socklen_t addrlen;
      getpeername (s->_socket, &addr, &addrlen);
      s->_address = CFDataCreate (CFGetAllocator (s), (const UInt8*)&addr,
                                  (CFIndex)addrlen);
    }
  if (s->_address != NULL)
    ret = CFRetain (s->_address);
  GSMutexUnlock (&s->_lock);
  
  return ret;
}

CFSocketError
CFSocketSetAddress (CFSocketRef s, CFDataRef address)
{
  CFSocketNativeHandle sock;
  struct sockaddr *addr;
  socklen_t addrlen;
  int err;
  
  if (CFSocketIsValid (s) == false || address == NULL)
    return kCFSocketError;
  
  addr = (struct sockaddr*)CFDataGetBytePtr (address);
  addrlen = CFDataGetLength (address);
  if (addr == NULL || addrlen == 0)
    return kCFSocketError;
  sock = CFSocketGetNative (s);
  
  err = bind (sock, addr, addrlen);
  if (err == 0)
    {
      listen (sock, 1024);
      return kCFSocketSuccess;
    }
  return kCFSocketError;
}

CFSocketError
CFSocketConnectToAddress (CFSocketRef s, CFDataRef address,
                          CFTimeInterval timeout)
{
  CFSocketNativeHandle sock;
  struct sockaddr *addr;
  socklen_t addrlen;
  int err;
  
  if (CFSocketIsValid (s) == false || address == NULL)
    return kCFSocketError;
  
  addr = (struct sockaddr*)CFDataGetBytePtr (address);
  addrlen = CFDataGetLength (address);
  if (addr == NULL || addrlen == 0)
    return kCFSocketError;
  sock = CFSocketGetNative (s);
  
  if (timeout < 0.0)
    {
      int f;
      f = fcntl (sock, F_GETFL, 0);
      f |= O_NONBLOCK;
      if (fcntl (sock, F_SETFL, f) != 0)
        return kCFSocketError;
    }
  err = connect (sock, addr, addrlen);
  if (err != 0 && CFSocketGetLastError () == EINPROGRESS)
    {
      if (timeout >= 0.0)
        {
          struct timeval tv;
          fd_set writefds[FD_SETSIZE];
          tv.tv_sec = (long)floor (timeout);
          tv.tv_usec = (long)((timeout - floor (timeout)) * 1000000.0);
          FD_ZERO(writefds);
          FD_SET(sock, writefds);
          err = select (sock + 1, NULL, writefds, NULL, &tv);
        }
      else
        {
          err = 0;
        }
    }
  return err == 0 ? kCFSocketSuccess : kCFSocketError;
}

void
CFSocketGetContext (CFSocketRef s, CFSocketContext *ctx)
{
  *ctx = s->_ctx;
}

CFSocketNativeHandle
CFSocketGetNative (CFSocketRef s)
{
  return s->_socket;
}

void
CFSocketDisableCallBacks (CFSocketRef s, CFOptionFlags cbTypes)
{
  return;
}

void
CFSocketEnableCallBacks (CFSocketRef s, CFOptionFlags cbTypes)
{
  return;
}

CFOptionFlags
CFSocketGetSocketFlags (CFSocketRef s)
{
  return s->_opts;
}

void
CFSocketSetSocketFlags (CFSocketRef s, CFOptionFlags flags)
{
  s->_opts = flags;
}

CFSocketError
CFSocketSendData (CFSocketRef s, CFDataRef address, CFDataRef data,
                  CFTimeInterval timeout)
{
  return kCFSocketError;
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

CFRunLoopSourceRef
CFSocketCreateRunLoopSource (CFAllocatorRef alloc, CFSocketRef s,
                             CFIndex order)
{
  return NULL;
}



CFSocketError
CFSocketCopyRegisteredSocketSignature (const CFSocketSignature *nameServerSignature,
                                       CFTimeInterval timeout,
                                       CFStringRef name,
                                       CFSocketSignature *sign,
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
                                 const CFSocketSignature *sign)
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

