/*
 * rgb133kernel.c
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

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/firmware.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/sched.h>
#if defined (RGB133_CONFIG_HAVE_KTIME_GET_REAL_TS64) || defined (RGB133_CONFIG_HAVE_KTIME_GET_TS64)
#include <linux/time64.h>
#include <linux/timekeeping.h>
#endif

#include <asm/uaccess.h>

#include "rgb133config.h"
#include "rgb133.h"
#include "rgb133deviceapi.h"
#include "rgb133capapi.h"
#include "rgb133kernel.h"
#include "rgb133debug.h"
#include "OSAPI_TYPES.h"
#include "rgb_windows_types.h"

/* Workaround for inheriting kernel symbols into closed-source archives at build time
 * and then trying to load them into a different kernel which does not define the symbols */
#ifdef RGB133_CONFIG_RT_AND_UNDEFINED_KERNEL_STACK
#ifndef kernel_stack
unsigned int kernel_stack;
#endif
#endif

bool KernelUseWorkLookup(void)
{
#ifndef RGB133_CONFIG_HAVE_NEW_WORK
   return FALSE;
#else
   return TRUE;
#endif
}

void* KernelVzalloc(unsigned int size)
{
   void* ptr = vmalloc(size);
   if(ptr)
      memset(ptr, 0, size);
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelVzalloc: failed to alloc %d bytes\n",
            size));
   }
   return ptr;
}

void KernelVfree(void* ptr)
{
   vfree(ptr);
}

void* KernelKmalloc(size_t size, int flags)
{
   void* ptr = kmalloc(size, flags);
   if(ptr)
      memset(ptr, 0, size);
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelKmalloc: failed to alloc %d bytes\n",
            size));
   }
   return ptr;
}

void KernelKfree(const void* ptr)
{
   kfree(ptr);
}

void KernelInitWork(void* arg1, void* arg2, void* arg3)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
   //RGB133PRINT((RGB133_LINUX_DBG_TODO, "KernelInitWork: INIT_WORK(0x%p) - data(0x%x)\n", arg1, arg3));
   INIT_WORK((struct work_struct*)arg1, arg2, arg3);
#else
   //RGB133PRINT((RGB133_LINUX_DBG_TODO, "KernelInitWork: INIT_WORK(0x%p)\n", arg1));
   INIT_WORK((struct work_struct*)arg1, arg2);
#endif /* RGB133PRE */
}

void KernelPrepareWork(void* arg1, void* arg2, void* arg3)
{
#ifdef RGB133_CONFIG_HAVE_MACRO_PREPARE_WORK
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
   //RGB133PRINT((RGB133_LINUX_DBG_TODO, "KernelPrepareWork: PREPARE_WORK(0x%p) - data(0x%x)\n", arg1, arg3));
   PREPARE_WORK((struct work_struct*)arg1, arg2, arg3);
#else
   //RGB133PRINT((RGB133_LINUX_DBG_TODO, "KernelPrepareWork: PREPARE_WORK(0x%p)\n", arg1));
   PREPARE_WORK((struct work_struct*)arg1, arg2);
#endif /* RGB133PRE */
#else /* PREPARE_WORK does not exist */
   struct work_struct* ws = (struct work_struct*)arg1;
   ws->func = arg2;
#endif /* PREPARE_WORK */
}

int KernelLoadFirmware(const PFWENTRYAPI* pfw_entry, char* fw_name, PDEAPI _pDE)
{
   const struct firmware** fw_entry = (const struct firmware**)pfw_entry;
   struct rgb133_dev* rgb133 = DeviceGetRGB133(_pDE);

   if(rgb133)
      return request_firmware(fw_entry, fw_name, (struct device*)KernelGetDriverDevice(rgb133));
   else
      return 0;
}

int KernelGetFwSize(PFWENTRYAPI fw_entry)
{
   return ((struct firmware*)fw_entry)->size;
}

char* KernelGetFwData(PFWENTRYAPI fw_entry)
{
   return (char*)((struct firmware*)fw_entry)->data;
}

void KernelReleaseFw(PFWENTRYAPI fw_entry)
{
   release_firmware((struct firmware*)fw_entry);
}

PDRIVERDEVAPI KernelGetDriverDevice(struct rgb133_dev* rgb133)
{
   return (PDRIVERDEVAPI)&rgb133->core.pci->dev;
}

void KernelAtomicSet(atomic_t* pAtomic, int value)
{
   atomic_set(pAtomic, value);
}

int KernelAtomicRead(const atomic_t* pAtomic)
{
   return atomic_read(pAtomic);
}

void KernelDoGettimeofday(unsigned long* pSec, unsigned long* pUsec)
{
   KernelGetCurrentTime(NULL, pSec, pUsec);
}

void KernelGetCurrentTime(PTIMEVALAPI _pTv, unsigned long* pSec, unsigned long* pUsec)
{
   sTimeval tv;
#if defined (RGB133_CONFIG_HAVE_KTIME_GET_REAL_TS64) || defined (RGB133_CONFIG_HAVE_KTIME_GET_TS64)
   struct timespec64 tspec;
#ifdef RGB133_CONFIG_HAVE_KTIME_GET_REAL_TS64
   ktime_get_real_ts64(&tspec);
#else /* RGB133_CONFIG_HAVE_KTIME_GET_TS64 */
   ktime_get_ts64(&tspec);
#endif
   tv.tv_sec = tspec.tv_sec;
   tv.tv_usec = tspec.tv_nsec / 1000;
#else /* !(RGB133_CONFIG_HAVE_KTIME_GET_REAL_TS64 || RGB133_CONFIG_HAVE_KTIME_GET_TS64) */
   do_gettimeofday(&tv);
#endif

   if(pSec && pUsec) {
      *pSec = tv.tv_sec;
      *pUsec = tv.tv_usec;
   }
   if(_pTv)
      memcpy((sTimeval*)_pTv, &tv, sizeof(sTimeval));
}

int KernelMemcmp(const void* s1, const void* s2, size_t n)
{
   return memcmp(s1, s2, n);
}

int KernelCopyFromUser(void* to, const __user void* from, unsigned long count)
{
   return copy_from_user(to, from, count);
}

int KernelCopyToUser(__user void* to, void* from, unsigned long count)
{
   return copy_to_user(to, from, count);
}

void KernelMutexInit(PMUTEXAPI pLock)
{
   mutex_init((struct mutex*)pLock);
}

void KernelMutexLock(PMUTEXAPI pLock)
{
   mutex_lock((struct mutex*)pLock);
}

void KernelMutexUnlock(PMUTEXAPI pLock)
{
   mutex_unlock((struct mutex*)pLock);
}

void KernelSpinlockInit(PSPINLOCKAPI pLock)
{
   spin_lock_init((spinlock_t*)pLock);
}

void KernelSpinLock(PSPINLOCKAPI pLock)
{
   spin_lock((spinlock_t*)pLock);
}

void KernelSpinUnlock(PSPINLOCKAPI pLock)
{
   spin_unlock((spinlock_t*)pLock);
}

void KernelPreemptEnable( )
{
   preempt_enable();
}

void KernelPreemptDisable( )
{
   preempt_disable();
}

void KernelMemset(void* ptr, int value, unsigned int size)
{
   if(ptr)
      memset(ptr, value, size);
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "KernelMemset: argument is a NULL pointer\n"));
   }
}

void KernelAddTimer(PVOID _pTimer)
{
   struct timer_list* pTimer = (struct timer_list*)_pTimer;

   add_timer(pTimer);
}

int KernelDelTimer(PVOID _pTimer)
{
   struct timer_list* pTimer = (struct timer_list*)_pTimer;

   return del_timer(pTimer);
}

BOOL KernelInitTimer(PVOID _pTimer, PVOID start_routine, PVOID data, unsigned int flags)
{
   struct timer_list* pTimer = (struct timer_list*)_pTimer;

#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
   timer_setup(pTimer, (void(*)(struct timer_list*))start_routine, flags);
#else
   init_timer(pTimer);
   pTimer->data = (unsigned long)data;
   pTimer->function = ((void(*)(unsigned long))start_routine);
#endif
   pTimer->expires = jiffies + 1;

   return TRUE;
}

void KernelSetTimerExpires(PVOID _pTimer, BOOL use_jiffies, unsigned long value_jffs)
{
   struct timer_list* pTimer = (struct timer_list*)_pTimer;

   if(use_jiffies)
      pTimer->expires = jiffies + value_jffs;
   else
      pTimer->expires = value_jffs;
}

PMODULEAPI KernelGetModule( )
{
   return (PMODULEAPI)THIS_MODULE;
}

int KernelGetGFP_DMAFlag( )
{
   return GFP_DMA;
}

unsigned long KernelGetPAGE_OFFSET( )
{
   return PAGE_OFFSET;
}

unsigned int KernelGetDeviceNodeNr(void* _dev, int channel)
{
   struct rgb133_dev* dev = (struct rgb133_dev*)_dev;
   unsigned int node = -1;

   /* If inputs are exposed, we create a video_device per channel so we can dereference device info map by channel number
    * Otherwise, the device we are looking for is always at index 0
    */
   if(dev->devices <= 1)
      channel = 0;

#ifdef RGB133_CONFIG_HAVE_NEW_VDEV_NUM
   node = dev->pVfd[channel]->num;
#else
   node = dev->pVfd[channel]->minor;
#endif

   return node;
}

void KernelGetTimestamp(struct rgb133_handle* h, int index, eTimestamp ts, unsigned long* pSec, unsigned long* pUsec)
{
   struct rgb133_unified_buffer* pDmaBuf = NULL;

   if(!h)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelGetTimestamp: Invalid h(0x%p)\n", h));
      return;
   }

   pDmaBuf = h->buffers[index];
   if(!pDmaBuf)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelGetTimestamp: Invalid h(%p)->buffers[%d](%p)\n", h, index, h->buffers[index]));
      return;
   }

   switch(pDmaBuf->notify)
   {
      case RGB133_NOTIFY_FRAME_CAPTURED:
         switch(ts)
         {
            case RGB133_LINUX_TIMESTAMP_ACQ:
               if(pDmaBuf->ACQ.sec == 0 && pDmaBuf->ACQ.usec == 0)
               {
                  KernelDoGettimeofday(pSec, pUsec);
                  if(*pUsec > 1000000)
                  {
                     (*pSec)++;
                     *pUsec -= 1000000;
                  }
                  RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KernelGetTimestamp: No valid timestamp buf[%d]. Stamp frame(%ld) with (%u.%06u)\n",
                        index, h->frame.seq, *pSec, *pUsec));
               }
               else
               {
                  *pSec = pDmaBuf->ACQ.sec;
                  *pUsec = pDmaBuf->ACQ.usec;
                  RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KernelGetTimestamp: buf[%d] frame(%ld) ACQ stamp (%u.%06u)\n",
                        index, h->frame.seq, *pSec, *pUsec));
               }
               break;
            case RGB133_LINUX_TIMESTAMP_Q:
               *pSec = pDmaBuf->Q.sec;
               *pUsec = pDmaBuf->Q.usec;
               break;
            case RGB133_LINUX_TIMESTAMP_DMA:
               *pSec = pDmaBuf->DMA.sec;
               *pUsec = pDmaBuf->DMA.usec;
               break;
            case RGB133_LINUX_TIMESTAMP_DQ:
               *pSec = pDmaBuf->DQ.sec;
               *pUsec = pDmaBuf->DQ.usec;
               break;
            default:
               RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelGetTimestamp: Invalid timestamp id(%d)\n", (int)ts));
         }
         break;
      case RGB133_NOTIFY_NO_SIGNAL:
      case RGB133_NOTIFY_DMA_OVERFLOW:
      default:
         KernelDoGettimeofday(pSec, pUsec);
         if(*pUsec > 1000000)
         {
            (*pSec)++;
            *pUsec -= 1000000;
         }
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "KernelGetTimestamp: buf[%d] notification(%d) - stamp frame(%ld) with (%u.%06u)\n",
               index, h->frame.seq, *pSec, *pUsec));
         break;
   }
}

void KernelSetTimestamp(struct rgb133_handle* h, int index, eTimestamp ts, unsigned long sec, unsigned long usec)
{
   struct rgb133_unified_buffer* pDmaBuf = NULL;

   if(!h)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelSetTimestamp: Invalid h(0x%p)\n", h));
      return;
   }

   if(index < 0 || index > (h->numbuffers - 1))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelSetTimestamp: Invalid buffer index(%d)\n", index));
      return;
   }

   pDmaBuf = h->buffers[index];
   if(!pDmaBuf)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelSetTimestamp: Invalid pDmaBuf(%p)\n", pDmaBuf));
      return;
   }

   switch(ts)
   {
      case RGB133_LINUX_TIMESTAMP_ACQ:
         pDmaBuf->ACQ.sec = sec;
         pDmaBuf->ACQ.usec = usec;
         break;
      case RGB133_LINUX_TIMESTAMP_Q:
         pDmaBuf->Q.sec = sec;
         pDmaBuf->Q.usec = usec;
         break;
      case RGB133_LINUX_TIMESTAMP_DMA:
         pDmaBuf->DMA.sec = sec;
         pDmaBuf->DMA.usec = usec;
         break;
      case RGB133_LINUX_TIMESTAMP_DQ:
         pDmaBuf->DQ.sec = sec;
         pDmaBuf->DQ.usec = usec;
         break;
      default:
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "KernelSetTimestamp: Invalid timestamp id(%d)\n", (int)ts));
   }
}

HANDLE KernelGetProcessId( )
{
   return (HANDLE)current->pid;
}

BOOLEAN KernelGetEventState(POSEVENT pEvent)
{
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "KernelGetEventState: Returning pEvent(%p)->signalled(%d)\n",
         pEvent, pEvent->signalled));

   return pEvent->signalled;
}

int KernelPrint(const char *fmt, ...)
{
   va_list args;
   int r;

   va_start(args, fmt);
   r = vprintk(fmt, args);
   va_end(args);

   return r;
}

