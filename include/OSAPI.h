/*
 * OSAPI.h
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

#ifndef OSAPI_H_
#define OSAPI_H_

#ifdef INCLUDE_OSAPI

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/************************************ MEMORY **********************************/
typedef enum _eOSAPIMEMORY_T {
   OSAPI_MEMORY_TYPE_UNKNOWN = 0,
   OSAPI_MEMORY_TYPE_VIRTUAL,
   OSAPI_MEMORY_TYPE_KERNEL_LOGICAL,
   OSAPI_MEMORY_TYPE_NONPAGEDPOOL,
   OSAPI_MEMORY_TYPE_PAGEDPOOL,
} eOSAPIMEMORY_T;

void* OSAllocateMemory(eOSAPIMEMORY_T type, uint64_t size, uint64_t flags);
void OSFreeMemory(eOSAPIMEMORY_T etype, void* pContext, uint64_t flags);

void OSMemoryBarrier(void);

/************************************ LOCKS ***********************************/
typedef enum _eOSAPILOCK_T {
   OSAPI_LOCK_TYPE_UNKNOWN = 0,
   OSAPI_LOCK_TYPE_SPINLOCK,
   OSAPI_LOCK_TYPE_INTERRUPT_SPINLOCK,
   OSAPI_LOCK_TYPE_MUTEX,
   OSAPI_LOCK_TYPE_SEMPAHORE,
} eOSAPILOCK_T;

/* Abstracted Functions */
void* OSAllocateLock(eOSAPILOCK_T eType);
void OSInitialiseLock(eOSAPILOCK_T eType, void* pContext);
void OSFreeLock(eOSAPILOCK_T eType, void* pContext);

uint64_t OSAcquireLock(eOSAPILOCK_T eType, void* pContext1, void* pContext2);
void OSReleaseLock(eOSAPILOCK_T eType, void* pContext1, void* pContext2, uint64_t flags);

/*********************************** COUNTERS *********************************/
uint32_t OSIncLockedCounter(void* pContext);
uint32_t OSDecLockedCounter( void* pContext );




/*******************************************************************************/
//TODO: should move below tags out of osapi

#define TAG4( ch0, ch1, ch2, ch3 )  (   (uint32_t)(uint8_t)(ch0)         | ( (uint32_t)(uint8_t)(ch1) << 8  ) |    \
                                      ( (uint32_t)(uint8_t)(ch2) << 16 ) | ( (uint32_t)(uint8_t)(ch3) << 24 )   )

#define TagSHEL  TAG4('S','H','E','L') // Shell driver tag
#define TagHNDL  TAG4('H','N','D','L') // Handle table
#define TagENTR  TAG4('E','N','T','R') // entries pci tree
#define TagCMEM  TAG4('C','M','E','M') // 
#define TagFLSH  TAG4('F','L','S','H') // flash file tag for test only

#define Tag165D  TAG4('1','6','5','D') // DGC165DE
#define Tag165C  TAG4('1','6','5','C') // DGC165CL
#define Tag165B  TAG4('1','6','5','B') // DGC165BF

#define Tag183D  TAG4('1','8','3','D') // DGC183DE
#define Tag183C  TAG4('1','8','3','C') // DGC183CL
#define Tag183G  TAG4('1','8','3','G') // DGC183 General
#define Tag183H  TAG4('1','8','3','H') // DGC183 Hash table

#define TagSGTC  TAG4('S','G','T','C') // COMMON SGT
#define TagWINR  TAG4('W','I','N','R') // WINDOWS RES

#define Tag133D  TAG4('1','3','3','D') // DGC133_DEVICE
#define Tag133P  TAG4('1','3','3','P') // DGC133 Data
#define Tag133E  TAG4('1','3','3','E') // DGC133 Send IOCTL Event

#define TagRGBZ  TAG4('R','G','B','Z') // very temporary

/******************************************************************************/

void* AllocMemHandle(uint32_t NumberOfBytes);
void FreeMemHandle(void* pData);

/******************************************************************************/

void OSDelayExecution(uint64_t usec);           // Delay execution for given number of microseconds

/******************************************************************************/

#ifdef    __cplusplus
}
#endif // __cplusplus

#endif /* INCLUDE_OSAPI */

#ifdef INCLUDE_OSAPI_EXT

/************************ Core Kernel Library - START *************************/

/* 
 * Every call to OSInitializeEvent must be complemented by OSUninitializeEvent 
 * once event object is not needed any more.
 */
void OSInitializeEvent(POSEVENT pEvent, OSEVENT_TYPE type, BOOLEAN state);
void OSUnInitializeEvent(POSEVENT pEvent);
LONG OSSetEvent(POSEVENT pEvent, OSPRIORITY increment, BOOLEAN wait);
LONG OsResetEvent(POSEVENT pEvent);
void OsClearEvent(POSEVENT pEvent);
NTSTATUS OSWaitForSingleObject(PVOID pObject, OSWAIT_REASON WaitReason,
                               OSPROCESSOR_MODE WaitMode, BOOLEAN Alertable,
                               PLARGE_INTEGER Timeout);

/************************* Core Kernel Library - END **************************/

/************************* Executive Library - START **************************/

NTSTATUS OSInitializeResourceLite(POSRESOURCE Resource);
NTSTATUS OSDeleteResourceLite(POSRESOURCE Resource);
NTSTATUS OSReinitializeResourceLite(POSRESOURCE Resource);
BOOLEAN OSAcquireResourceExclusiveLite(POSRESOURCE Resource, BOOLEAN Wait);
BOOLEAN OSAcquireResourceSharedLite(POSRESOURCE Resource, BOOLEAN Wait);
VOID OSReleaseResourceLite(POSRESOURCE Resource);

/************************** Executive Library - END ***************************/

#endif /* INCLUDE_OSAPI_EXT */

#endif /* OSAPI_H_ */
