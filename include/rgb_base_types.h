/****************************************************************************
*                                                                           *
* RGB_BaseTypes.h -- Basic Windows Type Definitions                         *
*                                                                           *
****************************************************************************/


#ifndef _RGB_BaseTypes_H_
#define _RGB_BaseTypes_H_

#ifdef linux
#ifdef __KERNEL__
#include <asm/atomic.h>
#endif /* __KERNEL__ */
#endif /* linux */

// BASETYPES is defined in ntdef.h if these types are already defined

#ifndef BASETYPES
#define BASETYPES
typedef unsigned long  ULONG;
typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef ULONG  *PULONG;
typedef USHORT *PUSHORT;
typedef UCHAR  *PUCHAR;
#endif  // BASETYPES


typedef unsigned long       DWORD;
typedef int                 BOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TEXT
#define TEXT(x) x
#endif
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

typedef unsigned int        UINT;

#ifdef linux
#ifdef __KERNEL__
typedef atomic_t           ATOMIC_LONG_T;
typedef uint64_t           ULONG64;
typedef uint64_t           UINT64;
#else
typedef long               ATOMIC_LONG_T;
#endif /* __KERNEL__ */
#else
typedef volatile long      ATOMIC_LONG_T;
#endif /* linux */

#endif // _RGB_BaseTypes_H_
