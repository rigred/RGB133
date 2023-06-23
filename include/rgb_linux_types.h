/*
 * rgb_linux_types.h
 *
 * Copyright (c) 2009 Datapath Limited All rights reserved.
 *
 * All information contained herein is proprietary and
 * confidential to Datapath Limited and is licensed under
 * the terms of the Datapath Limited Software License.
 * Please read the LICENCE file for full license terms
 * and conditions.
 *
 * http://www.datapath.co.uk/
 * support@datapath.co.uk
 *
 */

#ifndef _RGB_LINUX_TYPES_H
#define _RGB_LINUX_TYPES_H

#ifndef NULL
#define NULL 0
#endif

#define __int8          char
#define __uint8         unsigned char
#define __int16         short
#define __uint16        unsigned short
#define __int32         int
#define __uint32        unsigned int
#define __int64         long long
#define __uint64        unsigned long long

#define i64             LL

typedef int                 BOOLEAN;
typedef int*                PBOOL;
typedef long                LONG;
typedef long*               PLONG;
typedef long*               LPLONG;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
typedef unsigned long long* PVOID64;
typedef unsigned long       ULONG_PTR;
typedef void*               PVOID;
typedef void*               LPVOID;
typedef void*               WPARAM;
typedef void*               LPARAM;
typedef void*               HWND;
typedef unsigned char*      PBYTE;
typedef unsigned char*      LPBYTE;

typedef void                VOID;
typedef int                 SIZE_T;

#define POINTER_32

#endif /* _RGB_LINUX_TYPES_H */
