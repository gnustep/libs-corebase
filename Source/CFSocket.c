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

#include "config.h"
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFSocket.h"
#include "GSPrivate.h"
#if HAVE_LIBDISPATCH
#	include <dispatch/dispatch.h>
#endif

#include <math.h>
#include <stdio.h>

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
  Boolean          _isConnected;
  Boolean          _isListening;
  CFRunLoopSourceRef _source;
#if HAVE_LIBDISPATCH
  dispatch_source_t  _readSource;
  Boolean            _readFired;
  Boolean            _readResumed;
  dispatch_source_t  _writeSource;
  Boolean            _writeFired;
  Boolean            _writeResumed;
#endif
};

static CFTypeID _kCFSocketTypeID = 0;
static CFMutableDictionaryRef _kCFSocketObjects = NULL;
static GSMutex _kCFSocketObjectsLock;

static void
DummyBarrier (void* dummy)
{
}

static void
CFSocketFinalize (CFTypeRef cf)
{
  CFSocketRef s = (CFSocketRef)cf;
  
#if HAVE_LIBDISPATCH
  dispatch_queue_t q = dispatch_get_global_queue(
              DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  dispatch_source_cancel(s->_readSource);
  dispatch_source_cancel(s->_writeSource);
  
  // Wait for source handlers to complete
  // before we proceed to destruction.
  dispatch_barrier_sync_f(q, NULL, DummyBarrier);
  
  if (s->_source != NULL)
    CFRelease(s->_source);
#endif
  
  if (s->_socket != -1)
    {
      GSMutexLock (&_kCFSocketObjectsLock);
      CFDictionaryRemoveValue(_kCFSocketObjects,
                              (void*)(uintptr_t) s->_socket);
      closesocket (s->_socket);
      GSMutexUnlock (&_kCFSocketObjectsLock);
    }
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

#if HAVE_LIBDISPATCH
static void
CFSocketDispatchReadEvent(void* p)
{
  CFSocketRef socket = (CFSocketRef) p;
  CFRunLoopSourceRef src = socket->_source;
  
  socket->_readFired = true;
  
  if (src != NULL)
    CFRunLoopSourceSignal(src);
}

static void
CFSocketDispatchWriteEvent(void* p)
{
  CFSocketRef socket = (CFSocketRef) p;
  CFRunLoopSourceRef src = socket->_source;
  
  socket->_writeFired = true;
  
  if (src != NULL)
    CFRunLoopSourceSignal(src);
}
#endif

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
      CFRetain(new);
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
          new->_opts = kCFSocketCloseOnInvalidate
                       | kCFSocketAutomaticallyReenableAcceptCallBack
                       | kCFSocketAutomaticallyReenableDataCallBack
                       | kCFSocketAutomaticallyReenableReadCallBack;
          
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
          
#if HAVE_LIBDISPATCH
          dispatch_queue_t q = dispatch_get_global_queue(
              DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
          
          new->_readSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ,
                                                    new->_socket, 0, q);
          dispatch_set_context(new->_readSource, new);
          dispatch_source_set_event_handler_f(new->_readSource,
                                              CFSocketDispatchReadEvent);
          
          new->_writeSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE,
                                                    new->_socket, 0, q);
          dispatch_set_context(new->_writeSource, new);
          dispatch_source_set_event_handler_f(new->_writeSource,
                                              CFSocketDispatchWriteEvent);
#endif
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
      s->_isListening = true;
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
          int count;
          fd_set writefds[FD_SETSIZE];
          
          tv.tv_sec = (long)floor (timeout);
          tv.tv_usec = (long)((timeout - floor (timeout)) * 1000000.0);
          FD_ZERO(writefds);
          FD_SET(sock, writefds);
          count = select (sock + 1, NULL, writefds, NULL, &tv);
          
          if (count > 0)
            s->_isConnected = true;
          else
            err = -1;
        }
      else
        {
          err = 0;
        }
    }
    
  return (err == 0) ? kCFSocketSuccess : kCFSocketError;
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

static void
CFSocketUpdateDispatchSources (CFSocketRef s)
{
#if HAVE_LIBDISPATCH
#   define READ_EVENTS (kCFSocketReadCallBack|kCFSocketAcceptCallBack|kCFSocketDataCallBack)
#   define WRITE_EVENTS (kCFSocketConnectCallBack|kCFSocketWriteCallBack)
    
  if (s->_cbTypes & READ_EVENTS)
    {
      if (!s->_readResumed)
        {
          dispatch_resume(s->_readSource);
          s->_readResumed = true;
        }
    }
  else if (s->_readResumed)
    {
      dispatch_suspend(s->_readSource);
      s->_readResumed = false;
    }
  
  if (s->_cbTypes & WRITE_EVENTS)
    {
      if (!s->_writeResumed)
        {
          dispatch_resume(s->_writeSource);
          s->_writeResumed = true;
        }
    }
  else if (s->_writeResumed)
    {
      dispatch_suspend(s->_writeSource);
      s->_writeResumed = false;
    }
#endif
}

void
CFSocketDisableCallBacks (CFSocketRef s, CFOptionFlags cbTypes)
{
  s->_cbTypes &= ~cbTypes;
  CFSocketUpdateDispatchSources(s);
}

void
CFSocketEnableCallBacks (CFSocketRef s, CFOptionFlags cbTypes)
{
  if (s->_isConnected)
    cbTypes &= ~kCFSocketConnectCallBack;
  s->_cbTypes |= cbTypes;
  CFSocketUpdateDispatchSources(s);
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
  struct sockaddr* addr = NULL;
  socklen_t len;
  int err;
  struct timeval tv;
  
  if (CFSocketIsValid (s) == false || address == NULL || data == NULL)
    return kCFSocketError;
  
  tv.tv_sec = (int) floor(timeout);
  tv.tv_usec = (timeout - tv.tv_sec) * 1000000;
  
  err = setsockopt(s->_socket, SOL_SOCKET, SO_SNDTIMEO, &tv,
                   sizeof(struct timeval));
  
  if (err != 0)
    return kCFSocketError;
  
  if (address != NULL)
    {
      addr = (struct sockaddr*) CFDataGetBytePtr(address);
      len = CFDataGetLength(address);
      
      err = sendto(s->_socket, CFDataGetBytePtr(data), 0,
                   CFDataGetLength(data), addr, len);
    }
  else
    {
      err = send(s->_socket, CFDataGetBytePtr(data),
                 CFDataGetLength(data), 0);
    }
  if (err == 0)
    return kCFSocketSuccess;
  else if (errno == EAGAIN || errno == EWOULDBLOCK)
    return kCFSocketTimeout;
  else
    return kCFSocketError;
}

void
CFSocketInvalidate (CFSocketRef s)
{
#if HAVE_LIBDISPATCH
  if (s->_source != NULL)
    CFRunLoopSourceInvalidate(s->_source);
#endif
  if (s->_socket != -1 && s->_opts & kCFSocketCloseOnInvalidate)
    {
      GSMutexLock (&_kCFSocketObjectsLock);
      CFDictionaryRemoveValue(_kCFSocketObjects,
                              (void*)(uintptr_t) s->_socket);
      GSMutexUnlock (&_kCFSocketObjectsLock);
      
      closesocket (s->_socket);
      s->_socket = -1;
    }
}

Boolean
CFSocketIsValid (CFSocketRef s)
{
  if (s->_source != NULL)
    return CFRunLoopSourceIsValid(s->_source); // !kCFSocketCloseOnInvalidate case
  return s->_socket != -1;
}

#if HAVE_LIBDISPATCH
static void
CFSocketRLSSchedule (void* p, CFRunLoopRef rl, CFStringRef mode)
{
  CFSocketRef s = (CFSocketRef) p;
  
  CFSocketUpdateDispatchSources(s);
}

static void
CFSocketRLSPerform (void* p)
{
  CFSocketRef s = (CFSocketRef) p;
  
  if (s->_callback == NULL)
    return;
  
  if (s->_readFired)
    {
      if (s->_isListening)
        {
          if (s->_cbTypes & kCFSocketAcceptCallBack)
            {
              struct sockaddr addr;
              socklen_t len = sizeof(addr);
              int err;
          
              err = accept (s->_socket, &addr, &len);
              
              if (!(s->_opts & kCFSocketAutomaticallyReenableAcceptCallBack))
                CFSocketDisableCallBacks(s, kCFSocketAcceptCallBack);
          
              if (err != 0)
                {
                  CFSocketNativeHandle handle = (CFSocketNativeHandle) err;
                  CFDataRef addrData;
                  
                  addrData = CFDataCreate(NULL, (UInt8*) &addr, len);
                  s->_callback(s, kCFSocketAcceptCallBack, addrData, &handle,
                               s->_ctx.info);
                  
                  CFRelease(addrData);
                }
            }
        }
        
      if (s->_cbTypes & kCFSocketDataCallBack)
        {
          char buffer[512];
          int err;
          CFDataRef data, addrData;
          struct sockaddr addr;
          socklen_t len = sizeof(addr);
          
          if (!(s->_opts & kCFSocketAutomaticallyReenableDataCallBack))
            CFSocketDisableCallBacks(s, kCFSocketDataCallBack);
          
          err = recvfrom (s->_socket, buffer, sizeof(buffer), MSG_DONTWAIT,
                          &addr, &len);
          
          if (err < 0)
            err = 0;
          
          data = CFDataCreate(NULL, (UInt8*) buffer, err);
          addrData = CFDataCreate(NULL, (UInt8*) &addr, len);
          
          s->_callback(s, kCFSocketDataCallBack, addrData, data,
                       s->_ctx.info);
          
          CFRelease(data);
          CFRelease(addrData);
        }
      else if (s->_cbTypes & kCFSocketReadCallBack)
        {
          if (!(s->_opts & kCFSocketAutomaticallyReenableReadCallBack))
            CFSocketDisableCallBacks(s, kCFSocketReadCallBack);
          
          s->_callback(s, kCFSocketReadCallBack, NULL, NULL, s->_ctx.info);
        }  
      s->_readFired = false;
    }
  
  if (s->_writeFired)
    {
      if (!s->_isConnected && s->_cbTypes & kCFSocketConnectCallBack)
        {
          SInt32 err;
          socklen_t len = sizeof(err);
          int rv = getsockopt(s->_socket, SOL_SOCKET, SO_ERROR, &err, &len);
          
          if (rv == -1)
            err = errno;

          void* data;
              
          if (err != 0)
            data = &err;
          else
            data = NULL;
              
          s->_callback(s, kCFSocketConnectCallBack, NULL, data, s->_ctx.info);
        }
      else
        {
          s->_isConnected = true;
          if (!(s->_opts & kCFSocketAutomaticallyReenableWriteCallBack))
            CFSocketDisableCallBacks(s, kCFSocketWriteCallBack);
          
          if (!(s->_opts & kCFSocketLeaveErrors))
            {
              // Clear out last error
              SInt32 err;
              socklen_t len = sizeof(err);
              
              getsockopt(s->_socket, SOL_SOCKET, SO_ERROR, &err, &len);
            }  
          s->_callback(s, kCFSocketWriteCallBack, NULL, NULL, s->_ctx.info);
        }
      s->_writeFired = false;
    }
}

static void
CFSocketRLSCancel (void* p, CFRunLoopRef rl, CFStringRef mode)
{
}
#endif

CFRunLoopSourceRef
CFSocketCreateRunLoopSource (CFAllocatorRef alloc, CFSocketRef s,
                             CFIndex order)
{
#if HAVE_LIBDISPATCH
  CFRunLoopSourceRef source;
  CFRunLoopSourceContext context;
  
  if (s->_source != NULL)
    return (CFRunLoopSourceRef) CFRetain(s->_source);
  
  memset(&context, 0, sizeof(context));
  context.info = s;
  context.retain = CFRetain;
  context.release = CFRelease;
  context.schedule = CFSocketRLSSchedule;
  context.cancel = CFSocketRLSCancel;
  context.perform = CFSocketRLSPerform;
  
  source = CFRunLoopSourceCreate(CFGetAllocator(s), order, &context);
  s->_source = (CFRunLoopSourceRef) CFRetain(source);
  
  return source;
#else
  fprintf(stderr, "CFSocketCreateRunLoopSource(): dummy implementation, "
                  "GCD support not enabled\n");
  return NULL;
#endif
}


/* "Name server functionality is currently inoperable in OS X." */

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

