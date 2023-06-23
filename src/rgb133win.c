/*
 * rgb133win.c
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

#ifdef __KERNEL__
#include <asm/atomic.h>

#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>

#include "rgb133config.h"
#include "rgb133.h"
#include "rgb133capapi.h"
#include "rgb133deviceapi.h"
#include "rgb133kernel.h"
#include "rgb133sg.h"
#include "rgb133status.h"
#include "rgb133time.h"
#include "rgb133timestamp.h"
#include "rgb133vidbuf.h"
#else
#include <string.h>
#include "rgb_api_types.h"
#include "rgb133user.h"
#include "rgb_linux_user_functions.h"
#endif /* __KERNEL__ */

#include "rgb133win.h"
#include "rgb133sgapi.h"
#include "rgb133registry.h"
#include "rgb133timer.h"
#include "rgb133debug.h"


#ifndef RGB133_CONFIG_HAVE_SG_MARK_END
// sg_mark_end is a no-op for us under old non scatterlist-chain-capable kernels.
#define sg_mark_end(x, y)
#endif

/****************************************************************************
 * Windows Function Definitions                                             *
 ****************************************************************************/

/* Helper to convert 4 byte tags to ASCI str
 * tag - Pointer to base of string
 */
char* TagToStr(ULONG Tag, char* ret)
{
   int i;

   if(Tag)
   {
      for(i=0;i<4;i++)
      {
         ret[i] = Tag >> (i*8);
      }
   }

   return ret;
}

/***** Executive Library (EX) Support Routine Abstractions - START *****/

/* Abstracted memory allocation
 * PoolType       - type of memory to be allocated (paged, non-paged)
 *                  only non-paged makes sense in the linux kernel
 * NumberfBytes   - how much to allocate
 * Tag            - specifiec to memory tag (do we need this in linux?)
 *                  ignored for the moment!?
 * returns        - NULL if insufficient memory is available,
 *                  otherwise a pointed to the allocated space
 */
PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag)
{
   PVOID allocated = NULL;
   allocated = (PVOID)KernelVzalloc(NumberOfBytes);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "ExAllocatePoolWithTag: alloc %d bytes to npp allocated(0x%p)\n", NumberOfBytes, allocated));

   if(allocated == NULL)
      return NULL;

   return allocated;
}

/* Abstracted memory allocation -wrapper for above
 * PoolType       - type of memory to be allocated (paged, non-paged)
 *                  only non-paged makes sense in the linux kernel
 * NumberfBytes   - how much to allocate
 * returns        - NULL if insufficient memory is available,
 *                  otherwise a pointed to the allocated space
 */
PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T NumberOfBytes)
{
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "ExAllocatePool: Call ExAllocatePoolWithTag\n"));
   return ExAllocatePoolWithTag(PoolType, NumberOfBytes, 0);
}

/* Abstracted memory free
 * P     - The beginning addrsss of memory allocated by ExAllocatePoolWithTag  
 * Tag   - specifiec to memory tag (do we need this in linux?)
 *         ignored for the moment!?
 */
VOID ExFreePoolWithTag(PVOID P, ULONG Tag)
{
   if(P != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "ExFreePoolWithTag: Free P(0x%p)\n", P));
      KernelVfree(P);
   }
}

/* Abstracted memory free - wrapper for above
 * P     - The beginning addrsss of memory allocated by ExAllocatePoolWithTag  
 */
VOID ExFreePool(PVOID P)
{
   //RGB133PRINT((RGB133_LINUX_DBG_TODO, "ExFreePool: Call ExFreePoolWithTag\n"));
   ExFreePoolWithTag(P, 0);
}

/* Abstracted atomic increment
 * count    - atomic value to be incremented
 * returns  - the incremented value
 */
UINT InterlockedIncrement(ATOMIC_LONG_T* count)
{
#ifdef __KERNEL__
   return atomic_inc_return(count);
#else
   *count++;
   return *count;
#endif /* __KERNEL__ */
}

/* Abstracted atomic decrement
 * count    - atomic value to be decremented
 * returns  - the dencremented value
 */
UINT InterlockedDecrement(ATOMIC_LONG_T* count)
{
#ifdef __KERNEL__
   return atomic_dec_return(count);
#else
   *count--;
   return *count;
#endif /* __KERNEL__ */
}

/* Abstracted list_init
 * SListHead   - Pointer to list_head struct to initialise
 */
VOID ExInitializeSListHead(PSLIST_HEADER SListHead)
{
   INIT_LIST_HEAD(SListHead);
}

/* Abstracted locked list_add
 * ListHead    - Pointer to the list head for the linked list
 * ListEntry   - Pointer to the caller-alocated entry to be inserted
 * Lock        - spinlock to be used
 * returns     - Pointer to the first entry in the list before the
 *               new entry was inserted.  If the list was empty the
 *               routine returns NULL
 */
PSLIST_ENTRY ExInterlockedPushEntrySList(PSLIST_HEADER ListHead,
               PSLIST_ENTRY ListEntry, PKSPIN_LOCK Lock)
{
   unsigned long flags = 0;
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPushEntrySList: spin_lock_irqsave(0x%p - 0x%lx)\n",
         Lock, flags));
   KernelPreemptDisable();
   spin_lock_irqsave(&Lock->lock, flags);
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPushEntrySList: spin_lock_irqsave(0x%p - 0x%lx) - locked\n",
         Lock, flags));
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ExInterlockedPushEntrySList: add entry(0x%p) to list(0x%p)\n",
         ListEntry, ListHead));
   list_add(ListEntry, ListHead);
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPushEntrySList: spin_unlock_irqrestore(0x%p - 0x%lx)\n",
         Lock, flags));
   spin_unlock_irqrestore(&Lock->lock, flags);
   KernelPreemptEnable();
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPushEntrySList: spin_unlock_irqrestore(0x%p - 0x%lx) - unlocked\n",
         Lock, flags));

   return NULL;
}

/* Abstracted locked list_entry
 * ListHead - Pointer to the list head for the linked list
 * Lock     - spinlock to be used
 * returns  - Pointer to the first entry in the list.  If the list is
 *            empty it returns NULL
 */
PSLIST_ENTRY ExInterlockedPopEntrySList(PSLIST_HEADER ListHead,
               PKSPIN_LOCK Lock)
{
   unsigned long flags = 0;
   struct list_head* entry = NULL;
   struct list_head* cursor;
   struct list_head* next;
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPopEntrySList: spin_lock_irqsave(0x%p - 0x%lx)\n",
         Lock, flags));
   KernelPreemptDisable();
   spin_lock_irqsave(&Lock->lock, flags);
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPopEntrySList: spin_lock_irqsave(0x%p - 0x%lx) - locked\n",
         Lock, flags));
   if(list_empty(ListHead))
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ExInterlockedPopEntrySList: List(0x%p) is empty\n",
            ListHead));
   }
   else
   {
      list_for_each_safe(cursor, next, ListHead) {
         if(cursor) {
            entry = cursor;
            list_del(cursor);
            break;
         }
      }
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ExInterlockedPopEntrySList: pop entry(0x%p) off list(0x%p)\n",
         entry, ListHead));
   }
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPopEntrySList: spin_unlock_irq(0x%p - 0x%lx)\n",
         Lock, flags));
   spin_unlock_irqrestore(&Lock->lock, flags);
   KernelPreemptEnable();
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedPopEntrySList: spin_unlock_irq(0x%p - 0x%lx) - unlocked\n",
         Lock, flags));
   return entry;
}

/* Abstracted deletion of all entries in a list
 * ListHead - Pointer to the list head to be operate on
 */

static KSPIN_LOCK generic_lock;
static int first = 1;

PSLIST_ENTRY ExInterlockedFlushSList(PSLIST_HEADER ListHead)
{
   struct list_head* cursor;
   struct list_head* next;
   unsigned long flags = 0;
   
   if(first)
   {
      KeInitializeSpinLock(&generic_lock);
      first = 0;
   }
   
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedFlushSList: spin_lock_irqsave(0x%p - 0x%lx)\n",
         &generic_lock, flags));
   spin_lock_irqsave(&generic_lock.lock, flags);
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedFlushSList: spin_lock_irqsave(0x%p - 0x%lx) - locked\n",
         &generic_lock, flags));
   
   if(list_empty(ListHead))
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ExInterlockedFlushSList: List(0x%p) is empty\n",
            ListHead));
   }
   else
   {
      list_for_each_safe(cursor, next, ListHead) {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "ExInterlockedFlushSList: del ptr(0x%p) from list(0x%p)\n",
               cursor, ListHead));
         if(cursor)
            list_del(cursor);
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedFlushSList: spin_unlock_irqrestore(0x%p - 0x%lx) - locked\n",
         &generic_lock, flags));
   spin_unlock_irqrestore(&generic_lock.lock, flags);
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "ExInterlockedFlushSList: spin_unlock_irqrestore(0x%p - 0x%lx) - locked\n",
         &generic_lock, flags));
   
   return NULL;
}

/***** Executive Library (EX) Support Routine Abstractions - END   *****/

/***** Core Kernel Library (KE) Support Routine Abstractions - START *****/

//static int KeAcqCount = 0;
//static int KeAcqDPCCount = 0;
//static int KeAcqIntCount = 0;
//static int KeRelCount = 0;
//static int KeRelDPCCount = 0;
//static int KeRelIntCount = 0;

/* Abstracted spinlock_t initialisation
 * SpinLock - spinlock to be initialised (allocated by caller)
 */
VOID KeInitializeSpinLock(PVOID _SpinLock)
{
   PKSPIN_LOCK SpinLock = (PKSPIN_LOCK)_SpinLock;
   RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeInitializeSpinLock: initialize spinlock(0x%p)\n",
      &SpinLock->lock));
   spin_lock_init(&SpinLock->lock);
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeInitializeSpinLock: initialized spinlock(0x%p)\n",
   //   &SpinLock->lock));
}

/* Abstracted spin_lock_irqsave
 *  SpinLock - The spin lock to be operated on
 *  Flags    - IRQ Flags to be saved
 */
VOID KeAcquireSpinLock(PVOID _SpinLock, PULONG pFlags)
{
   PKSPIN_LOCK SpinLock = (PKSPIN_LOCK)_SpinLock;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: spin_lock_irqsave(0x%p) - flags(0x%lx)\n",
   //      &SpinLock->lock, *pFlags));
   KernelPreemptDisable();
   spin_lock_irqsave(&SpinLock->lock, *pFlags);
   //KeAcqCount++;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: spin_lock_irqsave(0x%p) - locked\n",
   //      &SpinLock->lock));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqCount   (%d) KeRelCount   (%d)\n", KeAcqCount, KeRelCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqDPCCount(%d) KeRelDPCCount(%d)\n", KeAcqDPCCount, KeRelDPCCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqIntCount(%d) KeRelIntCount(%d)\n", KeAcqIntCount, KeRelIntCount));
}

/* Abstracted spin_lock - used in an isr
 *  SpinLock - The spin lock to be operated on
 */
VOID KeAcquireSpinLockAtDpcLevel(PVOID _SpinLock)
{
   PKSPIN_LOCK SpinLock = (PKSPIN_LOCK)_SpinLock;
   KernelPreemptDisable();
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLockAtDpcLevel: spin_lock_irq(0x%p)\n",
   //      &SpinLock->lock));
   spin_lock_irqsave(&SpinLock->lock, SpinLock->flags);
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLockAtDpcLevel: spin_lock(0x%p) - locked\n",
   //      &SpinLock->lock));
   //KeAcqDPCCount++;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqCount   (%d) KeRelCount   (%d)\n", KeAcqCount, KeRelCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqDPCCount(%d) KeRelDPCCount(%d)\n", KeAcqDPCCount, KeRelDPCCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqIntCount(%d) KeRelIntCount(%d)\n", KeAcqIntCount, KeRelIntCount));
}

/* Abstracted spin_lock - used inside an isr
 * SpinLock - The spin lock to be operated on
 */
KIRQL KeAcquireInterruptSpinLock(PKINTERRUPT pInterruptObject)
{
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireInterruptSpinLock: use pInterruptObject(0x%p) and lock(0x%p)\n",
   //   pInterruptObject, &pInterruptObject->SpinLock));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireInterruptSpinLock: spin_lock_irqsave(0x%p)\n",
   //      &pInterruptObject->SpinLock.lock));
   spin_lock_irqsave(&pInterruptObject->SpinLock.lock, pInterruptObject->Flags);
   //KeAcqIntCount++;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireInterruptSpinLock: spin_lock_irqsave(0x%p) - locked\n",
   //      &pInterruptObject->SpinLock.lock));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqCount   (%d) KeRelCount   (%d)\n", KeAcqCount, KeRelCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqDPCCount(%d) KeRelDPCCount(%d)\n", KeAcqDPCCount, KeRelDPCCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeAcquireSpinLock: KeAcqIntCount(%d) KeRelIntCount(%d)\n", KeAcqIntCount, KeRelIntCount));
   return 0;
}

/* Abstracted spin_unlock_irqrestore
 *  SpinLock - The spin lock to be operated on
 *  pFlags   - Pointer to IRQ Flags to be restored
 */
VOID KeReleaseSpinLock(PVOID _SpinLock, ULONG Flags)
{
   PKSPIN_LOCK SpinLock = (PKSPIN_LOCK)_SpinLock;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLock: spin_unlock_irqrestore(0x%p)\n",
   //      &SpinLock->lock));
   spin_unlock_irqrestore(&SpinLock->lock, Flags);
   KernelPreemptEnable();
   //KeRelCount++;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLock: spin_unlock_irqrestore(0x%p)- unlocked\n",
   //      &SpinLock->lock));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLock: KeAcqCount   (%d) KeRelCount   (%d)\n", KeAcqCount, KeRelCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLock: KeAcqDPCCount(%d) KeRelDPCCount(%d)\n", KeAcqDPCCount, KeRelDPCCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLock: KeAcqIntCount(%d) KeRelIntCount(%d)\n", KeAcqIntCount, KeRelIntCount));
}

/* Abstracted spin_unlock - used in an isr
 *  SpinLock - The spin lock to be operated on
 */
VOID KeReleaseSpinLockFromDpcLevel(PVOID _SpinLock)
{
   PKSPIN_LOCK SpinLock = (PKSPIN_LOCK)_SpinLock;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLockFromDpcLevel: spin_unlock_irq(0x%p)\n",
   //      &SpinLock->lock));
   spin_unlock_irqrestore(&SpinLock->lock, SpinLock->flags);
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLockFromDpcLevel: spin_unlock(0x%p) - unlocked\n",
   //      &SpinLock->lock));
   //KeRelDPCCount++;
   KernelPreemptEnable();
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLockFromDpcLevel: KeAcqCount   (%d) KeRelCount   (%d)\n", KeAcqCount, KeRelCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLockFromDpcLevel: KeAcqDPCCount(%d) KeRelDPCCount(%d)\n", KeAcqDPCCount, KeRelDPCCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseSpinLockFromDpcLevel: KeAcqIntCount(%d) KeRelIntCount(%d)\n", KeAcqIntCount, KeRelIntCount));
}

/* Abstracted spin_unlock - used inside an isr
 * SpinLock - The spin lock to be operated on
 */
VOID KeReleaseInterruptSpinLock(PKINTERRUPT pInterruptObject, KIRQL flags)
{
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseInterruptSpinLock: use pInterruptObject(0x%p) and lock(0x%p)\n",
   //   pInterruptObject, &pInterruptObject->SpinLock));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseInterruptSpinLock: spin_unlock_irqrestore(0x%p)\n",
   //      &pInterruptObject->SpinLock.lock));
   spin_unlock_irqrestore(&pInterruptObject->SpinLock.lock, pInterruptObject->Flags);
   //KeRelIntCount++;
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseInterruptSpinLock: spin_unlock_irqrestore(0x%p) - unlocked\n",
   //      &pInterruptObject->SpinLock.lock));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseInterruptSpinLock: KeAcqCount   (%d) KeRelCount   (%d)\n", KeAcqCount, KeRelCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseInterruptSpinLock: KeAcqDPCCount(%d) KeRelDPCCount(%d)\n", KeAcqDPCCount, KeRelDPCCount));
   //RGB133PRINT((RGB133_LINUX_DBG_SPIN, "KeReleaseInterruptSpinLock: KeAcqIntCount(%d) KeRelIntCount(%d)\n", KeAcqIntCount, KeRelIntCount));
}

/* Get the high res timer freq in Hz
 * li - Pointer to LARGE_INTEGER struct to hold
 *      the timer frequency (in Hz!!).
 *      In other words, ticks per second.
 */
unsigned long long KeQueryPerformanceCounter(PVOID arg)
{
   PLARGE_INTEGER li = (PLARGE_INTEGER)arg;
   LARGE_INTEGER ret;
   unsigned long long now = 0, later = 0;
#ifdef RGB133_CONFIG_HAVE_NATIVE_RD_TSC
   now = native_read_tsc();
#else /* READ_TSC_NO_ARGUMENTS */
#ifdef RGB133_CONFIG_HAVE_READ_TSC_NO_ARGUMENTS
   now = rdtsc();
#else
   unsigned long l, h;
   rdtsc(l,h);
   now = (((unsigned long long)l) | (((unsigned long long)h) << 32));
#endif /* READ_TSC_NO_ARGUMENTS */
#endif /* RGB133PRE */
   
   ret.QuadPart = 0;
   
   if(li != NULL)
   {
      //msleep(1000);
#ifdef RGB133_CONFIG_HAVE_NATIVE_RD_TSC
      later = native_read_tsc();
#else /* READ_TSC_NO_ARGUMENTS */
#ifdef RGB133_CONFIG_HAVE_READ_TSC_NO_ARGUMENTS
      later = rdtsc();
#else
      rdtsc(l,h);
      later = (((unsigned long long)l) | (((unsigned long long)h) << 32));
#endif /* READ_TSC_NO_ARGUMENTS */
#endif /* RGB133PRE */
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KeQueryPerformanceCounter: freq(%llu)\n", (later-now)));
      li->QuadPart = (later-now);
   }
   else
   {
      ret.QuadPart = now;
   }

   return ret.QuadPart; 
}

/* Abstracted udelay
 * MicroSeconds - The number of micro-seconds to wait for
 */
VOID KeStallExecutionProcessor(ULONG MicroSeconds)
{
   ULONG ms = (MicroSeconds / 1000), us = 0;
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KeStallExecutionProcessor: udelay(%lu)\n", MicroSeconds));
#ifdef __KERNEL__
   us = do_div(MicroSeconds,1000);
   RGB133BUSYWAIT(ms, us);
#else
   udelay(MicroSeconds);
#endif
}

/* Abstracted udelay
 * WaitMode  - Processor mode
 * Alertable - Is the call alertable
 * Interval  - The number of 100ns unit to sleep for
 */
NTSTATUS KeDelayExecutionThread(OSPROCESSOR_MODE WaitMode,
            BOOLEAN Alertable, PLARGE_INTEGER Interval)
{
   // 100ns = 1 unit
   // 1us   = 10 units
   // 1ms   = 10000 units
   // 1s    = 10000000 units
   
   unsigned long long delay = Interval->QuadPart;
   unsigned long long us = 0;
   // 20000 = 20000 units = 2ms
   
   if(delay > 10)
      delay = 10;
   
#ifdef __KERNEL__
   us = do_div(delay, 10);
#else
   delay /= 10;
#endif
   // 2000us = 2ms
   
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KeDelayExecutionThread: udelay(%llu)\n", delay));
#ifdef __KERNEL__
   RGB133WAIT(delay, us);
#else
   usleep(delay/1000);
#endif
   return STATUS_SUCCESS;
}

/* Abstracted BUG_ON macro
 * !!! Will cause a kernel panic to avoid a corrupt system
 */
VOID KeBugCheckEx(ULONG BugCheckCode,
         ULONG_PTR BugCheckParameter1, ULONG_PTR BugCheckParameter2,
         ULONG_PTR BugCheckParameter3, ULONG_PTR BugCheckParameter4)
{
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KeBugCheckEx: 0x%lx\n", BugCheckCode));
   //BUG();
}

/* Abstracted workqueue init for DPC port
 * Dpc               - Pointer to kDPC structute
 * DeferredRoutine   - Pointer to custom DPC routine
 * DeferredContext   -  Value to pass to the DeferredContext
 */
VOID KeInitializeDpc(PKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine, PVOID DeferredContext)
{
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KeInitializeDpc: init work(0x%p) - routine(0x%p) - context(0x%p)\n",
         Dpc, DeferredRoutine, DeferredContext));
   /*tasklet_init(Dpc, DeferredRoutine, (ULONG)DeferredContext);*/
   KernelInitWork(Dpc, DeferredRoutine, DeferredContext);
}

/* Abstracted schedule_work for Windows DPC port
 * Dpc               - Pointer to the DPC object
 * SystemArgument1   - Driver determined context data
 * SystemArgument2   - Driver determined context data
 * returns           - If
 */
BOOLEAN KeInsertQueueDpc(PKDPC Dpc, PVOID arg1, PVOID arg2)
{
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "KeInsertQueueDpc: schedule work(0x%p) - arg1(0x%p) - arg2(0x%p)\n",
         Dpc, arg1, arg2));
   schedule_work(Dpc);
   return TRUE;
}

/* Abstracted flush for Windows DPC port
 * Dpc   - Pointer to the DPC object
 */
BOOLEAN KeRemoveQueueDpc(PKDPC Dpc)
{
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "KeInsertQueueDpc: flush work(0x%p)\n", Dpc));
   return TRUE;
}

/* Abstracted do_gettimeofday
 * CurrentTime - Pointer to the current time on return
 *               from KeQuerySystemTime
 */
VOID KeQuerySystemTime(PLARGE_INTEGER CurrentTime)
{
   sTimeval tv;
   ULONGLONG time_us, time_100ns;

   KernelGetCurrentTime((PTIMEVALAPI)&tv, NULL, NULL);
   time_us = tv.tv_sec * 1000000LL;
   time_us += tv.tv_usec;
   time_100ns = time_us * 10LL;
   CurrentTime->QuadPart = time_100ns;
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "KeQuerySystemTime: now(%lu)\n", CurrentTime->QuadPart));
}

/* Abstraction for obtaining the jiffies count
 * TickCount - Pointer to the jiffies value on return from
 *             KeQueryTickCount
 */
VOID KeQueryTickCount(PLARGE_INTEGER TickCount)
{
#ifdef __KERNEL__
   unsigned long tick = jiffies;
   TickCount->LowPart = tick;
   TickCount->HighPart = 0;
#else
   sTimeval tv;
   gettimeofday(&tv, NULL);
   TickCount->QuadPart = (tv.tv_sec * 1000000) + tv.tv_usec;
#endif
}

/* Abstraction for obtaining the number of 100ns units
 * between clock ticks
 */
ULONG KeQueryTimeIncrement(VOID)
{
#ifdef __KERNEL__
   return (10000000 / HZ);
#else
   return (10000000 / 250);
#endif
}

BOOL KeTestSingleObject(PVOID Object)
{
   POSEVENT Event = (POSEVENT) Object;

   //RGB133PRINT((RGB133_LINUX_DBG_TODO, "KeTestSingleObject: Check Event->Signalled(0x%x)\n",
   //      Event->signalled));
   if(Event->signalled)
   {
      return TRUE;
   }
   return FALSE;
}

/* Dummy function
 */
NTSTATUS KeSaveFloatingPointState(KFLOATING_SAVE* arg)
{
   return STATUS_SUCCESS;
}

/* Dummy function
 */
NTSTATUS KeRestoreFloatingPointState(KFLOATING_SAVE* arg)
{
   return STATUS_SUCCESS;
}

/* Dummy function
 */
KIRQL KeGetCurrentIrql( )
{
   return DISPATCH_LEVEL;
}

/* Dummy function
 */
VOID KeFlushIoBuffers(PMDL Mdl, BOOLEAN ReadOperation, BOOLEAN DmaOperation)
{
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KeFlushIoBuffers TODO!!\n"));
}

/* Dummy function */
VOID KeEnterCriticalRegion(VOID)
{
}

/* Dummy function
 */
VOID KeLeaveCriticalRegion(VOID)
{
}

/* Dummy function
 */
PKTHREAD KeGetCurrentThread(void)
{
   return NULL;
}

/***** Core Kernel Library (KE) Support Routine Abstractions - END   *****/


/***** Run Time Library (RTL) Abstractions - START *****/

/* Abstracted memset
 * Destination - Pointer to memory to be filled
 * Length      - Specifies the number of bytes to be filled
 * Fill        - Specifies the value to fill the memory
 */
VOID RtlFillMemory(VOID* Destination, SIZE_T Length, UCHAR Fill)
{
   memset(Destination, Fill, Length);
}

/* Abstracted memcpy
 * Destination - Pointer to the destination of the copy
 * Source      - Pointer to the memory to be copied
 * Length      - Specifies the number of bytes to be copied
 */
VOID RtlCopyMemory(VOID* Destination, const VOID* Source, SIZE_T Length)
{
   memcpy(Destination, Source, Length);
}

/* Abstraction of memset with zero's
 * Destination - Pointer to memory to be filled
 * Length      - Specifies the number of bytes to be filled
 */
VOID RtlZeroMemory(VOID* Destination, SIZE_T Length)
{
   RtlFillMemory(Destination, Length, 0);
}

/* Abstraction of obsolete memset with zero's
 * Destination - Pointer to memory to be filled
 * Length      - Specifies the number of bytes to be filled
 */
VOID RtlZeroBytes(PVOID Destination, SIZE_T Length)
{
   RtlFillMemory(Destination, Length, 0);
}

/* Abstracted method of converting native tsc time
 * to Year, Month, Day etc (hopefully!!)
 * Time        - The current time
 * TimeFields  - structure pointer to fill in
 */

VOID RtlTimeToTimeFields(PLARGE_INTEGER Time, PTIME_FIELDS TimeFields)
{
#ifdef RGB133_CONFIG_HAVE_RTC_TIME
   struct rtc_time tm;
   rtc_time_to_tm(Time->HighPart, &tm);
   
   TimeFields->Year = tm.tm_year + 1900;
   TimeFields->Month = tm.tm_mon + 1;
   TimeFields->Day = tm.tm_mday;
   TimeFields->Hour = tm.tm_hour;
   TimeFields->Minute = tm.tm_min;
   TimeFields->Second = tm.tm_sec;
   TimeFields->Milliseconds = Time->LowPart / 1000;
   TimeFields->Weekday = tm.tm_wday;
#endif /* RGB133_CONFIG_HAVE_RTC_TIME */
}

/* Compare blocks of memory
 * Source1, Source2 - Blocks of memory to compare
 * Length           - Number of bytes to compare
 */
SIZE_T RtlCompareMemory(const PVOID* Source1, const PVOID* Source2, SIZE_T Length)
{
   SIZE_T i;
   unsigned char* src = (unsigned char*)Source1;
   unsigned char* cmp = (unsigned char*)Source2;
   
   RGB133PRINT((RGB133_LINUX_DBG_STUPID, "RtlCompareMemory: Check %d bytes\n", Length));
   for(i=0; i<Length; i++)
   {
      if(src[i] != cmp[i])
         break;
   }
   
   if(i != Length)
   {
      RGB133PRINT((RGB133_LINUX_DBG_STUPID, "RtlCompareMemory: Source1[%d](0x%x) != Source2[%d](0x%x)\n",
         i, src[i], i, cmp[i]));
   }
   
   return i;
}

/* Initialise an empty unicode string
 * DestinationString - Pointer to string to be initialised
 * Buffer            - Pointer to caller allocated buffer to be used to contain a WCHAR string
 * BufferSize        - Length, in bytes of the buffer that Buffer points to
 */
VOID RtlInitEmptyUnicodeString(PUNICODE_STRING DestinationString,
      PWCHAR Buffer, USHORT BufferSize)
{
   DestinationString->Length = 0;
   DestinationString->MaximumLength = BufferSize;
   DestinationString->Buffer = Buffer;
}

/* Initialise a unicode string
 * DestinationString - Pointer to string to be initialised
 * SourceString      - Optional pointer to null-terminated string to initialize the counted string
 */
VOID RtlInitUnicodeString(PUNICODE_STRING DestinationString,
      PWSTR SourceString)
{
   if(SourceString)
   {
      int length = wcslen(SourceString);
      if(length == 0)
      {
         DestinationString->Length = 0;
         DestinationString->Buffer = 0;
      }
      else
      {
         DestinationString->Buffer = SourceString;
         DestinationString->Length = length;
      }
   }
   else
   {
      DestinationString->Length = 0;
      DestinationString->Buffer = 0;
   }
}

/* List helper function, is the list empty
 * ListHead - Pointer to a list head
 * returns  - true if list is empty, otherwise false
 */
BOOLEAN IsListEmpty(PLIST_ENTRY ListHead)
{
   if(ListHead)
   {
      if(ListHead == ListHead->Flink)
         return TRUE;
   }
   return FALSE;
}

/***** Run Time Library (RTL) Abstractions - END   *****/

/***** Driver Support Abstractions - START *****/

/* Abstracted INIT_LIST_HEAD
 * ListHead - Pointer to LIST_ENTRY struct that serves as the
 *            list header.
 */
VOID InitializeListHead(PLIST_ENTRY ListHead)
{
   ListHead->Flink = ListHead;
   ListHead->Blink = ListHead;
}

VOID InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
   if(ListHead)
   {
      PLIST_ENTRY b = ListHead->Blink;
      Entry->Flink = ListHead;
      Entry->Blink = b;
      b->Flink = Entry;
      ListHead->Blink = Entry;
   }
}

/* Remove Entry from the list
 * Returns TRUE if list has 0 entries after removal (head of the list may still exist)
 */
BOOLEAN RemoveEntryList(PLIST_ENTRY Entry)
{
   PLIST_ENTRY f = Entry->Flink;
   PLIST_ENTRY b = Entry->Blink;

   // headless list: nothing to remove
   if(Entry->Flink == NULL || Entry->Blink == NULL)
   {
      return TRUE;
   }

   f->Blink = b;
   b->Flink = f;
   Entry->Flink = Entry->Blink = NULL;

   if(f == b)
   {
      return TRUE;
   }

   return FALSE;
}

// Remove the first entry from the list
PLIST_ENTRY RemoveHeadList(PLIST_ENTRY ListHead)
{
   PLIST_ENTRY f = ListHead->Flink;
   PLIST_ENTRY n = f->Flink;

   ListHead->Flink = n;
   n->Blink = ListHead;

   return f;
}

/* Remove the last entry from the list and return removed entry
 * If list is empty, return list head */
PLIST_ENTRY RemoveTailList(PLIST_ENTRY ListHead)
{
   PLIST_ENTRY tail = ListHead->Blink;
   PLIST_ENTRY new_tail = tail->Blink;

   ListHead->Blink = new_tail;
   new_tail->Flink = ListHead;

   return tail;
}

// Insert Entry at the top of the list (as first entry)
VOID InsertHeadList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
   PLIST_ENTRY f = ListHead->Flink;

   Entry->Flink = f;
   Entry->Blink = ListHead;
   f->Blink = Entry;
   ListHead->Flink = Entry;
}

/***** Driver Support Abstractions - END   *****/

/***** Registry (REG) Abstractions - START *****/

/* Dummy Reg Open Key
 * Ignore all params and return TRUE.
 */
NTSTATUS RegOpenKeyEx(HANDLE hKey, WCHAR* lpSubKey, UINT ulOptions,
         DWORD samDesired, PHANDLE phKey)
{
   return ERROR_SUCCESS;
}

/* Dummy Reg Query Value
 * Ignore all params and return TRUE.
 */
NTSTATUS RegQueryValueEx(HANDLE hKey, WCHAR* lpValueName, DWORD lpReserved,
         DWORD* lpType, BYTE* lpData, DWORD* lpcbData)
{
   BOOLEAN tryValue = FALSE;
   DWORD* data = (DWORD*)lpData;
   
   /* Check the keys first (they contain multiple variables which we'll then get from
    * lpValueName */
   //RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegQueryValueEx: Check hKey(0x%p)\n", (PVOID)hKey));
   if(!hKey) {
      tryValue = TRUE;
   }
   
   if(lpValueName && tryValue)
   {
      //RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegQueryValueEx: Check value(0x%p)\n",
      //   lpValueName));
      //wprintk(lpValueName);
      if(!wstrcmp(lpValueName, regFirmwarePath25Tag))
      {
         //RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegQueryValueEx: got firmware path(%s)\n", regFirmwarePath25));
         wstrcpy(regFirmwarePath25, (WCHAR*)data, strlen(regFirmwarePath25));
         //RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegQueryValueEx: got firmware path:\n"));
         wprintk((WCHAR*)data);
      }
      else if(!wstrcmp(lpValueName, regFirmwarePath27Tag))
      {
         //RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegQueryValueEx: got firmware path(%s)(%d chars)\n",
         //   regFirmwarePath27, (int)strlen(regFirmwarePath27)));
         wstrcpy(regFirmwarePath27, (WCHAR*)data, (int)strlen(regFirmwarePath27));
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegQueryValueEx: got firmware path:\n"));
         wprintk((WCHAR*)data);
      }
      else
      {
         //RGB133PRINT((RGB133_LINUX_DBG_WARNING, "RegQueryValueEx: got unknown key(%s) and value(%s)\n",
         //   (char*)hKey, (char*)lpValueName));
         //RGB133PRINT((RGB133_LINUX_DBG_WARNING, "RegQueryValueEx: didn't match (%s) or (%s)\n",
         //   regFirmwarePath25, regFirmwarePath27));
         return !ERROR_SUCCESS;
      }
      
   }
   return ERROR_SUCCESS;
}

/* Dummy Reg Close Key
 * Ignore all params and return TRUE.
 */
NTSTATUS RegCloseKey(PHANDLE hKey)
{
   return STATUS_SUCCESS;
}

/* Dummy Reg Enum Key
 * hKey - Pointer to array of strings
 * dwIndex - Array index
 * lpName - String to hold array name
 */
NTSTATUS RegEnumKey(PVOID pDE, HANDLE hKey, DWORD dwIndex, WCHAR* lpName, DWORD not_used)
{
   if(dwIndex > 0)
      return !ERROR_SUCCESS;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "RegEnumKey: Copy string %s (%d bytes) from 0x%p to 0x%p\n",
      (char*)hKey, (int)strlen((char*)hKey)+1, hKey, lpName));
   memcpy(lpName, (char*)hKey, strlen((char*)hKey)+1);
   
   return ERROR_SUCCESS;
}

/* Dummy RegSetValueEx
 * Ignore all params and return TRUE.
 */
NTSTATUS RegSetValueEx(HANDLE hKey, WCHAR *pszValueName, DWORD Unused, DWORD dwType, BYTE *pDataBuffer, DWORD dwBufferSize)
{
   return STATUS_SUCCESS;
}
/***** Registry (REG) Abstractions - END   *****/

/***** IO Manager (IO) Abstractions - START *****/

/* Dummy abstraction
 */
VOID IoDisconnectInterrupt(PKINTERRUPT InterruptObject)
{
}

/* Abstracted INIT_WORK structure
 * work     - The work structure to use
 * returns  - The initialised work structure 
 */
PIO_WORKITEM IoAllocateWorkItem(PDEVICE_OBJECT pWI)
{
   if(pWI)
   {
      RGB133PRINT((RGB133_LINUX_DBG_IRQ, "IoAllocateWorkItem: Init work(0x%p - &0x%p)\n", pWI, &pWI));
      KernelInitWork(pWI, 0, 0);
   }
   else
      return 0;
   return (PIO_WORKITEM)pWI;
}

/* Abstracted queue work
 * IoWorkItem  - Not used
 * WorkerRoutine  - Pointer to the routine to run on the workqueue
 * QueueType      - Name of the workqueue to use
 * Context        - Driver specific info (unused)
 */
struct tagWorkContext;

VOID IoQueueWorkItem(PIO_WORKITEM IoWorkItem,
      PIO_WORKITEM_ROUTINE WorkerRoutine, WORK_QUEUE_TYPE QueueType,
      PVOID Context)
{
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "IoQueueWorkItem: pWI(0x%p &0x%p) - routine(0x%p) - type(%s) - pWC(0x%p)\n",
         IoWorkItem, &IoWorkItem, WorkerRoutine, QueueType, Context));
   KernelPrepareWork(IoWorkItem, WorkerRoutine, Context);

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "IoQueueWorkItem: schedule work, pWI(0x%p)\n",
      IoWorkItem));
   schedule_work(IoWorkItem);
}

/* Abstraction for clearing scheduled work
 */
VOID IoFreeWorkItem(PIO_WORKITEM pWI)
{
}

/* Abstraction for setup of an MDL virtual memory struct
 * VirtualAddress    - Pointer to the virtual address of the buffer
 * Length            - Length, in bytes, of the buffer
 * SecondaryBuffer   - Not Used
 * ChargeQuota       - Not Used
 * Irp               - Not Used
 */
PMDL IoAllocateMdl(PVOID VirtualAddress, ULONG Length, BOOLEAN SecondaryBuffer,
      BOOLEAN ChargeQuota, PIRP Irp0)
{
   PMDL mdl = 0;
   PMDL p;
   
   mdl = KernelVzalloc(sizeof(MDL));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "IoAllocateMdl: alloc %d bytes to mdl(0x%p)\n",
         (int)sizeof(MDL), mdl));
   if(mdl == NULL)
      return NULL;
   
   /* Set up */ 
   mdl->MdlFlags = 0x0;  
   mdl->Size = Length;
   mdl->StartVa = VirtualAddress;
   mdl->map = 0;
   
   if(Irp0)
   {
      if(SecondaryBuffer)
      {
         p = Irp0->MdlAddress;
         while(p->Next)
            p = p->Next;
         p->Next = mdl;
      }
      else
      {
         Irp0->MdlAddress = mdl;
      }
   }
   
   return mdl;   
}

/* Abstraction for tear down of an MDL virtual memory struct
 * Mdl - Pointer to struct to be torn down
 */
VOID IoFreeMdl(PMDL Mdl)
{
   if(Mdl)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "IoFreeMdl: free Mdl(0x%p)\n", Mdl));
      KernelVfree(Mdl);
   }
}

/* Get a pointer to a DMA Adapter structure
 * PhysicalDeviceObject - 
 * DeviceDescription    - Description of the device
 * NumberOfMapRegisters - Pointer to, on output, the max number of registers
 *                        that the driver can allocate for any DMA transfer
 *                        operation
 * returns              - A pointer to a DMA_ADAPTEr object
 */
 
static NTSTATUS LinuxGetScatterGatherList(PDMA_ADAPTER DmaAdapter, PDEVICE_OBJECT DeviceObject,
                                          PMDL Mdl, PVOID CurrentVa, ULONG Length,
                                          PDRIVER_LIST_CONTROL ExecutionRoutine,
                                          PVOID Context, BOOLEAN WriteToDevice);
                                   
static NTSTATUS LinuxPutScatterGatherList(PDMA_ADAPTER DmaAdapter, PSCATTER_GATHER_LIST pWinSGList,
                                          int BufferIndex, BOOLEAN WriteToDevice);
                                   
static DMA_OPERATIONS LinuxDmaOperations = {
   .GetScatterGatherList = LinuxGetScatterGatherList,
   .PutScatterGatherList = LinuxPutScatterGatherList,
   .PutDmaAdapter = LinuxPutDmaAdapter,
   .Size = 0
};

struct rgb133_unified_buffer * CaptureGetLiveBuffer(struct rgb133_handle * h, PVOID pVa)
{
   int i;
   struct rgb133_unified_buffer * pBuf = 0;

   if(rgb133_is_reading(h))
      return h->buffers[0];

   for (i=0;i<h->numbuffers;i++)
   {
      pBuf = h->buffers[i];

      if(pBuf)
      {
         if (pBuf->pMemory == pVa)
         {
            RGB133PRINT((RGB133_LINUX_DBG_INOUT, "CaptureGetLiveBuffer: Returning buffer[%d](%p)\n", pBuf->index, pBuf));
            return pBuf;
         }
      }
   }
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "CaptureGetLiveBuffer: Returning no buffer.\n"));
   return NULL;
}

NTSTATUS LinuxGetScatterGatherList(PDMA_ADAPTER DmaAdapter, PDEVICE_OBJECT DeviceObject,
                                   PMDL Mdl, PVOID CurrentVa, ULONG Length,
                                   PDRIVER_LIST_CONTROL ExecutionRoutine,
                                   PVOID Context, BOOLEAN WriteToDevice)
{
#ifndef RGB133_USER_MODE
   PDESTDESCAPI pDD = 0;
   PRGBCAPTUREAPI pRGBCapture = 0;
   PDEAPI pDE = 0;
   struct rgb133_dev* dev = 0;
   struct pci_dev* pdev = 0;
   struct rgb133_unified_buffer* buf = 0;
   struct rgb133_handle * h = 0;
   ULONG plane = 0;

   RGB133_KERNEL_DMA* kernel_dma = 0;
   PSCATTER_GATHER_LIST pWinSGL = 0;

   int capture = -1;
   int channel = 0;

   int i, j=20;

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxGetScatterGatherList: START Mdl(0x%p)\n", Mdl));

   if(Context == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: Context is NULL\n"));
      return STATUS_INVALID_PARAMETER;
   }

   pDD = DeviceGetPDDFromContext(Context);
   if(pDD == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: Context->pDD is NULL\n"));
      return STATUS_INVALID_PARAMETER;
   }

   pRGBCapture = SGGetCapturePtr(pDD);
   if(pRGBCapture == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: pRGBCapture for pDD(0x%p) is NULL\n",
            pDD));
      return STATUS_INVALID_PARAMETER;
   }

   pDE = (PDEAPI)CaptureGetDevicePtr(pRGBCapture);
   if(pDE == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: pDE for pRGBCapture(0x%p) is NULL\n",
            pRGBCapture));
      return STATUS_INVALID_PARAMETER;
   }

   dev = DeviceGetRGB133(pDE);
   if(dev == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: dev for pRGBCapture(0x%p) pDE(0x%p) is NULL\n",
            pRGBCapture, pDE));
      return STATUS_INVALID_PARAMETER;
   }

   pdev = dev->core.pci;
   if(pdev == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: pdev for dev(0x%p) is NULL\n",
            dev));
      return STATUS_INVALID_PARAMETER;
   }

   h = CaptureGetLinuxCapture(pRGBCapture);
   if(h == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: h for dev[%d][%d][%d](0x%p) pRGBCapture(0x%p) pDE(0x%p) is NULL\n",
            dev->index, CaptureGetChannelNumber(pRGBCapture), CaptureGetCaptureNumber(pRGBCapture),
            dev, pRGBCapture, pDE));
      return STATUS_INVALID_PARAMETER;
   }

   capture = CaptureGetCaptureNumber(pRGBCapture);
   channel = CaptureGetChannelNumber(pRGBCapture);

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "LinuxGetScatterGatherList: pRBCapture(0x%p), h(0x%p)\n",
      pRGBCapture, h));

   buf = CaptureGetLiveBuffer(h, CurrentVa); // CurrentVa *must* match one of the buffers' addresses.
   if(buf == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: buf for pRGBCapture[%d][%d][%d](0x%p) CurrentVa(0x%lx) is NULL\n",
            dev->index, channel, capture, pRGBCapture, CurrentVa));
      return STATUS_INVALID_PARAMETER;
   }
   plane = DeviceGetPlaneFromContext(Context);
   pWinSGL = (PSCATTER_GATHER_LIST)buf->pWinSGL[plane];
   kernel_dma = rgb133_buffer_get_kernel_dma(&buf, plane);
      
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "LinuxGetScatterGatherList: Called - DmaAdapter(0x%p), Mdl(0x%p), DeviceObject(0x%p)\n",
      DmaAdapter, Mdl, DeviceObject));
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "LinuxGetScatterGatherList: Called - CurrentVa(0x%p), Length(%lu)\n",
      CurrentVa, Length));
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "LinuxGetScatterGatherList: Called - pRGBCapture[%d](0x%p), pDE(0x%p) - pdev(0x%p)\n",
      capture, pRGBCapture, pDE, pdev));

   /* Map SG List */
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxGetScatterGatherList: Map SG - pdev(0x%p) - kernel_dma(0x%p) - SGList(0x%p) - page_count(%d)\n",
      pdev, kernel_dma, kernel_dma->pSGList, kernel_dma->page_count));
   kernel_dma->SG_length = dma_map_sg(&pdev->dev, kernel_dma->pSGList, kernel_dma->page_count, DMA_FROM_DEVICE);
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxGetScatterGatherList: Map SG - Length(%d) page_count(%d)\n",
         kernel_dma->SG_length, kernel_dma->page_count));
   sg_mark_end(kernel_dma->pSGList, kernel_dma->SG_Length);
   
   if(kernel_dma->SG_length > 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxGetScatterGatherList: Fill SG Array - kernel_dma(0x%p) - data(0x%p) - length(%d)\n",
            kernel_dma, Mdl->StartVa, kernel_dma->SG_length));
      rgb133_kernel_dma_fill_sg_array(kernel_dma, (unsigned long)Mdl->StartVa, 0, -1);
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxGetScatterGatherList: Failed to Fill SG Array - kernel_dma(0x%p) - data(0x%p)\n",
            kernel_dma, Mdl->StartVa));
   }
   
   /* Sync the DMA */
   rgb133_kernel_dma_sync_for_device(pdev, kernel_dma->pSGList, kernel_dma->page_count);
   
   /* Now fill up a windows style SGL */
   if(pWinSGL)
   {
      pWinSGL->NumberOfElements = kernel_dma->page_count;
      for(i=0; i<kernel_dma->page_count; i++)
      {
         pWinSGL->Elements[i].Address.QuadPart = kernel_dma->pSGArray[i].src;
         pWinSGL->Elements[i].Length = kernel_dma->pSGArray[i].size;

         if(j-->0)
             RGB133PRINT((RGB133_LINUX_DBG_STUPID, "LinuxGetScatterGatherList: pWinSGL->Elements[%d]->addr(0x%llx) (%lu)\n",
               i, pWinSGL->Elements[i].Address.QuadPart, pWinSGL->Elements[i].Length));
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxGetScatterGatherList: Failed to alloc %d bytes to WinSGL\n",
            (int)sizeof(SCATTER_GATHER_LIST)));
      return STATUS_INSUFFICIENT_RESOURCES;
   }
   
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "LinuxGetScatterGatherList: Call SGT ExecutionRoutine callback(0x%p)\n",
      ExecutionRoutine));
   (*ExecutionRoutine)(DeviceObject, NULL /*pIrp*/, pWinSGL, Context);
#else
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "LinuxGetScatterGatherList: Not Required?\n"));
#endif /* RGB133_USER_MODE */
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxGetScatterGatherList: END\n"));
   return STATUS_SUCCESS;
}

NTSTATUS LinuxPutScatterGatherList(PDMA_ADAPTER DmaAdapter, PSCATTER_GATHER_LIST pWinSGL,
                                   int BufferIndex, BOOLEAN WriteToDevice)
{
#ifndef RGB133_USER_MODE
   PDESTDESCAPI pDD = 0;
   PRGBCAPTUREAPI pRGBCapture = 0;
   PDEAPI pDE = 0;
   struct rgb133_dev* dev = 0;
   struct pci_dev* pdev = 0;
   struct rgb133_handle* h = 0;

   int capture = -1;
   int channel = -1;
   ULONG plane = 0;

   RGB133_KERNEL_DMA* kernel_dma = 0;

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxPutScatterGatherList: START buffer[%d]\n", BufferIndex));

   if(DmaAdapter == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxPutScatterGatherList: DmaAdapter is NULL\n"));
      return STATUS_INVALID_PARAMETER;
   }
   pDD = DeviceGetPDDFromContext(DmaAdapter->Context);

   if(pDD == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxPutScatterGatherList: pDD for DmaAdapter(0x%p) is NULL\n",
            DmaAdapter));
      return STATUS_INVALID_PARAMETER;
   }
   pRGBCapture = SGGetCapturePtr(pDD);

   if(pRGBCapture == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxPutScatterGatherList: pRGBCapture for pDD(0x%p) is NULL\n",
            pDD));
      return STATUS_INVALID_PARAMETER;
   }
   RGB133_DEBUG_ASSERT(pRGBCapture != 0x5b5b5b5b);
   pDE = CaptureGetDevicePtr(pRGBCapture);

   if(pDE == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxPutScatterGatherList: pDE for pRGBCapture[%d](0x%p) is NULL\n",
            CaptureGetCaptureNumber(pRGBCapture), pRGBCapture));
      return STATUS_INVALID_PARAMETER;
   }
   dev = DeviceGetRGB133(pDE);

   if(dev == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxPutScatterGatherList: dev for pDE[%d].[%d](0x%p) is NULL\n",
            CaptureGetChannelNumber(pRGBCapture), CaptureGetCaptureNumber(pRGBCapture),
            pDE));
      return STATUS_INVALID_PARAMETER;
   }
   pdev = dev->core.pci;
   h = CaptureGetLinuxCapture(pRGBCapture);

   capture = CaptureGetCaptureNumber(pRGBCapture);
   channel = CaptureGetChannelNumber(pRGBCapture);
   
   plane = DeviceGetPlaneFromContext(DmaAdapter->Context);
   if(rgb133_is_reading(h))
      kernel_dma = rgb133_buffer_get_kernel_dma(&h->buffers[0], plane);
   else
      kernel_dma = rgb133_buffer_get_kernel_dma(&h->buffers[BufferIndex], plane);
   
   if(kernel_dma == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "LinuxPutScatterGatherList: kernel_dma for dev[%d][%d][%d](0x%p), pRGBCapture(0x%p) is NULL\n",
            dev->index, h->channel, h->capture, dev, pRGBCapture));
      return STATUS_INVALID_PARAMETER;
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "LinuxPutScatterGatherList: pRGBCapture[%d](0x%p), Free pWinSGL(0x%p)\n",
      capture, pRGBCapture, pWinSGL));
   
   /* Unmap SG List */
   if(kernel_dma->SG_length)
   {
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxPutScatterGatherList: unmap DMA - SGList(0x%p)\n",
         (void*)kernel_dma->pSGList));
      dma_unmap_sg(&pdev->dev, kernel_dma->pSGList, kernel_dma->page_count, DMA_FROM_DEVICE);
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxPutScatterGatherList: no need to unmap SGList\n"));
   }

#else
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "LinuxGetScatterGatherList: Not Required?\n"));
#endif /* RGB133_USER_MODE */
   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "LinuxPutScatterGatherList: END buffer[%d]\n", BufferIndex));
   return STATUS_SUCCESS;
}

PDMA_ADAPTER IoGetDmaAdapter(PDEVICE_OBJECT PhysicalDeviceObject,
                             PDEVICE_DESCRIPTION DeviceDescription,
                             PULONG NumberOfMapRegisters)
{
   PDMA_ADAPTER pDmaAdapter = KernelVzalloc(sizeof(DMA_ADAPTER));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "IoGetDmaAdapter: alloc %d bytes to pDmaAdapter(0x%p)\n",
         sizeof(DMA_ADAPTER), pDmaAdapter));
   if(pDmaAdapter == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "IoGetDmaAdapter: Failed to alloc %d bytes to pDmaAdapter\n",
            sizeof(DMA_ADAPTER)));
      return 0;
   }
   LinuxDmaOperations.Size = sizeof(LinuxDmaOperations);
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "IoGetDmaAdapter: GetScatterGatherList(0x%p), Size(%lu)\n",
      LinuxDmaOperations.GetScatterGatherList, LinuxDmaOperations.Size));
   
   pDmaAdapter->DmaOperations = &LinuxDmaOperations;
   pDmaAdapter->Size = sizeof(*pDmaAdapter);
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "IoGetDmaAdapter: DmaOperations(0x%p), Size(%u), Version(%u)\n",
         pDmaAdapter->DmaOperations, pDmaAdapter->Size, pDmaAdapter->Version));
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "IoGetDmaAdapter: return pDmaAdapter(0x%p)\n",
         pDmaAdapter));
   return pDmaAdapter;
}

VOID LinuxPutDmaAdapter(PDMA_ADAPTER pDmaAdapter)
{
   if(pDmaAdapter)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "LinuxPutDmaAdapter: free pDmaAdapter(0x%p)\n",
            pDmaAdapter));
      KernelVfree(pDmaAdapter);
   }
}

/* Dummy function, does nothing
 * pIrp - Pointer to an IRP
 */
VOID IoMarkIrpPending(PIRP Irp)
{
}

/* Dummy function, does nothing
 * pIrp          - Pointer to an IRP
 * PriorityBoost - System defined constant by which to increment the run-time
 *                 priority of the original thread
 */
VOID IoCompleteRequest(PIRP pIrp, CCHAR PriorityBoost)
{
} 

NTSTATUS IoGetDeviceProperty(PDEVICE_OBJECT DeviceObject, DEVICE_REGISTRY_PROPERTY DeviceProperty,
                             ULONG BufferLength, PVOID PropertyBuffer, PULONG ResultLength)
{
   if(DeviceProperty == DevicePropertyLocationInformation)
   {
      PropertyBuffer = L"PCI bus 3, device 2, function 1";
   }
   else
      return STATUS_NO_MATCH;

   return STATUS_SUCCESS;
}

/* Dummy function, does nothing
 * pIrp          - Pointer to Irp
 * Always returns false.
 */
BOOLEAN IoIs32bitProcess(PIRP pIrp)
{
   return FALSE;
}

/* Abstraction for getting a pointer to the caller's I/O stack location from the specified IRP
 * Irp     - A pointer to the IRP
 * returns - A pointer to an IO_STACK_LOCATION structure that contains the I/O stack location for the driver
 */
PIO_STACK_LOCATION IoGetCurrentIrpStackLocation(_In_ PIRP Irp)
{
   return Irp->irpStack;
}

/***** IO Manager (IO) Abstractions - END   *****/

/***** Memory Manager (MM) Abstractions - START *****/

/* Abstraction for get_free_pages
 * MemoryDescriptorList - Pointer to the virtual memory buffer structure
 * AccessMode           - GFP_KERNEL, GFP_ATOMIC, etc
 * Operation            - Not used
 */
VOID MmProbeAndLockPages(PMDL MemoryDescriptorList, OSPROCESSOR_MODE AccessMode,
      LOCK_OPERATION Operation)
{
   struct page** map = MemoryDescriptorList->map;
   unsigned long base = (unsigned long)MemoryDescriptorList->StartVa;
   unsigned long size = (unsigned long)MemoryDescriptorList->Size;
   unsigned long first = (base & PAGE_MASK) >> PAGE_SHIFT;
   unsigned long last = ((base+size-1) & PAGE_MASK) >> PAGE_SHIFT;
   int page_count = last - first + 1;
   
   int err, i, j=0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "MmProbeAndLockPages: START\n"));
   MemoryDescriptorList->AccessMode = AccessMode;
      
   if(AccessMode == UserMode)
   {
      struct rw_semaphore* sem =
#ifdef RGB133_CONFIG_HAVE_MMAP_LOCK_IN_MM_STRUCT
         &current->mm->mmap_lock;
#else
         &current->mm->mmap_sem;
#endif

      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "MmProbeAndLockPages: get_user_pages from(0x%lx) into map(0x%p) \n", (base & PAGE_MASK), map));
      down_read(sem);

      /* From online discussions on the usage of parameter 'force' of get_user_pages():
      (1): I guess the question is what happens if userspace tells us to
      write to a read-only mapping that userspace could have mapped
      writable?
      (2): Always ask get_user_pages() for writable pages, but pass force=1
      if the consumer has only asked for read-only pages. This fixes a
      problem registering memory that has just been allocated but not
      touched yet, while allowing registration of read-only memory to
      continue to work.
      (3): I'd love to say go ahead with this, it looks so obviously correct...
      So, ugly as it looks, I think you have to stick with write=1 force=!write.
      Does that make sense now?
       */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0) /* kernel >= 4.9.0 */
      err = get_user_pages((base & PAGE_MASK), page_count, (!Operation ? FOLL_WRITE : 0), map, NULL);
#else /* kernel < 4.9.0 */
#ifdef RGB133_CONFIG_HAVE_GET_USER_PAGES_6_ARGUMENTS
      err = get_user_pages((base & PAGE_MASK), page_count, !Operation, Operation,
                           map, NULL);
#else
      err = get_user_pages(current, current->mm,
                           (base & PAGE_MASK), page_count, !Operation, Operation,
                           map, NULL);
#endif
#endif /* kernel >= 4.9.0 */
      up_read(sem);
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "MmProbeAndLockPages: get_user_pages(%d)\n", err));
                           
      if(page_count != err)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "MmProbeAndLockPages: failed to get_user_pages\n"));
         return;
      }
      
      for(i=0; i<j; i++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "MmProbeAndLockPages: map[%d](0x%p)\n", i, map[i]));
      }
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "MmProbeAndLockPages: vmalloc_to_page from(0x%lx) into map(0x%p) - %d pages\n",
         (base & PAGE_MASK), map, page_count));
      for(i=0 ; i<page_count; i++)
      {
         MemoryDescriptorList->map[i] = vmalloc_to_page((void*)(base + (i << PAGE_SHIFT)));
      }
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "MmProbeAndLockPages: vmalloc_to_page(%d)\n", i));
                           
      if(page_count != i)
      {
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "MmProbeAndLockPages: failed to get_user_pages\n"));
         return;
      }
      
      for(i=0; i<j; i++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "MmProbeAndLockPages: map[%d](0x%p)\n", i, map[i]));
      }
   }
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "MmProbeAndLockPages: END\n"));
}

/* Abstraction for free_pages
 * MemoryDescriptorList - Pointer to a description of the memory buffer
 */
VOID MmUnlockPages(PMDL MemoryDescriptorList)
{
   if(MemoryDescriptorList)
   {
      struct page** map = MemoryDescriptorList->map;
      unsigned long base = (unsigned long)MemoryDescriptorList->StartVa;
      unsigned long size = (unsigned long)MemoryDescriptorList->Size;
      unsigned long first = (base & PAGE_MASK) >> PAGE_SHIFT;
      unsigned long last = ((base+size-1) & PAGE_MASK) >> PAGE_SHIFT;
      int page_count = last - first + 1;

      int i;

      if(MemoryDescriptorList->AccessMode == UserMode)
      {
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "MmUnlockPages: check map(0x%p) != NULL\n",
               map));
         if(map != NULL)
         {
            RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "MmUnlockPages: check and release %d pages from map(0x%p)\n",
               page_count, map));
            for(i=0; i<page_count; i++)
            {
               if(map[i] != NULL)
               {
                  if(!PageReserved(map[i]))
                     SetPageDirty(map[i]);
                  page_cache_release(map[i]);
               }
            }
         }
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "MmUnlockPages: vmalloc'd memory(0x%lx)\n",
            base));
      }
   }
}

PVOID MmGetSystemAddressForMdlSafe(PMDL pMdl, ULONG Priority)
{
   return pMdl->StartVa;
}

VOID MmUnmapLockedPages(PVOID BaseAddress, PMDL Mdl)
{
}

/* Return the base virtual address of an Mdl buffer 
 * Mdl - Pointer to MDL
 */
PVOID MmGetMdlVirtualAddress(PMDL Mdl)
{
   if(Mdl)
      return Mdl->StartVa;
   else
      return 0;
}

/* Return the byte count of the Mdl 
 * Mdl - Pointer to MDL
 */
ULONG MmGetMdlByteCount(PMDL pMdl)
{
   if(pMdl)
      return pMdl->Size;
   else
      return 0;
}
/***** Memory Manager (MM) Abstractions - END   *****/

/***** Stream Class (Stream) Abstractions - START *****/
VOID StreamRegistryOverrides (PVOID pDE)
{
}

/***** Stream Class (Stream) Abstractions - END   *****/

/***** Adapter (Adapter) Abstractions - START *****/
VOID AdapterEventRun ( PVOID arg )
{
}

VOID AdapterEventStop ( PVOID arg )
{
}

VOID AdapterEventFromAnyThread ( PVOID arg, int event )
{
}

/***** Adapter (Adapter) Abstractions - END   *****/

/***** String Functions - START *****/
/* Wide string length */
int wcslen(const PWSTR w_str)
{
   int i, Length = 0;
   char* _w_str = (char*)w_str;
   
   RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrlen: of 0x%p\n", _w_str));
    
   /* Find the Length */
   for(i=0;i<256;i++)
   {
      if(_w_str[i]) {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrlen: w_str[%d](%c)\n", i, _w_str[i]));
         Length++;
         i+=3;
         if(Length >= 256) /* Only count 256 characters */
            break; 
      }
      else
      {
         break;
      }
   }
   RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrlen: length(%d)\n", Length));
   return Length;
}

/* Wide string printk */
VOID wprintk(WCHAR* wstr)
{
   char* w_str = (char*)wstr;
   char* output;
   int i, Length = 0;
   
   /* Find the Length */
   Length = wcslen(wstr);
   
   output = KernelVzalloc(Length+4);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "wprintk: alloc %d bytes to output(0x%p)\n", Length+4, output));

   if(output)
   {
      for(i=0;i<(Length*4);i++) {
         output[i/4] = w_str[i];
         i+=3;
      }
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "%s\n", output));

      RGB133PRINT((RGB133_LINUX_DBG_MEM, "wprintk: free output(0x%p)\n", output));
      KernelVfree(output);
   }   
}

/* Wide string compare against std char */
BOOLEAN wstrcmp(WCHAR* wcompare, const char* with)
{
   char* compare = (char*)wcompare;
   int Length = strlen(with);
   int i, j=0;
   BOOLEAN matched = TRUE;
   
   for(i=0; i<=Length; i++)
   {
      if(with[i] == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrcmp: end of %s, matched(%d)\n",
            with, matched));
         break;
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrcmp: compare[%d](%c)(0x%x)\n",
            j, compare[j], compare[j]));
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrcmp: with[%d]   (%c)(0x%x)\n",
            i, with[i], with[i]));
         if(compare[j] != with[i])
         {
            RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrcmp: match failed on char(%d)\n", i));
            matched = FALSE;
            break;
         }
         j+=4;
      }
   }
   return !matched;
}

/* Normal string copy to wide string */
VOID wstrcpy(char* src, WCHAR* dest, int Length)
{
   int i, j=0;
   char* _dest = (char*)dest;
   
   RGB133PRINT((RGB133_LINUX_DBG_STUPID, "wstrcpy: %d bytes from 0x%p to 0x%p\n",
      Length, src, dest));
   if(src && _dest)
   {
      for(i=0; i<Length; i++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "Put src[%d](%c) into _dest[%d]\n",
            i, src[i], j));
         _dest[j++] = src[i];
         _dest[j++] = 0;
         _dest[j++] = 0;
         _dest[j++] = 0;
      }
   }
   _dest[j] = 0;
}

/* Wide string copy to normal string */
VOID strwcpy(WCHAR* src, char* dest, int Length)
{
   int i, j=0;
   char* _src = (char*)src;
   
   RGB133PRINT((RGB133_LINUX_DBG_STUPID, "strwcpy: %d bytes from 0x%p to 0x%p\n",
      Length, _src, dest)); 
   if(_src && dest)
   {
      Length *= 4;
      for(i=0; i<Length;i++)
      {
         RGB133PRINT((RGB133_LINUX_DBG_STUPID, "Put _src[%d](%c) into dest[%d]\n",
            i, _src[i], j));
         dest[j++] = _src[i];
         i+=3;
      }
      dest[j] = 0;
   }
}

/* Wide string copy with length */
VOID wcsncpy(PWCHAR dest, const PWSTR src, int n)
{
   int i = 0;
   if(dest && src)
   {
      int max = wcslen(src);
      if(n > max)
         n = max;
      for(i=0; i<n; i++)
      {
         dest[i] = src[i];
      }
   }
}

/***** String Functions - END   *****/

/***** Process and Thread Manager (Ps) Routines Abstractions - START *****/

/* Abstraction for getting current process id
 * MemoryDescriptorList - Pointer to a description of the memory buffer
 * returns  - process ID of the current thread's process */
HANDLE PsGetCurrentProcessId(void)
{
   return KernelGetProcessId();
}

/***** Process and Thread Manager (Ps) Routines Abstractions - END *****/

/***** Windows Management Instrumentation (WMI) Routines Abstractions - START *****/

VOID WmiInitTemperatureAddresses(PVOID pDE)
{
}

NTSTATUS WmiRegisterDriver(PDRIVER_OBJECT pDriverObject,
                           PUNICODE_STRING pRegistryPath)
{
   return STATUS_SUCCESS;
}

NTSTATUS WmiUnregisterDriver(PDRIVER_OBJECT pDriverObject)
{
   return STATUS_SUCCESS;
}

NTSTATUS WmiRegisterDevice(PDRIVER_OBJECT pDriverObject,
                           PDEVICE_OBJECT pDeviceObject,
                           PDEVICE_OBJECT pPhysicalDeviceObject,
                           PVOID pDE)
{
   return STATUS_SUCCESS;
}

NTSTATUS WmiUnregisterDevice(PDEVICE_OBJECT pDeviceObject,
                             PVOID pDE)
{
   return STATUS_SUCCESS;
}

NTSTATUS WmiRaiseInputSignalEvent(PVOID pDE,
                                  ULONG instanceIndex)
{
   return STATUS_SUCCESS;
}

/***** Windows Management Instrumentation (WMI) Routines Abstractions - END *****/

/***** DUMMY FUNCTIONS - START  *****/
   /* NONE */
/***** DUMMY FUNCTIONS - END    *****/
