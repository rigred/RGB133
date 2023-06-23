/*
 * OSAPI.c
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

#ifdef INCLUDE_OSAPI_EXT
#include <linux/sched.h>

#include "rgb133config.h"

#include "rgb_base_types.h"
#include "OSAPI_TYPES.h"
#include "rgb_windows_types.h"
#include "rgb133kernel.h"
#include "rgb133timer.h"
#include "OSAPI.h"
#endif /* INCLUDE_OSAPI_EXT */

#ifdef INCLUDE_OSAPI
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/types.h>

#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

#include "OSAPI.h"
#include "OSAPIDEBUG.h"
#include "OSAPIERROR.h"

/* Debug */
int32_t osapi_debug_level = OSAPI_DBG_ERRWARN;

void OSSetDebugLevel(int32_t debug_level)
{
   osapi_debug_level = debug_level;
}

/************************************ MEMORY **********************************/
void* OSAllocateMemory(eOSAPIMEMORY_T eType, uint64_t size, uint64_t flags)
{
   void* ptr = 0;
   switch(eType)
   {
      case OSAPI_MEMORY_TYPE_VIRTUAL:
         ptr = vmalloc(size);
         OSAPIPRINT(OSAPI_DBG_MEM, "Allocated %llu bytes to ptr(0x%p)\n", size, ptr);
         break;
      case OSAPI_MEMORY_TYPE_KERNEL_LOGICAL:
         ptr = kmalloc(size, flags);
         OSAPIPRINT(OSAPI_DBG_MEM, "Allocated %llu bytes to ptr(0x%p)\n", size, ptr);
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Memory Type: %d\n", eType);
         break;
   }
   return ptr;
}

void OSFreeMemory(eOSAPIMEMORY_T eType, void* pContext, uint64_t flags)
{
   switch(eType)
   {
      case OSAPI_MEMORY_TYPE_VIRTUAL:
         if(pContext)
         {
            OSAPIPRINT(OSAPI_DBG_MEM, "Free pContext(0x%p)\n", pContext);
            vfree(pContext);
         }
         break;
      case OSAPI_MEMORY_TYPE_KERNEL_LOGICAL:
         if(pContext)
         {
            OSAPIPRINT(OSAPI_DBG_MEM, "Free pContext(0x%p)\n", pContext);
            kfree(pContext);
         }
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Memory Type: %d\n", eType);
         break;
   }
}

void OSMemoryBarrier()
{
   mb();
}

/************************************ LOCKS ***********************************/
void* OSAllocateLock(eOSAPILOCK_T eType)
{
   void* ptr = 0;

   switch(eType)
   {
      case OSAPI_LOCK_TYPE_SPINLOCK:
      case OSAPI_LOCK_TYPE_INTERRUPT_SPINLOCK:
         ptr = OSAllocateMemory(OSAPI_MEMORY_TYPE_KERNEL_LOGICAL,
            sizeof(spinlock_t), GFP_KERNEL);
         break;
      case OSAPI_LOCK_TYPE_MUTEX:
         ptr = OSAllocateMemory(OSAPI_MEMORY_TYPE_KERNEL_LOGICAL,
            sizeof(struct mutex), GFP_KERNEL);
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Lock Type: %d\n", eType);
         break;
   }
   return ptr;
}

void OSInitialiseLock(eOSAPILOCK_T eType, void* pContext)
{
   switch(eType)
   {
      case OSAPI_LOCK_TYPE_SPINLOCK:
      case OSAPI_LOCK_TYPE_INTERRUPT_SPINLOCK:
         spin_lock_init((spinlock_t*)pContext);
         break;
      case OSAPI_LOCK_TYPE_MUTEX:
         mutex_init((struct mutex*)pContext);
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Lock Type: %d\n", eType);
         break;
   }
}

void OSFreeLock(eOSAPILOCK_T eType, void* pContext)
{
   switch(eType)
   {
      case OSAPI_LOCK_TYPE_SPINLOCK:
      case OSAPI_LOCK_TYPE_INTERRUPT_SPINLOCK:
      case OSAPI_LOCK_TYPE_MUTEX:
         OSFreeMemory(eType, pContext, 0);
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Lock Type: %d\n", eType);
         break;
   }
}

uint64_t OSAcquireLock(eOSAPILOCK_T eType, void* pContext1, void* pContext2)
{
   unsigned long flags = 0;

   switch(eType)
   {
      case OSAPI_LOCK_TYPE_SPINLOCK:
         spin_lock((spinlock_t*)pContext1);
         break;
      case OSAPI_LOCK_TYPE_INTERRUPT_SPINLOCK:
         spin_lock_irqsave((spinlock_t*)pContext1, flags);
         break;
      case OSAPI_LOCK_TYPE_MUTEX:
         mutex_lock((struct mutex*)pContext1);
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Lock Type: %d\n", eType);
         break;
   }
   return (uint64_t)flags;
}

void OSReleaseLock(eOSAPILOCK_T eType, void* pContext1, void* pContext2, uint64_t flags)
{
   switch(eType)
   {
      case OSAPI_LOCK_TYPE_SPINLOCK:
         spin_unlock((spinlock_t*)pContext1);
         break;
      case OSAPI_LOCK_TYPE_INTERRUPT_SPINLOCK:
         spin_unlock_irqrestore((spinlock_t*)pContext1, flags);
         break;
      case OSAPI_LOCK_TYPE_MUTEX:
         mutex_unlock((struct mutex*)pContext1);
         break;
      default:
         OSAPIPRINT(OSAPI_DBG_WARNING, "Unhandled Lock Type: %d\n", eType);
         break;
   }
}

/*********************************** COUNTERS *********************************/
uint32_t OSIncLockedCounter(void* pContext)
{
   return 0;
}

uint32_t OSDecLockedCounter(void* pContext)
{
   return 0;
}

/******************************************************************************/

void OSDelayExecution(uint64_t usec)
{
   udelay(usec);
}
#endif /* INCLUDE_OSAPI */

#ifdef INCLUDE_OSAPI_EXT

/************************ Core Kernel Library - START *************************/

/* Abstraction of initialising event object as synchronization or
 * as notification type event. Event gets preset to signaled or 
 * not-signaled state.
 *
 * Every call to OSInitializeEvent must be complemented by calling OSUninitializeEvent, 
 * once event object is not needed any more.
 *
 * pEvent   - Pointer to an event object, for which the caller 
 *            provides the storage
 * type     - NotificationEvent: when event is set to signalled, all threads
 *            waiting for event become eligible for execution.
 *            Event remains signalled until manually set to a non-signalled
 *            state with OSResetEvent or OSClearEvent.
 *            SynchronizationEvent: when event is set to signalled, single thread
 *            waiting for event becomes eligible for execution and event is 
 *            automatically reset to non-signaled state.
 * state    - The event's initial state
 */
void OSInitializeEvent(POSEVENT pEvent, OSEVENT_TYPE type, BOOLEAN state)
{
   memset(pEvent, 0, sizeof(OSEVENT));
   pEvent->pmlock = KernelVzalloc(sizeof(struct mutex));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSInitializeEvent: alloc %d bytes to pEvent(0x%p)->pmlock(0x%p)\n",
          (int)sizeof(struct mutex), pEvent, pEvent->pmlock));
   KernelMutexInit(pEvent->pmlock);
   pEvent->q = KernelVzalloc(sizeof(wait_queue_head_t));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSInitializeEvent: alloc %d bytes to pEvent(0x%p)->q(0x%p)\n",
          (int)sizeof(wait_queue_head_t), pEvent, pEvent->q));
   init_waitqueue_head(pEvent->q);
   atomic_set(&pEvent->count, 0);
   pEvent->type = type;
   pEvent->signalled = state;
   pEvent->timeout = FALSE;
}


/* Abstraction of uninitialising event object.
 * Frees resources dynamically allocated in OSInitializeEvent.
 *
 * pEvent   - Pointer to an event object
 */
void OSUnInitializeEvent(POSEVENT pEvent)
{
   if(pEvent->pmlock != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSUnInitializeEvent: free pmlock(0x%p)\n", pEvent->pmlock));
      KernelVfree(pEvent->pmlock);
      pEvent->pmlock = NULL;
   }
   if(pEvent->q != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSUnInitializeEvent: free q(0x%p)\n", pEvent->q));
      KernelVfree(pEvent->q);
      pEvent->q = NULL;
   }
}

/* Abstraction of data setting in the OSEVENT struct
 * pEvent      - Pointer to the OSEVENT type struct
 * increment   - Whether to increment event set count
 * wait        - Not used
 */
LONG OSSetEvent(POSEVENT pEvent, OSPRIORITY increment, BOOLEAN wait)
{
   LONG ret = FALSE;

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "Set pEvent(0x%p) to signalled\n",
          pEvent));
    
   if(increment)
      atomic_inc(&pEvent->count);

   if(pEvent->signalled)
      ret = TRUE;

   pEvent->signalled  = TRUE;

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "Wake up all on q(0x%p)\n",
          pEvent->q));

   wake_up(pEvent->q);
   
   return ret;
}

/* Abstraction of data clearing in OSEVENT struct
 * pEvent   - Pointer to the OSEVENT struct
 */
LONG OsResetEvent(POSEVENT pEvent)
{
   LONG ret = pEvent->signalled;

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "Clear signalled pEvent(0x%p)\n",
          pEvent));

   atomic_set(&pEvent->count, 0);
   pEvent->signalled = 0;

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "Return previous state(%lu)\n",
          ret));

   return ret;
}

/* Abstraction of data clearing in the OSEVENT struct
 * pEvent   - Pointer to the OSEVENT struct
 */
void OsClearEvent(POSEVENT pEvent)
{
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "Clear signalled pEvent(0x%p)\n",
          pEvent));

   /* PG: added that pEvent is cleared only if pEvent->count is 0
   * This is to make sure that poll does not always clear MultiBuffer event
   * as we can have more than 1 DMAs completed compared how fast app is processing buffers
    * In such case we do not want to wait for MultiBuffer event in poll...
    * Just take the buffer which is ready filled
    */
   if(atomic_read(&pEvent->count) > 0)
      atomic_dec(&pEvent->count);
   if(atomic_read(&pEvent->count) == 0)
      pEvent->signalled = 0;
}

/* Function to wake up a queue after a timeout
 * Event - OSEVENT object to operate on
 */
#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
VOID WaitQueueTimerFn(struct timer_list* pTimer)
{
   struct rgb133_timer_holder* pHolder = from_timer(pHolder, pTimer, timer);
   POSEVENT Event = (POSEVENT)pHolder->data;
#else
VOID WaitQueueTimerFn(unsigned long arg)
{
   POSEVENT Event = (POSEVENT)arg;
#endif
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "WaitQueueTimerFn: Timer fired for Event(0x%p)\n", Event));

   /* Set the appropriate data */
   Event->timeout = TRUE;
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "WaitQueueTimerFn: Wake up all on q(0x%p)\n",
         Event->q));
   wake_up(Event->q);
}

/* Abstraction of wait_event
 * Note: Because we need exclusivity AND a timeout we must create
 *       our own timer to handle the timeout.  There is no timeout
 *       extension for prepare_to_wait.
 * pObject     - OSEVENT object
 * WaitReason  - Driver work or on behalf of a user thread
 * WaitMode    - Kernel or User
 * Alertable   - Interruptible??
 * Timeout     - How long to wait for (in jiffies)
 */
NTSTATUS OSWaitForSingleObject(PVOID pObject, OSWAIT_REASON WaitReason,
                               OSPROCESSOR_MODE WaitMode, BOOLEAN Alertable,
                               PLARGE_INTEGER Timeout)
{
   POSEVENT pEvent = (POSEVENT) pObject;
   RGB133TIMERHOLDER timerHolder;
   struct timer_list* pTimer = &timerHolder.timer;
   unsigned long j = jiffies, units, delay;
   int interruptible = (Alertable ? TASK_INTERRUPTIBLE : TASK_UNINTERRUPTIBLE);
   NTSTATUS status = STATUS_SUCCESS;
         
   /* Start the timer */
   if(Timeout)
   {
      /* Convert the time to jiffies */
      if(Timeout->QuadPart == 0)       /* Don't wait */
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Got zero timeout, return immediately\n"));
         return STATUS_SUCCESS;
      }
      else if(Timeout->QuadPart < 0)   /* relative wait */
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Got a relative timeout(%lld - 0x%llx) for timer(0x%p)\n",
               Timeout->QuadPart, Timeout->QuadPart, pTimer));
#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
         timer_setup(pTimer, WaitQueueTimerFn, 0);
         timerHolder.data = (void*)pEvent;
#else
         init_timer(pTimer);
         pTimer->function = WaitQueueTimerFn;
         pTimer->data = (unsigned long)pEvent;
#endif
         
         /* Calculate delay in jiffies (min 4ms) */
         //units = Timeout->QuadPart * -1;
         units = Timeout->QuadPart;
         units *= -1;
         if(units < 40000)
            units = 40000;
            
         units /= 10000;
         
         delay = ((units * HZ) / 1000);
         pTimer->expires = j + delay;
      }
      else                             /* absolute wait */
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSWaitForSingleObject: Invalid timeout(%lld)\n",
               Timeout->QuadPart));
         return STATUS_INVALID_PARAMETER;
      }
   }
      
   if(Timeout != NULL) /* We've been asked to use a timeout */
   {
      RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Start timer(0x%p)\n", pTimer));
      add_timer(pTimer);
   }
      
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Check pEvent->Signalled(0x%x) and pEvent->Timeout(0x%x)\n",
         pEvent->signalled, pEvent->timeout));

   KernelMutexLock(pEvent->pmlock);

   while(pEvent->signalled != TRUE && pEvent->timeout != TRUE)
   {
      DEFINE_WAIT(wait);
      RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Called DEFINE_WAIT(wait))\n"));
      if(pEvent->type == NotificationEvent)
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: prepare_to_wait\n"));
         prepare_to_wait(pEvent->q, &wait, interruptible);
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: prepare_to_wait_exclusive\n"));
         prepare_to_wait_exclusive(pEvent->q, &wait, interruptible);
      }         
      RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Re-Check pEvent->Signalled(0x%x) and pEvent->Timeout(0x%x)\n",
            pEvent->signalled, pEvent->timeout));
      if(pEvent->signalled != TRUE && pEvent->timeout != TRUE)
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Call schedule to really wait\n"));

         KernelMutexUnlock(pEvent->pmlock);
         schedule();
         KernelMutexLock(pEvent->pmlock);
      }
      RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Call finish_wait(pEvent->q(0x%p), &wait(0x%p))\n",
         pEvent->q, &wait));
      finish_wait(pEvent->q, &wait);
   }

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Got required event or timeout\n"));

   if(pEvent->timeout == TRUE) /* Timer expired */
   {
      RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Got a timeout on timer(0x%p)\n", pTimer));
      pEvent->timeout = FALSE;
      KernelMutexUnlock(pEvent->pmlock);

      return STATUS_TIMEOUT;
   }
   else                       /* Timer didn't expire, cancel if necessary */
   {
      if(Timeout)             /* Only if we were asked to use a timeout */
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: Cancel timer(0x%p)\n", pTimer));
         del_timer(pTimer);
      }

      /* If we were waiting on a synchronisation event, reset the event (clear) */
      if(pEvent->type == SynchronizationEvent)
      {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: synchronization event\n"));
         if(WaitMode != UserMode)
         {
            /* Only clear the event if we're not UserMode,
             * this is so the Linux kernel API can wait for data
             * but not screw up the underlying code
             */
            RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: synchronization event - clear\n"));
            OsClearEvent(pObject);
         }
      }
      else {
         RGB133PRINT((RGB133_LINUX_DBG_IRQ, "OSWaitForSingleObject: notification event - nothing to clear\n"));
      }

      KernelMutexUnlock(pEvent->pmlock);
   }

   return STATUS_SUCCESS;
}

BOOL OsTestSingleObject(PVOID pObject);

/************************* Core Kernel Library - END **************************/

/************************* Executive Library - START **************************/

/* Abstracted initialise resource
 * Resource - Pointer to resource to initialise
 * returns  - STATUS_SUCCESS or STATUS_INSUFFICIENT_RESOURCES
 */
NTSTATUS OSInitializeResourceLite(POSRESOURCE Resource)
{
   Resource->pSpinLock = KernelKmalloc(sizeof(spinlock_t), GFP_ATOMIC);
   if(!Resource->pSpinLock)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSInitializeResourceLite: Failed to allocate %u bytes to pLock for Resource(%p)\n",
            sizeof(spinlock_t), Resource));
      return STATUS_INSUFFICIENT_RESOURCES;
   }
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSInitializeResourceLite: kmalloc %d bytes to Resource(0x%p)->pSpinLock(0x%p)\n",
         sizeof(spinlock_t), Resource, Resource->pSpinLock));

   KernelSpinlockInit(Resource->pSpinLock);

   InitializeListHead(&Resource->Owners);
   InitializeListHead(&Resource->ExclusiveWaiters);
   InitializeListHead(&Resource->SharedWaiters);

   Resource->numActive = 0;
   Resource->numExclusiveWaiters = 0;
   Resource->numSharedWaiters = 0;
   Resource->ownedExclusive = FALSE;

   return STATUS_SUCCESS;
}

/* Abstracted delete resource
 * Resource - Pointer to resource to be deleted
 * returns  - STATUS_SUCCESS if resource was deleted
 */
NTSTATUS OSDeleteResourceLite(POSRESOURCE Resource)
{
   if(Resource->pSpinLock)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSDeleteResourceLite: kfree Resource(0x%p)->pSpinLock(0x%p)\n",
            Resource, Resource->pSpinLock));
      KernelKfree(Resource->pSpinLock);
   }

   RtlZeroMemory((VOID*)Resource, sizeof(OSRESOURCE));

   return STATUS_SUCCESS;
}

/* Abstracted reinitialise resource
 * Resource - Pointer to resource to reinitialise
 * returns  - STATUS_SUCCESS or STATUS_INSUFFICIENT_RESOURCES
 */
NTSTATUS OSReinitializeResourceLite(POSRESOURCE Resource)
{
   OSDeleteResourceLite(Resource);
   return OSInitializeResourceLite(Resource);
}

/* Abstracted acquire resource for exclusive access
 * Resource - Pointer to resource to acquire
 * Wait     - Specifies routine's behaviour whenever resource cannot be acquired immediately
 *            If TRUE, caller is put into a wait state until resource can be acquired
 *            If FALSE, routine immediately returns, regardless of whether resource can be acquired
 * returns  - TRUE if resource is acquired
 *            FALSE if input Wait is FALSE and exclusive access cannot be granted immediately
 */
BOOLEAN OSAcquireResourceExclusiveLite(POSRESOURCE Resource, BOOLEAN Wait)
{
   pid_t currentThread = current->pid;
   BOOLEAN ret = FALSE;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceExclusiveLite(START): Resource(%p)\n", Resource));

   KernelSpinLock(Resource->pSpinLock);

   /* Resource not owned - acquire */
   if(Resource->numActive == 0)
   {
      POWNER_ENTRY pOwner = (POWNER_ENTRY)KernelKmalloc(sizeof(OWNER_ENTRY), GFP_ATOMIC);
      if(!pOwner)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSAcquireResourceExclusiveLite: KernelKmalloc failed for pOwner\n"));

         ret = FALSE;
         goto out;
      }
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceExclusiveLite: kmalloc %d bytes to pOwner(0x%p)\n",
            sizeof(OWNER_ENTRY), pOwner));

      pOwner->threadId = currentThread;
      pOwner->numRecursive = 1;
      /* Insert us into owners' list */
      InsertTailList(&Resource->Owners, &pOwner->Entry);
      Resource->numActive = 1;
      Resource->ownedExclusive = TRUE;

      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceExclusiveLite: acquired Resource(%p)\n", Resource));

      ret = TRUE;
      goto out;
   }
   else /* Resource already acquired - we can only acquire if we already own it exclusively */
   {
      PLIST_ENTRY pEntry = Resource->Owners.Flink;
      POWNER_ENTRY pOwner = CONTAINING_RECORD(pEntry, OWNER_ENTRY, Entry);

      // Recursive acquire
      if(Resource->ownedExclusive == TRUE && pOwner->threadId == currentThread)
      {
         pOwner->numRecursive++;
         Resource->numActive++;

         RGB133PRINT((RGB133_LINUX_DBG_LOG, "OSAcquireResourceExclusiveLite: acquired recursively Resource(%p)\n", Resource));

         ret = TRUE;
         goto out;
      }

      /* Resource not available for us at the moment - wait for it or return */
      if(Wait)
      {
         PWAIT_ENTRY pWaiter = NULL;

         /* Allocate a waiter, insert it into exclusive waiters list and go to sleep */
         /* Exclusive waiters list is FIFO and oldest entry is closest to head */
         pWaiter = (PWAIT_ENTRY)KernelKmalloc(sizeof(WAIT_ENTRY), GFP_ATOMIC);
         if(!pWaiter)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSAcquireResourceExclusiveLite: KernelKmalloc failed for exclusive waiter\n"));
            ret = FALSE;
            goto out;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceExclusiveLite: kmalloc %d bytes to pWaiter(0x%p)\n",
               sizeof(WAIT_ENTRY), pWaiter));

         pWaiter->pSem = KernelKmalloc(sizeof(struct semaphore), GFP_ATOMIC);
         if(!pWaiter->pSem)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSAcquireResourceExclusiveLite: KernelKmalloc failed for exclusive waiter sem\n"));
            KernelKfree(pWaiter);
            ret = FALSE;
            goto out;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceExclusiveLite: kmalloc %d bytes to pSem(0x%p)\n",
               sizeof(struct semaphore), pWaiter->pSem));

         pWaiter->threadId = currentThread;
         sema_init(pWaiter->pSem, 0);
         InsertTailList(&Resource->ExclusiveWaiters, &pWaiter->Entry);
         Resource->numExclusiveWaiters++;
         KernelSpinUnlock(Resource->pSpinLock);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceExclusiveLite: wait for Resource(%p) - sleep now\n",
                Resource));

         down(pWaiter->pSem);

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceExclusiveLite: kfree pSem(0x%p)\n",
                pWaiter->pSem));
         KernelKfree(pWaiter->pSem);

         ret = TRUE;
         goto acquired;
      }
      else
      {
         /* Resource not available and we were told not to wait */
         ret = FALSE;
         goto out;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceExclusiveLite(END): Resource(%p) ret(%d)\n",
          Resource, ret));

out:
   KernelSpinUnlock(Resource->pSpinLock);

acquired:
   return ret;
}

/* Abstracted acquire resource for shared access
 * Resource - Pointer to resource to acquire
 * Wait     - Specifies routine's behavior whenever resource cannot be acquired immediately
 *            If TRUE, caller is put into a wait state until resource can be acquired
 *            If FALSE, routine immediately returns, regardless of whether resource can be acquired
 * returns  - TRUE if resource is acquired
 *            FALSE if argument Wait is FALSE and exclusive access cannot be granted immediately
 */
BOOLEAN OSAcquireResourceSharedLite(POSRESOURCE Resource, BOOLEAN Wait)
{
   POWNER_ENTRY pOwner = NULL;
   PLIST_ENTRY pEntry = NULL;
   pid_t currentId = current->pid;
   BOOLEAN ownedByUs = FALSE;
   BOOLEAN exclWaiters = FALSE;
   BOOLEAN ret = FALSE;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceSharedLite(START): Resource(%p)\n", Resource));

   KernelSpinLock(Resource->pSpinLock);

   if(Resource->numActive > 0)
   {
      /* Check if we own resource */
      pEntry = Resource->Owners.Flink;
      while(pEntry != (&Resource->Owners) && ownedByUs == FALSE)
      {
         pOwner = CONTAINING_RECORD(pEntry, OWNER_ENTRY, Entry);
         if(pOwner->threadId == currentId)
            ownedByUs = TRUE;
         else
            pEntry = pEntry->Flink;
      }

      /* Check for exclusive waiters */
      if(IsListEmpty(&Resource->ExclusiveWaiters))
         exclWaiters = FALSE;
      else
         exclWaiters = TRUE;
   }

   /* Terms for immediate acquire for shared access */
   if((Resource->numActive == 0) ||
      (Resource->numActive > 0 && !Resource->ownedExclusive && !ownedByUs && !exclWaiters))
   {
      pOwner = (POWNER_ENTRY)KernelKmalloc(sizeof(OWNER_ENTRY), GFP_ATOMIC);
      if(!pOwner)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSAcquireResourceSharedLite: KernelKmalloc failed for pOwner(1)\n"));

         ret = FALSE;
         goto unlock_out;
      }
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceSharedLite: kmalloc %d bytes to pOwner(0x%p)\n",
            sizeof(OWNER_ENTRY), pOwner));

      pOwner->threadId = currentId;
      pOwner->numRecursive = 1;
      /* Insert us into owners' list */
      InsertTailList(&Resource->Owners, &pOwner->Entry);
      Resource->numActive++;
      Resource->ownedExclusive = FALSE;

      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceSharedLite: acquired Resource(%p)\n", Resource));

      ret = TRUE;

      goto unlock_out;
   }
   else if(!Resource->ownedExclusive && ownedByUs) /* Terms for immediate recursive acquire */
   {
      pOwner->numRecursive++;

      RGB133PRINT((RGB133_LINUX_DBG_LOG, "OSAcquireResourceSharedLite: acquired recursively Resource(%p)\n", Resource));

      ret = TRUE;
      goto unlock_out;
   }
   else if(Resource->ownedExclusive ||
           (!Resource->ownedExclusive && !ownedByUs && exclWaiters)) /* Else we wait or return false */
   {
      if(Wait)
      {
         PWAIT_ENTRY pWaiter = NULL;

         /* Allocate a waiter, insert us into shared waiters list and go to sleep */
         /* Shared waiters list is FIFO and oldest entry is closest to head */
         pWaiter = (PWAIT_ENTRY)KernelKmalloc(sizeof(WAIT_ENTRY), GFP_ATOMIC);
         if(!pWaiter)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSAcquireResourceSharedLite: KernelKmalloc failed for shared waiter\n"));
            ret = FALSE;
            goto unlock_out;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceSharedLite: kmalloc %d bytes to pWaiter(0x%p)\n",
               sizeof(WAIT_ENTRY), pWaiter));

         pWaiter->pSem = KernelKmalloc(sizeof(struct semaphore), GFP_ATOMIC);
         if(!pWaiter->pSem)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSAcquireResourceSharedLite: KernelKmalloc failed for exclusive waiter sem\n"));
            KernelKfree(pWaiter);
            ret = FALSE;
            goto unlock_out;
         }
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceSharedLite: kmalloc %d bytes to pSem(0x%p)\n",
               sizeof(struct semaphore), pWaiter->pSem));

         pWaiter->threadId = currentId;
         sema_init(pWaiter->pSem, 0);
         InsertTailList(&Resource->SharedWaiters, &pWaiter->Entry);
         Resource->numSharedWaiters++;
         KernelSpinUnlock(Resource->pSpinLock);

         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceSharedLite: wait to acquire Resource(%p) - sleep\n",
                Resource));

         down(pWaiter->pSem);

         RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSAcquireResourceSharedLite: kfree pSem(0x%p)\n",
                pWaiter->pSem));
         KernelKfree(pWaiter->pSem);

         ret = TRUE;
         goto acquired;
      }
      else
      {
         /* Resource not available and we were told not to wait */
         ret = FALSE;
         goto unlock_out;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSAcquireResourceSharedLite(END): Resource(%p) ret(%d)\n",
          Resource, ret));

unlock_out:
   KernelSpinUnlock(Resource->pSpinLock);

acquired:
   return ret;
}

/* Abstracted release acquired resource
 * Resource - Pointer to resource to release
 */
VOID OSReleaseResourceLite(POSRESOURCE Resource)
{
   PLIST_ENTRY pEntry = NULL;
   POWNER_ENTRY pOwner = NULL;
   PWAIT_ENTRY pWaiter = NULL;
   pid_t currentThread = current->pid;
   BOOLEAN ownedByUs = FALSE;
   BOOLEAN ret = FALSE;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSReleaseResourceLite(START): Resource(%p)\n", Resource));

   KernelSpinLock(Resource->pSpinLock);

   /* part 1: Sanity check - we must own resource to do things here */
   pEntry = Resource->Owners.Flink;
   while(pEntry != (&Resource->Owners) && ownedByUs == FALSE)
   {
      pOwner = CONTAINING_RECORD(pEntry, OWNER_ENTRY, Entry);
      if(pOwner->threadId == currentThread)
         ownedByUs = TRUE;
      else
         pEntry = pEntry->Flink;
   }

   if(!ownedByUs)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "OSReleaseResourceLite: Attempting to release a not acquired Resource(%p)\n",
             Resource));
      goto out;
   }

   /* part 2: Deletion from resource owners list
    * Decrement recursive counter if we acquired lock more than once
    * Otherwise if we are single owner, delete us from owners' list */
   if(pOwner->numRecursive > 1)
   {
      pOwner->numRecursive--;
   }
   else if(pOwner->numRecursive == 1)
   {
      RemoveEntryList(pEntry);

      RGB133PRINT((RGB133_LINUX_DBG_MEM, "OSReleaseResourceLite: kfree pOwner(0x%p)\n", pOwner));
      KernelKfree(pOwner);
      if(Resource->ownedExclusive)
         Resource->ownedExclusive = FALSE;
   }
   Resource->numActive--;

   /* part 3: Waking up waiting threads
    * Only wake up if currently resource is not owned
    * Exclusive waiters get priority
    * Exclusive waiters are FIFO and oldest is closest to head in list */
   if(Resource->numActive > 0)
      goto out;

   {
      POWNER_ENTRY pNewOwner = NULL;

      if(!IsListEmpty(&Resource->ExclusiveWaiters))
      {
         // Remove entry from exclusive waiters list and move it to owners
         pEntry = Resource->ExclusiveWaiters.Flink;
         RemoveEntryList(pEntry);
         pWaiter = CONTAINING_RECORD(pEntry, WAIT_ENTRY, Entry);
         pNewOwner = (POWNER_ENTRY)pWaiter;
         pNewOwner->numRecursive = 1;
         InsertTailList(&Resource->Owners, pEntry);

         Resource->ownedExclusive = TRUE;
         Resource->numExclusiveWaiters--;
         Resource->numActive = 1;
         up(pWaiter->pSem);
      }
      else if(!IsListEmpty(&Resource->SharedWaiters))
      {
         // move all waiters to owners
         while(!IsListEmpty(&Resource->SharedWaiters))
         {
            pEntry = RemoveTailList(&Resource->SharedWaiters);
            InsertHeadList(&Resource->Owners, pEntry);
         }
         Resource->ownedExclusive = FALSE;
         Resource->numActive = Resource->numSharedWaiters;
         Resource->numSharedWaiters = 0;

         // wake up all shared waiters; they are owners now
         pEntry = Resource->Owners.Flink;
         while(pEntry != (&Resource->Owners))
         {
            pWaiter = CONTAINING_RECORD(pEntry, WAIT_ENTRY, Entry);
            pNewOwner = (POWNER_ENTRY)pWaiter;
            pNewOwner->numRecursive = 1;
            up(pWaiter->pSem);
            pEntry = pEntry->Flink;
         }
      }
   }

out:
   KernelSpinUnlock(Resource->pSpinLock);

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "OSReleaseResourceLite(END): Resource(%p)\n", Resource));
}

/************************** Executive Library - END ***************************/

#endif /* INCLUDE_OSAPI_EXT */
