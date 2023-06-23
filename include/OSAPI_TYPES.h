/*
 * OSAPI_TYPES.h
 *
 * Copyright (c) 2022 Datapath Limited All rights reserved.
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

#ifndef OSAPI_LINUX_TYPES_H_
#define OSAPI_LINUX_TYPES_H_

#ifdef INCLUDE_OSAPI_EXT

#include "rgb133config.h"

#include "rgb_api_types.h"
#include "rgb_base_types.h"
#include "rgb_linux_types.h"


/**********************************************************************************/

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY* Flink;
   struct _LIST_ENTRY* Blink;
} LIST_ENTRY, *PLIST_ENTRY;

#define IO_NO_INCREMENT             0
#define IO_INCREMENT                1

#define NTSTATUS                    DWORD

typedef unsigned int OSPRIORITY;
typedef unsigned int OSWAIT_REASON;
typedef unsigned int OSPROCESSOR_MODE;

typedef enum _OSPROCESSOR_MODE_E {
   Executive,
   KernelMode,
   UserMode,
} OSPROCESSOR_MODE_E, *POSPROCESSOR_MODE_E;

typedef enum _OSEVENT_TYPE {
   NotificationEvent,
   SynchronizationEvent,
} OSEVENT_TYPE, *POSEVENT_TYPE;

typedef union _LARGE_INTEGER {
   struct
   {
      __int32 LowPart;
      __int32  HighPart;
   };
   __int64 QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _OSEVENT {
   PWAITQUEUEAPI     q;
   OSEVENT_TYPE      type;
   BOOLEAN           signalled;
   BOOLEAN           interruptible;
   BOOLEAN           timeout;
   ATOMIC_LONG_T     count;
   PMUTEXAPI         pmlock;
} OSEVENT, *POSEVENT;

/* Keep structs _OWNER_ENTRY and _WAIT_ENTRY the same size and keep their members the same order
 * so we can cast their pointers one to another and they can be moved around between waiters and owners lists */
typedef struct _WAIT_ENTRY {
   LIST_ENTRY         Entry;
   pid_t              threadId;
   PSEMAPHOREAPI      pSem;
   ULONG              dummy;
} WAIT_ENTRY, *PWAIT_ENTRY;

typedef struct _OWNER_ENTRY {
   LIST_ENTRY         Entry;
   pid_t              threadId;
   PSEMAPHOREAPI      dummy;
   ULONG              numRecursive;
} OWNER_ENTRY, *POWNER_ENTRY;

typedef struct _OSRESOURCE {
   LIST_ENTRY   Owners;
   LIST_ENTRY   SharedWaiters;
   LIST_ENTRY   ExclusiveWaiters;
   PSPINLOCKAPI pSpinLock;
   ULONG        numActive;
   ULONG        numExclusiveWaiters;
   ULONG        numSharedWaiters;
   BOOLEAN      ownedExclusive;
} OSRESOURCE, *POSRESOURCE;

/**********************************************************************************/

#endif /* INCLUDE_OSAPI_EXT */

#endif /* OSAPI_LINUX_TYPES_H_ */
