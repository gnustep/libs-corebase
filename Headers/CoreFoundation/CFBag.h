/* CFBag.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of CoreBase.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#ifndef __COREFOUNDATION_CFBAG_H__
#define __COREFOUNDATION_CFBAG_H__

#include <CoreFoundation/CFBase.h>

CF_EXTERN_C_BEGIN
/** \ingroup CFBagRef */
typedef const struct __CFBag *CFBagRef;
/** \ingroup CFMutableBagRef */
typedef struct __CFBag *CFMutableBagRef;

/** \defgroup CFBagRef CFBag Reference
    \{
 */
typedef void (*CFBagApplierFunction) (const void *value, void *context);

typedef CFStringRef (*CFBagCopyDescriptionCallBack) (const void *value);
typedef Boolean (*CFBagEqualCallBack) (const void *value1, const void *value2);
typedef CFHashCode (*CFBagHashCallBack) (const void *value);
typedef void (*CFBagReleaseCallBack) (CFAllocatorRef alloc, const void *value);
typedef const void *(*CFBagRetainCallBack) (CFAllocatorRef alloc,
                                            const void *value);

typedef struct CFBagCallBacks CFBagCallBacks;
struct CFBagCallBacks
{
  CFIndex version;
  CFBagRetainCallBack retain;
  CFBagReleaseCallBack release;
  CFBagCopyDescriptionCallBack copyDescription;
  CFBagEqualCallBack equal;
  CFBagHashCallBack hash;
};

CF_EXPORT const CFBagCallBacks kCFCopyStringBagCallBacks;
CF_EXPORT const CFBagCallBacks kCFTypeBagCallBacks;



/** \name Creating a bag
    \{
 */
CF_EXPORT CFBagRef
CFBagCreate (CFAllocatorRef alloc, const void **values, CFIndex numValues,
             const CFBagCallBacks * callBacks);

CF_EXPORT CFBagRef CFBagCreateCopy (CFAllocatorRef alloc, CFBagRef bag);
/** \} */

/** \name Examining a dictionary
    \{
 */
CF_EXPORT Boolean CFBagContainsValue (CFBagRef bag, const void *value);

CF_EXPORT CFIndex CFBagGetCount (CFBagRef bag);

CF_EXPORT CFIndex CFBagGetCountOfValue (CFBagRef bag, const void *value);

CF_EXPORT void CFBagGetValues (CFBagRef bag, const void **values);

CF_EXPORT const void *CFBagGetValue (CFBagRef bag, const void *value);

CF_EXPORT Boolean
CFBagGetValueIfPresent (CFBagRef bag, const void *candidate,
                        const void **value);
/** \} */

/** \name Applying a funcation to a dictionary
    \{
 */
CF_EXPORT void
CFBagApplyFunction (CFBagRef bag, CFBagApplierFunction applier, void *context);
/** \} */

/** \name Getting the CFBag type ID
    \{
 */
CF_EXPORT CFTypeID CFBagGetTypeID (void);
/** \} */
/** \} */

/** \defgroup CFMutableBagRef CFMutableBag Reference
    \{
 */
/** \name Creating a Mutable Dictionary
    \{
 */
CF_EXPORT CFMutableBagRef
CFBagCreateMutable (CFAllocatorRef alloc, CFIndex capacity,
                    const CFBagCallBacks * callBacks);

CF_EXPORT CFMutableBagRef
CFBagCreateMutableCopy (CFAllocatorRef alloc, CFIndex capacity, CFBagRef bag);
/** \} */

/** \name Modifying a Dictionary
    \{
 */
CF_EXPORT void CFBagAddValue (CFMutableBagRef bag, const void *value);

CF_EXPORT void CFBagRemoveAllValues (CFMutableBagRef bag);

CF_EXPORT void CFBagRemoveValue (CFMutableBagRef bag, const void *value);

CF_EXPORT void CFBagReplaceValue (CFMutableBagRef bag, const void *value);

CF_EXPORT void CFBagSetValue (CFMutableBagRef bag, const void *value);
/** \} */
/** \} */

CF_EXTERN_C_END
#endif /* __COREFOUNDATION_CFBAG_H__ */
