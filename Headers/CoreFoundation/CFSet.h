/* CFSet.h

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by: Stefan Bidigaray
   Date: January, 2010

   This file is part of the GNUstep CoreBase Library.

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

#ifndef __COREFOUNDATION_CFSET_H__
#define __COREFOUNDATION_CFSET_H__ 1

#include <CoreFoundation/CFBase.h>

CF_EXTERN_C_BEGIN
/** \ingroup CFSetRef */
typedef const struct __CFSet *CFSetRef;
/** \ingroup CFMutableSetRef */
typedef struct __CFSet *CFMutableSetRef;

/** \defgroup CFSetRef CFSet Reference
    \{
 */
typedef void (*CFSetApplierFunction) (const void *value, void *context);

typedef CFStringRef (*CFSetCopyDescriptionCallBack) (const void *value);
typedef Boolean (*CFSetEqualCallBack) (const void *value1, const void *value2);
typedef CFHashCode (*CFSetHashCallBack) (const void *value);
typedef void (*CFSetReleaseCallBack) (CFAllocatorRef alloc, const void *value);
typedef const void *(*CFSetRetainCallBack) (CFAllocatorRef alloc,
                                            const void *value);

typedef struct CFSetCallBacks CFSetCallBacks;
struct CFSetCallBacks
{
  CFIndex version;
  CFSetRetainCallBack retain;
  CFSetReleaseCallBack release;
  CFSetCopyDescriptionCallBack copyDescription;
  CFSetEqualCallBack equal;
  CFSetHashCallBack hash;
};

CF_EXPORT const CFSetCallBacks kCFCopyStringSetCallBacks;
CF_EXPORT const CFSetCallBacks kCFTypeSetCallBacks;

/** \name Creating a set
 */
CF_EXPORT CFSetRef
CFSetCreate (CFAllocatorRef alloc, const void **values, CFIndex numValues,
             const CFSetCallBacks * callBacks);

CF_EXPORT CFSetRef CFSetCreateCopy (CFAllocatorRef alloc, CFSetRef set);
/** \} */

/** \name Examining a set
    \{
 */
CF_EXPORT Boolean CFSetContainsValue (CFSetRef set, const void *value);

CF_EXPORT CFIndex CFSetGetCount (CFSetRef set);

CF_EXPORT CFIndex CFSetGetCountOfValue (CFSetRef set, const void *value);

CF_EXPORT void CFSetGetValues (CFSetRef set, const void **values);

CF_EXPORT const void *CFSetGetValue (CFSetRef set, const void *value);

CF_EXPORT Boolean
CFSetGetValueIfPresent (CFSetRef set, const void *candidate,
                        const void **value);
/** \} */

/** \name Applying a funcation to a set
    \{
 */
CF_EXPORT void
CFSetApplyFunction (CFSetRef set, CFSetApplierFunction applier, void *context);
/** \} */

/** \name Getting the CFSet type ID
    \{
 */
CF_EXPORT CFTypeID CFSetGetTypeID (void);
/** \} */
/** \} */

/** \defgroup CFMutableSetRef CFMutableSet Reference
    \{
 */
/** \name Creating a Mutable Set
    \{
 */
CF_EXPORT CFMutableSetRef
CFSetCreateMutable (CFAllocatorRef alloc, CFIndex capacity,
                    const CFSetCallBacks * callBacks);

CF_EXPORT CFMutableSetRef
CFSetCreateMutableCopy (CFAllocatorRef alloc, CFIndex capacity, CFSetRef set);
/** \} */

/** \name Modifying a Set
    \{
 */
CF_EXPORT void CFSetAddValue (CFMutableSetRef set, const void *value);

CF_EXPORT void CFSetRemoveAllValues (CFMutableSetRef set);

CF_EXPORT void CFSetRemoveValue (CFMutableSetRef set, const void *value);

CF_EXPORT void CFSetReplaceValue (CFMutableSetRef set, const void *value);

CF_EXPORT void CFSetSetValue (CFMutableSetRef set, const void *value);
/** \} */
/** \} */

CF_EXTERN_C_END
#endif /* __COREFOUNDATION_CFSET_H__ */

