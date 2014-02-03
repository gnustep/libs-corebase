/* CFStream.h
   
   Copyright (C) 2014 Free Software Foundation, Inc.
   
   Written by: Lubos Dolezel
   Date: February, 2014
   
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

#ifndef __COREFOUNDATION_CFSTREAMPRIV_H__
#define __COREFOUNDATION_CFSTREAMPRIV_H__ 1
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFStream.h>

/* 
 * This header file is intended to be used by CFStream implementations, namely
 * by CFStream itself and CFNetwork.
 * 
 */

struct CFWriteStreamImpl
{
    void (*close)(CFWriteStreamRef);
    void (*finalize)(CFWriteStreamRef);
    Boolean (*open)(CFWriteStreamRef);
    CFIndex (*write)(CFWriteStreamRef, const UInt8 *, CFIndex);
    CFTypeRef (*copyProperty)(CFWriteStreamRef, CFStringRef);
    Boolean (*setProperty)(CFWriteStreamRef, CFStringRef, CFTypeRef);
    Boolean (*acceptBytes)(CFWriteStreamRef);
};

struct __CFWriteStream
{
    CFRuntimeBase               parent;
    struct CFWriteStreamImpl    impl;

    Boolean                     open, closed, failed;
    CFErrorRef                  error;

    /* callbacks when used with a runloop */
    CFOptionFlags               streamEvents;
    CFWriteStreamClientCallBack clientCB;  
};

struct CFReadStreamImpl
{
    void (*close)(CFReadStreamRef);
    void (*finalize)(CFReadStreamRef);
    Boolean (*open)(CFReadStreamRef);
    CFIndex (*read)(CFReadStreamRef, UInt8 *, CFIndex);
    CFTypeRef (*copyProperty)(CFReadStreamRef, CFStringRef);
    Boolean (*setProperty)(CFReadStreamRef, CFStringRef, CFTypeRef);
    const UInt8* (*getBuffer)(CFReadStreamRef, CFIndex, CFIndex*);
    Boolean (*hasBytes)(CFReadStreamRef);
};

struct __CFReadStream
{
    CFRuntimeBase              parent;
    struct CFReadStreamImpl    impl;

    Boolean                    open, closed, failed;
    CFErrorRef                 error;

    /* callbacks when used with a runloop */
    CFOptionFlags              streamEvents;
    CFReadStreamClientCallBack clientCB;
};

#endif
