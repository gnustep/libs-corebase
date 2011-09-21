/* NSCFType.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
   This file is part of GNUstep CoreBase library.
   
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

#import <Foundation/NSObject.h>

/* This is NSCFType, the ObjC class that all non-bridged CF types belong to.
 */
@interface NSCFType : NSObject
{
  /* NSCFType's ivar layout must match CFRuntimeBase.
   */
  int16_t _typeID;
  struct
    {
      int16_t ro:       1; // 0 = read-only object
      int16_t unused:   7;
      int16_t reserved: 8;
    } _flags;
}

- (CFTypeID) _cfTypeID;
@end
