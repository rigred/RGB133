/*
 * rgb133vidbuf.c
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
#include <linux/mutex.h>

#include "rgb133config.h"
#include "rgb133.h"
#include "rgb133kernel.h"
#include "rgb133mapping.h"
#include "rgb133timestamp.h"
#include "rgb133vidbuf.h"
#include "rgb133win.h"
#include "rgb133deviceapi.h"

struct rgb133_vm_privdata {
   struct rgb133_handle * h;
   struct rgb133_unified_buffer * b;
   int count;
};

static DEFINE_MUTEX(vm_area_lock);

void rgb133_vm_open(struct vm_area_struct* vma)
{
   struct rgb133_vm_privdata * pd = 0;
   int ret = 0;

   if(vma == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_open: vma NULL! return\n"));
      return ;
   }
   if(vma->vm_private_data == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_open: pd NULL! return\n"));
      return ;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_vm_open: mutex_lock_interruptible(%p)\n", &vm_area_lock));
   ret = mutex_lock_interruptible(&vm_area_lock);
   if (ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_vm_open: mutex_lock_interruptible(%d) interrupted by a signal ret(%d)\n",
            vm_area_lock, ret));
      return ;
   }

   if(vma->vm_private_data == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_open: pd NULL! mutex_unlock(%p) and return\n", &vm_area_lock));
      mutex_unlock(&vm_area_lock);
      return ;
   }

   pd = (struct rgb133_vm_privdata*)vma->vm_private_data;
   if( (pd->h == 0) || (pd->b == 0) )
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_open: h(0x%p), pd->b(0x%p), mutex_unlock(%p) and return\n",
            pd->h, pd->b, &vm_area_lock));
      mutex_unlock(&vm_area_lock);
      return ;
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_vm_open: For pd(0x%p)->h(0x%p)->b(0x%p)->index(%d) open vma(0x%p) - start(0x%lx), size(%lu)\n",
                pd, pd->h, pd->b, pd->b->index, vma, vma->vm_start, vma->vm_end - vma->vm_start));

   /* Increment the usage counter, so when know when
    * we can free memory in the vm_close function */
   pd->count++;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_vm_open: mutex_unlock(%p)\n", &vm_area_lock));
   mutex_unlock(&vm_area_lock);
}

void rgb133_vm_close(struct vm_area_struct* vma)
{
   struct rgb133_vm_privdata* pd = 0;
   int ret = 0;

   if(vma == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_close: vma NULL! return\n"));
      return ;
   }
   if(vma->vm_private_data == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_close: pd NULL! return\n"));
      return ;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_vm_close: mutex_lock_interruptible(%p)\n", &vm_area_lock));
   ret = mutex_lock_interruptible(&vm_area_lock);
   if (ret < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_vm_close: mutex_lock_interruptible(%d) interrupted by a signal ret(%d)\n",
            vm_area_lock, ret));
      return ;
   }

   if(vma->vm_private_data == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_close: pd NULL! mutex_unlock(%p) and return\n", &vm_area_lock));
      mutex_unlock(&vm_area_lock);
      return ;
   }

   pd = (struct rgb133_vm_privdata*)vma->vm_private_data;
   if( (pd->h == 0) || (pd->b == 0))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_close: pd->h(0x%p), pd->b(0x%p), mutex_unlock(%p) and return\n",
            pd->h, pd->b, &vm_area_lock));
      mutex_unlock(&vm_area_lock);
      return ;
   }

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_vm_close: close vma(0x%p) - start(0x%lx), size(%lu)\n",
                vma, vma->vm_start, vma->vm_end - vma->vm_start));
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_vm_close: on pd(0x%p)->h(0x%p), b(0x%p)->pMemory(0x%p) for bsize(%d) bytes\n",
                pd, pd->h, pd->b->pMemory, pd->b->pMemory, pd->b->bsize));

   /* Decrement the usage count, if zero, free memory */
   pd->count--;
   if(pd->count == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_vm_close: free pd(0x%p)\n", pd));
      KernelVfree(pd);
      vma->vm_private_data = 0;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_vm_close: mutex_unlock(%p)\n\n", &vm_area_lock));
   mutex_unlock(&vm_area_lock);
}

#ifdef RGB133_CONFIG_HAVE_VMA_FAULT
#ifdef RGB133_CONFIG_HAVE_PAGE_FAULT_HANDLER_RETURN_VM_FAULT_T
static vm_fault_t rgb133_vm_fault(struct vm_fault *vmf)
{
   struct vm_area_struct* vma = vmf->vma;
#else /* !RGB133_CONFIG_HAVE_PAGE_FAULT_HANDLER_RETURN_VM_FAULT_T */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0) /* kernel >= 4.11.0 */
static int rgb133_vm_fault(struct vm_fault* vmf)
{
   struct vm_area_struct* vma = vmf->vma;
#else /* !(kernel >= 4.11.0) */
static int rgb133_vm_fault(struct vm_area_struct* vma, struct vm_fault* vmf)
{
#endif /* kernel >= 4.11.0 */
#endif /* RGB133_CONFIG_HAVE_PAGE_FAULT_HANDLER_RETURN_VM_FAULT_T */
   struct page* page = NULL;
   struct rgb133_vm_privdata* pd = NULL;
   unsigned long address = 0;
   unsigned long offset = 0;

   if(vma == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_vm_fault: vma is NULL, vmf(0x%p)\n", vmf));
      return VM_FAULT_ERROR;
   }

#if defined(RGB133_CONFIG_HAVE_PAGE_FAULT_HANDLER_RETURN_VM_FAULT_T) || (LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0))
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0) /* kernel >= 4.10.0 */
   address = (unsigned long)vmf->address;
#else
   address = (unsigned long)vmf->virtual_address;
#endif

   offset = address - vma->vm_start;
   RGB133PRINT((RGB133_LINUX_DBG_FAULT, "rgb133_vm_fault: fault @ 0x%lx (vma: 0x%lx - 0x%lx) offset(0x%lx)\n",
      address, vma->vm_start, vma->vm_end, vma->vm_pgoff));
   pd = vma->vm_private_data;
   page = vmalloc_to_page(pd->b->pMemory + offset);
   RGB133PRINT((RGB133_LINUX_DBG_FAULT, "rgb133_vm_fault: mapping(0x%lx) to fault(0x%p) @ page(0x%p)\n",
      ((unsigned long)pd->b->pMemory + offset), address, page));
   get_page(page);

   vmf->page = page;

   return 0;
}
#else /* !RGB133_CONFIG_HAVE_VMA_FAULT */
static struct page* rgb133_vm_nopage(struct vm_area_struct* vma, unsigned long address, int* type)
{
   struct page* page;
   struct rgb133_vm_privdata* pd = vma->vm_private_data;
   struct rgb133_unified_buffer* b = pd->b;
   unsigned long offset = address - vma->vm_start + (vma->vm_pgoff << PAGE_SHIFT);

   RGB133PRINT((RGB133_LINUX_DBG_FAULT, "rgb133_vm_nopage: fault @ 0x%lx (vma: 0x%lx - 0x%lx) offset(0x%lx)\n",
      address, vma->vm_start, vma->vm_end, vma->vm_pgoff));

   RGB133PRINT((RGB133_LINUX_DBG_FAULT, "rgb133_vm_nopage: mapping(0x%lx) to fault(0x%lx)\n",
      ((unsigned long)b->pMemory + offset), address));
   page = vmalloc_to_page(b->pMemory + offset);
   get_page(page);

   if(type)
      *type = VM_FAULT_MINOR;

   return page;
}
#endif

static struct vm_operations_struct rgb133_buf_vm_ops = {
   .open  = rgb133_vm_open,
   .close = rgb133_vm_close,
#ifdef RGB133_CONFIG_HAVE_VMA_FAULT
   .fault  = rgb133_vm_fault,
#else
   .nopage  = rgb133_vm_nopage,
#endif
};

/* Enter handle critical section */
void rgb133_enter_critical_section(struct rgb133_handle* h)
{
   int rc = 0;
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_enter_critical_section: Entering Critical section for %p\n", h));
   rc = down_interruptible(h->pSem);
}

/* Exit handle critical section */
void rgb133_exit_critical_section(struct rgb133_handle* h)
{
   up(h->pSem);
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_exit_critical_section: Exiting Critical section for %p\n", h));
}

/* Buffer queue spinlock protection */
void rgb133_acquire_spinlock(struct rgb133_handle* h, char * caller, int lock_action)
{
   if(lock_action == LOCK_SPINLOCK)
   {
      spin_lock_bh(h->pSpinlock);
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_acquire_spinlock: Acquired spinlock for handle %p on behalf of %s\n",
            h, caller));
   }
}

void rgb133_release_spinlock(struct rgb133_handle* h, char * caller, int unlock_action)
{
   if(unlock_action == UNLOCK_SPINLOCK)
   {
      spin_unlock_bh(h->pSpinlock);
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_release_spinlock: Releasing spinlock for handle %p on behalf of %s\n",
            h, caller));
   }
}

/* Create an internal buffer */
void* rgb133_buffer_alloc(struct rgb133_handle *h)
{
   struct rgb133_unified_buffer* buffer = 0;

   buffer = (struct rgb133_unified_buffer*)KernelVzalloc(sizeof(struct rgb133_unified_buffer));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_buffer_alloc: alloc %d byes to buffer(0x%p)\n",
         sizeof(struct rgb133_unified_buffer), buffer));
   if (!buffer)
      return NULL;
   
   buffer->h = h;
   return buffer;
}

/* Free an internal buffer */
void rgb133_buffer_free(struct rgb133_unified_buffer** ppBuf)
{
   if(ppBuf && *ppBuf)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_buffer_free: free ppBuf(0x%p *0x%p)\n", ppBuf, *ppBuf));
      KernelVfree(*ppBuf);
      *ppBuf = 0;
   }
   else
   {
      if(ppBuf)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_buffer_free: oops ppBuf(0x%p *0x%p)\n",
            ppBuf, *ppBuf));
      }
      else
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_buffer_free: oops ppBuf(0x%p)\n",
            ppBuf));
      }
   }
}

void rgb133_q_uninit(struct rgb133_handle* h)
{
   if(h)
   {
      rgb133_set_streaming(h, RGB133_STRM_NOT_STARTED);
      rgb133_set_reading(h, 0);
      rgb133_set_mapped(h, 0);
   }
}

int rgb133_read_buffer_setup(struct rgb133_unified_buffer** ppBuf, char __user* data, unsigned int size,
      OSPROCESSOR_MODE AccessMode)
{
   PIRP pIrp = NULL;
   void* pWinSGL = 0;
   int plane = 0;
   int sizePlane = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_read_buffer_setup: START\n"));

   if(ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_setup: invalid buffer ptr ppBuf\n"));
      return -EINVAL;
   }
   else if((*ppBuf) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_setup: invalid buffer (*ppBuf)\n"));
      return -EINVAL;
   }

   (*ppBuf)->index = 0;
   (*ppBuf)->mode = AccessMode;

   for(plane=0;plane<(*ppBuf)->h->sCapture.planes;plane++)
   {
      /* for NV12 and YV12, size of plane Y is 2/3 the size of all data */
      if (((*ppBuf)->h->sCapture.pixelformat == V4L2_PIX_FMT_NV12 || (*ppBuf)->h->sCapture.pixelformat == V4L2_PIX_FMT_YVU420)
           && plane == 0)
         sizePlane = 2 * size / 3;
      else
      {
         /* Intuitively, this should be 1/3 * size
          * However, to work correctly the core driver expects the length of the second plane to be set to the entire buffer length
          * (for detail, look into RGBWaitForDataK in the core or into SetUpIoctlIn)
          */
         sizePlane = size;
      }
      (*ppBuf)->pMdlsPlanar[plane] = IoAllocateMdl(data, sizePlane, FALSE, FALSE, NULL);
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_buffer_setup: allocate pMdlsPlanar[%d](0x%p) for buffer(0x%p - size(%d))\n",
            plane, (*ppBuf)->pMdlsPlanar[plane], data, sizePlane));

      if((*ppBuf)->pMdlsPlanar[plane] == NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_setup: failed to allocate pMdlsPlanar[%d] with data(0x%p - %d)\n",
        		 plane, data, sizePlane));
         return -ENOMEM;
      }

      rgb133_init_kernel_dma(&(*ppBuf)->dma_info[plane], &(*ppBuf)->kernel_dma[plane], (*ppBuf)->pMdlsPlanar[plane], (*ppBuf)->mode);

      if((*ppBuf)->kernel_dma[plane].page_count == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_setup: failed to init kernel DMA, plane(%d) page_count(%d)\n",
        		 plane, (*ppBuf)->kernel_dma[plane].page_count));
         return -ENOMEM;
      }

     /* Allocate Windows SGL Memory */
	  {
		 unsigned long WinSGLength = sizeof(SCATTER_GATHER_LIST) +
				                     ((*ppBuf)->kernel_dma[plane].page_count * sizeof(SCATTER_GATHER_ELEMENT));

		 /* Assign memory to pWinSGL */
		 (*ppBuf)->pWinSGL[plane] = KernelVzalloc(WinSGLength);

		 RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_buffer_setup: Alloc %lu bytes to pWinSGL[%d](0x%p)\n",
			   WinSGLength, plane, (*ppBuf)->pWinSGL[plane]));
		 if((*ppBuf)->pWinSGL[plane] == NULL)
		 {
		    RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_setup: failed to alloc %d bytes to pWinSGL[%d]\n", WinSGLength, plane));
		    return -ENOMEM;
	     }
	  }
   }

   pIrp = AllocAndSetupIrp((*ppBuf)->h);
   if(NULL == pIrp)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_setup: failed to allocate IRP\n"));
      return -ENOMEM;
   }
   (*ppBuf)->pIrp = pIrp;

   (*ppBuf)->pReadMemory = (void*)data;  // Populate member in buffer (via union).

   return 0;
}

int rgb133_userptr_buffer_setup(struct rgb133_unified_buffer** ppBuf, unsigned long data, unsigned int size,
      OSPROCESSOR_MODE AccessMode)
{
   int plane = 0;
   int sizePlane = 0;
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_userptr_buffer_setup: START\n"));

   if(ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_setup: invalid buffer ptr ppBuf\n"));
      return -EINVAL;
   }
   else if((*ppBuf) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_setup: invalid buffer (*ppBuf)\n"));
      return -EINVAL;
   }

   if((*ppBuf)->pMessageBuffer == 0)
   {
      (*ppBuf)->pMessageBuffer = KernelVzalloc(size);
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_userptr_buffer_setup: alloc %d bytes to buf->pMessageBuffer(0x%p)\n",
            size, (*ppBuf)->pMessageBuffer));
   }

   for(plane=0;plane<(*ppBuf)->h->sCapture.planes;plane++)
   {
      /* for NV12 and YV12, size of plane Y is 2/3 the size of all data */
      if (((*ppBuf)->h->sCapture.pixelformat == V4L2_PIX_FMT_NV12 || (*ppBuf)->h->sCapture.pixelformat == V4L2_PIX_FMT_YVU420)
           && plane == 0)
         sizePlane = 2 * size / 3;
      else
      {
         /* Intuitively, this should be 1/3 * size
          * However, to work correctly the core driver expects the length of the second plane to be set to the entire buffer length
          * (for detail, look into RGBWaitForDataK in the core or into SetUpIoctlIn)
          */
         sizePlane = size;
      }

      (*ppBuf)->pMdlsPlanar[plane] = IoAllocateMdl((void*)data, sizePlane, FALSE, FALSE, NULL);
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_userptr_buffer_setup: alloc pMdlsPlanar[%d](0x%p)\n", plane, (*ppBuf)->pMdlsPlanar[plane]));
      
      if((*ppBuf)->pMdlsPlanar[plane] == NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_setup: failed to allocate pMdlsPlanar[%d] with data(0x%lx - %d)\n",
               plane, data, size));
         return -ENOMEM;
      }
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_userptr_buffer_setup: buf[%d](0x%p) allocate pMdlsPlanar[%d](0x%p) with data(0x%lx - %d)\n",
            (*ppBuf)->index, (*ppBuf), plane, (*ppBuf)->pMdlsPlanar[plane], data, size));

      rgb133_init_kernel_dma(&(*ppBuf)->dma_info[plane], &(*ppBuf)->kernel_dma[plane], (*ppBuf)->pMdlsPlanar[plane], (*ppBuf)->mode);
      if((*ppBuf)->kernel_dma[plane].page_count == 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_setup: failed to init kernel DMA, plane(%d) page_count(%d)\n",
               plane, (*ppBuf)->kernel_dma[plane].page_count));
         return -ENOMEM;
      }

      /* Allocate Windows SGL Memory */
      {
         unsigned long WinSGLength = sizeof(SCATTER_GATHER_LIST) +
                                     ((*ppBuf)->kernel_dma[plane].page_count * sizeof(SCATTER_GATHER_ELEMENT));

         /* Assign memory to pWinSGL */
         (*ppBuf)->pWinSGL[plane] = KernelVzalloc(WinSGLength);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_userptr_buffer_setup: Alloc %lu bytes to pWinSGL[%d](0x%p)\n\n",
               WinSGLength, plane, (*ppBuf)->pWinSGL[plane]));
         if((*ppBuf)->pWinSGL[plane] == NULL)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_setup: failed to alloc %d bytes to pWinSGL[%d]\n", WinSGLength, plane));
            return -ENOMEM;
         }
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_userptr_buffer_setup: END\n"));
   return 0;
}

void rgb133_read_buffer_free(struct rgb133_unified_buffer** ppBuf,
      OSPROCESSOR_MODE AccessMode)
{
   struct rgb133_handle * h = 0;
   int i = 0;
   if(ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_free: invalid buffer ptr ppBuf\n"));
      return;
   }
   else if((*ppBuf) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_read_buffer_free: invalid buffer (*ppBuf)\n"));
      return;
   }

   /* Unallocate Windows SGL Memory and Memory Descriptor Lists */
   for(i=0;i<((*ppBuf)->h->sCapture.planes);i++)
   {
      if((*ppBuf)->pWinSGL[i] != NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_read_buffer_free: Free pWinSGL[%d](0x%p)\n", i, (*ppBuf)->pWinSGL[i]));
         KernelVfree((*ppBuf)->pWinSGL[i]);
         (*ppBuf)->pWinSGL[i] = 0;
      }

      rgb133_uninit_kernel_dma(&(*ppBuf)->dma_info[i], &(*ppBuf)->kernel_dma[i], (*ppBuf)->pMdlsPlanar[i], AccessMode);
      if((*ppBuf)->pMdlsPlanar[i])
      {
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_buffer_free: buf[%d](0x%p), free pMdlsPlanar[%d](0x%p)\n",
               (*ppBuf)->index, (*ppBuf), i, (*ppBuf)->pMdlsPlanar[i]));
         IoFreeMdl((*ppBuf)->pMdlsPlanar[i]);
         (*ppBuf)->pMdlsPlanar[i] = 0;
      }
   }

   FreeIrp((*ppBuf)->pIrp);

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_read_buffer_free: free buffer[%d](0x%p)\n",
         (*ppBuf)->index, (*ppBuf)));
   rgb133_buffer_free(&(*ppBuf));
}

void rgb133_userptr_buffer_free(struct rgb133_unified_buffer** ppBuf,
      OSPROCESSOR_MODE AccessMode)
{
   int i = 0;
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_userptr_buffer_free: START\n"));

   if(ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_free: invalid buffer ptr ppBuf\n"));
      return;
   }
   else if((*ppBuf) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_free: invalid buffer (*ppBuf)\n"));
      return;
   }

   /* Unallocate Windows SGL Memory
    * and Memory Descriptor Lists */
   for(i=0;i<((*ppBuf)->h->sCapture.planes);i++)
   {
      /* Free memory from pWinSGL */
      if((*ppBuf)->pWinSGL[i] != NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_userptr_buffer_free: Free pWinSGL[%d](0x%p)\n", i, (*ppBuf)->pWinSGL[i]));
         KernelVfree((*ppBuf)->pWinSGL[i]);
         (*ppBuf)->pWinSGL[i] = 0;
      }

      rgb133_uninit_kernel_dma(&(*ppBuf)->dma_info[i], &(*ppBuf)->kernel_dma[i], (*ppBuf)->pMdlsPlanar[i], AccessMode);

      if((*ppBuf)->pMdlsPlanar[i])
      {
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_userptr_buffer_free: buf(0x%p), free pMdlsPlanar[%d](0x%p)\n\n",
               (*ppBuf), i, (*ppBuf)->pMdlsPlanar[i]));
         IoFreeMdl((*ppBuf)->pMdlsPlanar[i]);
         (*ppBuf)->pMdlsPlanar[i] = 0;
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_userptr_buffer_free: END\n"));
}

/*!
** This indicates the amount of memory to allocate (by __rgb133_buffer_mmap_dma_setup) as our "shared"
** buffer with usermode.  It must indicate enough memory to span RGB133_MAX_FRAME buffers of data, otherwise
** our page-fault handler will cause a panic when it tries to map pages in from this buffer to buffer 3
** (i.e. 4th buffer).
** Called by rgb133_reqbufs() IOCTL handler
*/
int rgb133_mmap_buffer_setup(struct rgb133_handle* h, unsigned int* pCount, unsigned int* pSize)
{
   unsigned int num = 0;
   unsigned int denum = 0;
   int channel = -1;
   int bpp = -1;

   if(pCount == NULL ||
      pSize == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap_buffer_setup: invalid pCount(0x%p) or pSize(0x%p)\n",
            pCount, pSize));
      return -EINVAL;
   }

   channel = h->channel;

   bpp = v4lBytesPerPixel(h->sCapture.pixelformat);
   if(bpp == -1)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap_buffer_setup: invalid pixelformat(0x%x) bpp(%d)\n", h->sCapture.pixelformat, bpp));
      return -EINVAL;
   }

   switch(h->sCapture.pixelformat)
   {
      case V4L2_PIX_FMT_NV12:
      case V4L2_PIX_FMT_YVU420:
         /* because we divide CaptureWidth by 2 below, we need CaptureWidth even */
         if(h->sCapture.CaptureWidth & 1)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap_buffer_setup: Incorrect capture width(%d) for pixel format NV12. Must be a multiply of 2\n",
                  h->sCapture.CaptureWidth));
           return -EINVAL;
         }
         num = 3;
         denum = 2;
         break;
      default:
         num = denum = 1;
         break;
   }

   *pSize = (h->sCapture.CaptureWidth * bpp) * h->sCapture.CaptureHeight * num / denum;
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_mmap_buffer_setup(a): assign size = (h->sCapture.CaptureWidth(%lu) * bpp[0x%x](%d) * h->sCapture.CaptureHeight)(%u) * %d / %d\n",
       h->sCapture.CaptureWidth, h->sCapture.pixelformat, bpp, h->sCapture.CaptureHeight, num, denum));

   if(*pSize == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap_buffer_setup: Failed to assign valid size from cap_w(%lu) cap_h(%lu) bpp(%lu)\n",
            h->sCapture.CaptureWidth, h->sCapture.CaptureHeight, v4lBytesPerPixel(h->sCapture.pixelformat)));
      return -1;
   }

   if(pCount)
   {
      if(*pCount > RGB133_MAX_FRAME)
      {
         RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_mmap_buffer_setup: limit *pCount(%u) to RGB133_MAX_FRAME(%d)\n",
               *pCount, RGB133_MAX_FRAME));
         *pCount = RGB133_MAX_FRAME;
      }
   }

   return 0;
}

/*!
** Called to setup the buffer's allocation.  Also sorts out SG tables ahead of time.
** Memory allocation is undone by a call to __rgb133_buffer_mmap_dma_free().
**
** This function is now called as part of the VIDIOVTL_REQBUFS handler, rather than in mmap.
*/
int __rgb133_buffer_reqbuf_dma_setup(struct rgb133_unified_buffer** ppBuf)
{
   int retval = -EINVAL;

   void* baddr = 0;
   int bsize = 0;
   int plane = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "__rgb133_buffer_reqbuf_dma_setup: START\n"));

   if(ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_free: invalid buffer ptr ppBuf\n"));
      retval = -EINVAL;
      goto out;
   }
   else if((*ppBuf) == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_userptr_buffer_free: invalid buffer (*ppBuf)\n"));
      retval = -EINVAL;
      goto out;
   }

   baddr = KernelVzalloc(PAGE_ALIGN((*ppBuf)->bsize));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_reqbuf_dma_setup: alloc %d bytes to baddr(0x%p)\n",
         PAGE_ALIGN((*ppBuf)->bsize), baddr));
   if(baddr)
      memset(baddr, 0x5c, PAGE_ALIGN((*ppBuf)->bsize));
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_reqbuf_dma_setup: failed to allocate %d bytes to mmap buffer\n",
            PAGE_ALIGN((*ppBuf)->bsize)));
      retval = -ENOMEM;
      goto out;
   }

   for(plane=0;plane<(*ppBuf)->h->sCapture.planes;plane++)
   {
      /* for NV12 and YV12, size of plane Y is 2/3 the size of all data */
      if (((*ppBuf)->h->sCapture.pixelformat == V4L2_PIX_FMT_NV12 || (*ppBuf)->h->sCapture.pixelformat == V4L2_PIX_FMT_YVU420)
           && plane == 0)
        bsize = 2 * (*ppBuf)->bsize / 3;
      else
      {
         /* Intuitively, this should be 1/3 * size
          * However, to work correctly the core driver expects the length of the second plane to be set to the entire buffer length
          * (for detail, look into RGBWaitForDataK in the core or into SetUpIoctlIn)
          */
         bsize = (*ppBuf)->bsize;
      }

      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_reqbuf_dma_setup: setup MDL[%d] using baddr(0x%p) and bsize(%d)\n",
    		  plane, baddr, bsize));
      (*ppBuf)->pMdlsPlanar[plane] = IoAllocateMdl(baddr, bsize, FALSE, FALSE, NULL);

      if((*ppBuf)->pMdlsPlanar[plane] == NULL)
      {
	     RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_reqbuf_dma_setup: failed to allocate pMdlsPlanar[%d] with baddr(0x%p - %d)\n",
			    plane, baddr, bsize));
	     retval = -ENOMEM;
	     goto out;
      }
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_reqbuf_dma_setup: allocate pMdlsPlanar[%d](0x%p) with baddr(0x%p - %d)\n",
		      plane, (*ppBuf)->pMdlsPlanar[plane], baddr, bsize));

      rgb133_init_kernel_dma(&(*ppBuf)->dma_info[plane], &(*ppBuf)->kernel_dma[plane], (*ppBuf)->pMdlsPlanar[plane], (*ppBuf)->mode);

      if((*ppBuf)->kernel_dma[plane].page_count == 0)
      {
	     RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_reqbuf_dma_setup: failed to init user DMA, plane(%d) page_count(%d)\n",
	    		 plane, (*ppBuf)->kernel_dma[plane].page_count));
	     retval = -ENOMEM;
	     goto out;
      }

      /* Allocate Windows SGL Memory */
      {
         unsigned long WinSGLength = sizeof(SCATTER_GATHER_LIST) +
                                      ((*ppBuf)->kernel_dma[plane].page_count * sizeof(SCATTER_GATHER_ELEMENT));

	      /* Assign memory to pWinSGL */
	      (*ppBuf)->pWinSGL[plane] = KernelVzalloc(WinSGLength);
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_reqbuf_dma_setup: Alloc %lu bytes to pWinSGL[%d](0x%p)\n",
              WinSGLength, plane, (*ppBuf)->pWinSGL[plane]));
         if((*ppBuf)->pWinSGL[plane] == NULL)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_reqbuf_dma_setup: failed to alloc %d bytes to pWinSGL[%d]\n", WinSGLength, plane));
            retval = -ENOMEM;
            goto out;
         }
      }
   }

   (*ppBuf)->pMMapMemory = baddr;  // Populate member in buffer (via union).
   retval = 0;

out:

RGB133PRINT((RGB133_LINUX_DBG_INOUT, "__rgb133_buffer_reqbuf_dma_setup: END\n"));
   return retval;
}

int NumQueuedBuffers(struct rgb133_handle *h, int buf_type)
{
   int i, count = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "NumQueuedBuffers: START - h(0x%p)->buffers(0x%p) numbuffers(%d)\n",
         h, h->buffers, h->numbuffers));

   if(h->buffers != NULL)
   {
      for (i=0;i<h->numbuffers;i++)
      {
         if(h->buffers[i] != NULL)
         {
            if (h->buffers[i]->state & buf_type)
            {
               count++;
            }
         }
      }
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "NumQueuedBuffers: END - h(0x%p)\n", h));

   return count;
}

void DeleteQueuedBuffers(struct rgb133_handle *h, int buf_type)
{
   int i;

   if(h->buffers != NULL)
   {
      for (i=0;i<h->numbuffers;i++)
      {
         if(h->buffers[i] != NULL)
         {
            if (h->buffers[i]->state & buf_type)
            {
               RGB133PRINT((RGB133_LINUX_DBG_MEM, "DeleteQueuedBuffers: free h(0x%p)->buffers[%d](0x%p)\n", h, i, h->buffers[i]));
               KernelVfree(h->buffers[i]);
               h->buffers[i] = NULL;
            }
         }
      }
      h->numbuffers = 0;
   }
}

/*!
** Called by VIDIOC_REQBUF handler to perform the queue initialisation for mmap buffers.
** On exit, h will be setup with count buffers.
*/
int __rgb133_buffer_reqbuf_setup(struct rgb133_handle* h, unsigned int count,
      unsigned int size, enum v4l2_memory memory)
{
   PIRP pIrp = NULL;
   struct rgb133_unified_buffer * buf;
   int retval = 0;
   int i = 0;
   
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "__rgb133_buffer_reqbuf_setup: START [%d][%d][%d] - count(%u) size(%u)\n",
         h->dev->index, h->channel, h->capture, count, size));

   if(NumQueuedBuffers(h, RGB133_BUF_QUEUED))
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "__rgb133_buffer_reqbuf_setup: freeing extant buffers on h(0x%p)\n", h));

      DeleteQueuedBuffers(h, V4L2_BUF_FLAG_QUEUED);
   }

   if (h->buffers != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_reqbuf_setup: free h(0x%p)->buffers(0x%p)\n", h, h->buffers));
      KernelVfree(h->buffers);
      h->buffers = NULL;
      h->numbuffers = 0;
   }
   
   h->buffers = KernelVzalloc(sizeof(void*)*count);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_reqbuf_setup: alloc %d bytes to h(0x%p)->buffers(0x%p)\n",
         sizeof(void*)*count, h, h->buffers));
   h->numbuffers = count;

   for (i=0; i<count; i++)
   {
      buf = rgb133_buffer_alloc(h);

      if(buf == NULL)
      {
         retval = -ENOMEM;
         break;
      }
      buf->index = i;
      buf->bsize = size;
      buf->boff = PAGE_ALIGN(size) * i;
      buf->state = RGB133_BUF_PREPARED; // As in "prepared for use" not prepared in terms of captured into.
      buf->init = false;
      buf->memory = memory;

      if(memory == V4L2_MEMORY_MMAP)
      {
         /* Set the appropriate access mode */
         buf->mode = KernelMode;

         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_reqbuf_setup: setup pMDL and DMA\n"));
         if((retval = __rgb133_buffer_reqbuf_dma_setup(&buf)) != 0)
         {
            retval = -ENOMEM;
         }
      }
      else
      {
         /* Set the appropriate access mode */
         buf->mode = UserMode;

         /* Page Align Size */
         buf->bsize = PAGE_ALIGN(size);
      }

      /* Allocate fake windows IRP */
      pIrp = AllocAndSetupIrp(h);
      if(NULL == pIrp)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_reqbuf_setup: failed to allocate IRP\n"));
         retval = -ENOMEM;
      }
      buf->pIrp = pIrp;

      if(memory == V4L2_MEMORY_MMAP)
      {
         // The buffer's MDL was set up upfront in __rgb133_buffer_reqbuf_dma_setup()... so let's just copy it in here.
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_reqbuf_setup: assign pMdlsPlanar[0](0x%p), pMdlsPlanar[1](0x%p), pMdlsPlanar[2](0x%p) to pIrp(0x%p)\n",
               buf->pMdlsPlanar[0], buf->pMdlsPlanar[1], buf->pMdlsPlanar[2], pIrp));

         {
            int i;
            for(i=0; i<(h->sCapture.planes); i++)
               pIrp->MdlPlanarAddress[i] = buf->pMdlsPlanar[i];
         }
         // we still need this one (used by ReadFlash_V27)
         pIrp->MdlAddress = buf->pMdlsPlanar[0];
      }
      
      h->buffers[i] = buf;
      
      RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_reqbuf_setup: alloc buffer %d (0x%p)\n",
            i, buf));
   }
   
   if (retval != 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "__rgb133_buffer_reqbuf_setup: at least 1 allocation failed - cleaning up\n"));
      for (i=0; i<count; i++)
      {
         if (h->buffers[i])
         {
            rgb133_buffer_free(&h->buffers[i]);
            h->buffers[i] = NULL;
         }
      }
      h->numbuffers = 0;
   }
   
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "__rgb133_buffer_reqbuf_setup: END [%d][%d][%d]\n",
         h->dev->index, h->channel, h->capture, count, size));

   return retval;
}

int rgb133_mmap_buffer_status(struct rgb133_handle* h, struct v4l2_buffer* b)
{
   enum v4l2_buf_type type = h->buffertype;
   struct rgb133_unified_buffer * buf;
   
   if(b == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap_buffer_status: invalid buffer b == 0\n"));
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_mmap_buffer_status: START - buffer[%d]\n",
         b->index));

   if (b->index >= h->numbuffers)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mmap_buffer_status: invalid buffer index %d (max is %d)\n",
                   b->index, h->numbuffers));
      return -EINVAL;
   }
   
   if(h->buffers[b->index] == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "rgb133_mmap_buffer_status: h(0x%p)->buffers(0x%p)[%d] is NULL [%d][%d][%d]\n",            h, h->buffers, b->index, h->dev->index, h->channel, h->capture));
      return -ENOBUFS;
   }

   buf = h->buffers[b->index];
   
   b->index = buf->index;
   b->type = type;
   b->memory = buf->memory;
   b->timecode.type = V4L2_TC_TYPE_60FPS;

   switch(b->memory)
   {
      case V4L2_MEMORY_MMAP:
      case V4L2_MEMORY_USERPTR:
         b->m.offset = buf->boff;
         b->length = buf->bsize;
         RGB133PRINT((RGB133_LINUX_DBG_TRACE, "rgb133_mmap_buffer_status: set offset(%u) and length(%u)\n",
                      b->m.offset, b->length));
         break;
      default:
         break;
   }

   b->flags = 0;

   switch(buf->state)
   {
      case RGB133_BUF_PREPARED:
      case RGB133_BUF_QUEUED:
      case RGB133_BUF_ACTIVE:
         b->flags |= V4L2_BUF_FLAG_QUEUED;
         break;
      case RGB133_BUF_DONE:
      case RGB133_BUF_ERROR:
         b->flags |= V4L2_BUF_FLAG_DONE;
         break;
      case RGB133_BUF_NEEDS_INIT:
      case RGB133_BUF_IDLE:
         break;
   }

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_mmap_buffer_status: END - buffer[%d]\n",
         b->index));
   return 0;
}

/*
** Maps the buffer into the user-mode process.
** Called as part of the mmap driver handler, rgb133_mmap()
*/
int __rgb133_buffer_mmap_mapper(struct rgb133_handle* h, struct vm_area_struct* vma)
{
   unsigned int first = 0, last = 0;
   int retval = -EINVAL;
   struct rgb133_vm_privdata * pd;
   unsigned long size, offset;
   
   vma->vm_private_data = 0;
   
   if(!(vma->vm_flags & VM_SHARED))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_mmap_mapper: vma->vm_flags requires MAP_SHARED\n"));
      goto out;
   }

   /*
   ** first and last allow for a frame of data to be spread over multiple buffers.
   */
   
   if(first == RGB133_MAX_FRAME)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "__rgb133_buffer_mmap_mapper: "
                   "offset(0x%lx) invalid\n", (vma->vm_pgoff << PAGE_SHIFT)));
      goto out;
   }

   if(last == RGB133_MAX_FRAME)
   {
      RGB133PRINT((RGB133_LINUX_DBG_WARNING, "__rgb133_buffer_mmap_mapper: "
                   "size(0x%lx) invalid\n", (vma->vm_end - vma->vm_start)));
   }

   /*
   ** In our scheme, a buffer will always represent one frame.
   */
   RGB133PRINT((RGB133_LINUX_DBG_LOG, "__rgb133_buffer_mmap_mapper: vma->\n   vm_start: %p\n vm_end: %p\n   vm_pgoff: %p (%x)\n",
                vma->vm_start, vma->vm_end, vma->vm_pgoff*PAGE_SIZE, vma->vm_pgoff));

   /* Get offset passed to user by calculating it, rather than being given it, like all the documentation states. */
   size = vma->vm_end - vma->vm_start;
   offset = (vma->vm_pgoff * PAGE_SIZE) / size;

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "__rgb133_buffer_mmap_mapper: Worked out size (%d) and offset (%d)\n", size, offset));

   /* offset now actually contains the buffer number that the user's trying to mmap in. */
   first = last = offset;
   
   pd = KernelVzalloc(sizeof(struct rgb133_vm_privdata));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_mmap_mapper: alloc %d bytes to pd(0x%p) for buffer[%d] h(0x%p)",
         sizeof(struct rgb133_vm_privdata), pd, offset, h))
   pd->b = h->buffers[offset];
   pd->h = h;
   pd->count = 0;
   rgb133_set_mapped_buffer(h->buffers[offset], 1);

vma->vm_ops = &rgb133_buf_vm_ops;
#ifdef RGB133_CONFIG_HAVE_VM_RESERVED
   vm_flags_set(vma, VM_DONTEXPAND | VM_RESERVED);
#else
   vm_flags_set(vma, VM_DONTEXPAND | VM_DONTDUMP);
#endif
   vm_flags_clear(vma, VM_IO);
   vma->vm_private_data = pd;
   retval = 0;

   RGB133PRINT((RGB133_LINUX_DBG_TRACE, "__rgb133_buffer_mmap_mapper: mapped %d bytes into 0x%lx"
                "- 0x%lx with buffers %d - %d\n", size, vma->vm_start, vma->vm_end, first, last));

out:
   return retval;
}

void __rgb133_buffer_mmap_dma_free(struct rgb133_unified_buffer ** ppBuf)
{
   int i;
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "__rgb133_buffer_mmap_dma_free: START\n"));

   if(ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_mmap_dma_free: Invalid buffer address.\n"));
      return;
   }
   else if(*ppBuf == 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "__rgb133_buffer_mmap_dma_free: Invalid buffer at 0x%p.\n",
            ppBuf));
      return;
   }

   /* Unallocate Windows SGL Memory */
   /* and Memory Descriptor Lists */
   for(i=0;i<((*ppBuf)->h->sCapture.planes);i++)
   {
      if((*ppBuf)->pWinSGL[i] != NULL)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_mmap_dma_free: Free pWinSGL[%d](0x%p)\n",
               i, (*ppBuf)->pWinSGL[i]));
         KernelVfree((*ppBuf)->pWinSGL[i]);
         (*ppBuf)->pWinSGL[i] = 0;
      }

      rgb133_uninit_kernel_dma(&(*ppBuf)->dma_info[i], &(*ppBuf)->kernel_dma[i], (*ppBuf)->pMdlsPlanar[i], (*ppBuf)->mode);

     if((*ppBuf)->pMdlsPlanar[i])
     {
        RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_mmap_dma_free: free buffer->pMdlsPlanar[%d](0x%p)\n",
             i, (*ppBuf)->pMdlsPlanar[i]));
        IoFreeMdl((*ppBuf)->pMdlsPlanar[i]);
        (*ppBuf)->pMdlsPlanar[i] = 0;
     }
   }

   if((*ppBuf)->memory == V4L2_MEMORY_MMAP)
   {
      if((*ppBuf)->pMMapMemory)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_mmap_dma_free: free buffer[%d](0x%p)->pMMapMemory(0x%p)\n",
               (*ppBuf)->index, (*ppBuf), (*ppBuf)->pMMapMemory));
         KernelVfree((*ppBuf)->pMMapMemory);
         (*ppBuf)->pMMapMemory = NULL;
      }
   }

   if((*ppBuf)->memory == V4L2_MEMORY_USERPTR)
   {
      if((*ppBuf)->pMessageBuffer)
      {
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_mmap_dma_free: free buf[%d](0x%p)->pMessageBuffer(0x%p)\n",
               (*ppBuf)->index, (*ppBuf), (*ppBuf)->pMessageBuffer));
         KernelVfree((*ppBuf)->pMessageBuffer);
         (*ppBuf)->pMessageBuffer = 0;
      }
   }

   FreeIrp((*ppBuf)->pIrp);

   RGB133PRINT((RGB133_LINUX_DBG_MEM, "__rgb133_buffer_mmap_dma_free: free buffer[%d](0x%p)\n",
         (*ppBuf)->index, *ppBuf));
   KernelVfree(*ppBuf);
   *ppBuf = 0;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "__rgb133_buffer_mmap_dma_free: END\n"));
}

BOOL rgb133_buffer_queued(struct rgb133_handle* h, struct v4l2_buffer *b)
{
   int bufn = b?b->index:-1;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_buffer_q_queued: investigating buffer index %d\n", bufn));

   if (bufn != -1)
   {
      if(h->buffers != NULL)
      {
         if(h->buffers[bufn] != NULL)
         {
            return !!(h->buffers[bufn]->state & RGB133_BUF_QUEUED);
         }
      }
   }
   return false;
}

void rgb133_buffer_set_queued(struct rgb133_handle* h, struct rgb133_unified_buffer * buf, BOOL value)
{
   int bufn = buf ? buf->index : -1;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_buffer_set_queued: investigating for handle (0x%p) buffer index %d\n", h, bufn));

   if (bufn != -1)
   {
      if (value)
      {
         buf->state = RGB133_BUF_QUEUED;
         buf->notify = RGB133_NOTIFY_RESERVED;
      }
      else
         buf->state &= ~RGB133_BUF_QUEUED;
   }
}

void rgb133_buffer_set_active(struct rgb133_handle* h, struct rgb133_unified_buffer * buf, BOOL value)
{
   int bufn = buf ? buf->index : -1;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_buffer_set_active: investigating for handle (0x%p) buffer index %d\n", h, bufn));

   if (bufn != -1)
   {
      if (value)
         buf->state = RGB133_BUF_ACTIVE;
      else
         buf->state &= ~RGB133_BUF_ACTIVE;
   }
}

/*!
** Slightly misnamed - returns whether there are mapped buffers queued
*/
BOOL rgb133_is_mapped(struct rgb133_handle* h)
{
   if(h)
   {
      return h->mmapped;
   }

   return false;
}
 
void rgb133_set_mapped(struct rgb133_handle* h, BOOL value)
{
   if(h)
      h->mmapped = value;
}

/*
 * Returns whether a buffer is mapped
 */
BOOL rgb133_is_mapped_buffer(struct rgb133_unified_buffer * b)
{
   if(b)
   {
      return b->mmapped;
   }

   return false;
}

void rgb133_set_mapped_buffer(struct rgb133_unified_buffer * b, BOOL value)
{
   if(b)
      b->mmapped = value;
}

PMDL rgb133_userptr_buffer_get_mdl(struct rgb133_unified_buffer** ppBuf, ULONG plane)
{
   if(ppBuf)
   {
      if((*ppBuf))
         return (*ppBuf)->pMdlsPlanar[plane];
      else
         return 0;
   }
   else
      return 0;
}

PIRP AllocAndSetupIrp(struct rgb133_handle* h)
{
   PIRP pIrp = NULL;
   PIO_STACK_LOCATION irpStack = NULL;
   PFILE_OBJECT FileObject = NULL;

   FileObject = KernelVzalloc(sizeof(FILE_OBJECT));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "AllocAndSetupIrp: alloc %d bytes to FileObject(%p)\n",
         (int)sizeof(FILE_OBJECT), FileObject));
   if(NULL == FileObject)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "AllocAndSetupIrp: failed to allocate FileObject\n"));
      return NULL;
   }

   irpStack = KernelVzalloc(sizeof(IO_STACK_LOCATION));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "AllocAndSetupIrp: alloc %d bytes to irpStack(%p)\n",
         (int)sizeof(IO_STACK_LOCATION), irpStack));
   if(NULL == irpStack)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "AllocAndSetupIrp: failed to allocate irpStack free FileObject(0x%p)\n", FileObject));
      KernelVfree(FileObject);
      return NULL;
   }

   pIrp = KernelVzalloc(sizeof(IRP));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "AllocAndSetupIrp: alloc %d bytes to pIrp(%p)\n", (int)sizeof(IRP), pIrp));
   if(NULL == pIrp)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "AllocAndSetupIrp: failed to allocate IRP free irpStack(0x%p) FileObject(0x%p)\n",
            irpStack, FileObject));
      KernelVfree(irpStack);
      KernelVfree(FileObject);
      return NULL;
   }

   FileObject->FsContext2 = NULL;
   irpStack->FileObject = FileObject;
   DeviceSetDeviceObject(h->dev->pDE, irpStack);
   pIrp->irpStack = irpStack;

   return pIrp;
}

void FreeIrp(PIRP pIrp)
{
   if(pIrp)
   {
      if(pIrp->irpStack)
      {
         if(pIrp->irpStack->FileObject)
         {
            RGB133PRINT((RGB133_LINUX_DBG_MEM, "FreeIrp: free pIrp->irpStack->FileObject(%p)\n", pIrp->irpStack->FileObject));
            KernelVfree(pIrp->irpStack->FileObject);
            pIrp->irpStack->FileObject = NULL;
         }

         pIrp->irpStack->DeviceObject = NULL;
         RGB133PRINT((RGB133_LINUX_DBG_MEM, "FreeIrp: free pIrp->irpStack(%p)\n", pIrp->irpStack));
         KernelVfree(pIrp->irpStack);
         pIrp->irpStack = NULL;
      }

      RGB133PRINT((RGB133_LINUX_DBG_MEM, "FreeIrp: free pIrp(%p)\n", pIrp));
      KernelVfree(pIrp);
      pIrp = NULL;
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "FreeIrp: Invalid pIrp(NULL)\n"));
   }
}

BOOL rgb133_is_reading(struct rgb133_handle* h)
{
   if(h)
      return h->reading;
   else
      return false;
}
 
void rgb133_set_reading(struct rgb133_handle* h, BOOL value)
{
   if(h)
      h->reading = value;
}

BOOL rgb133_is_streaming(struct rgb133_handle* h)
{
   if(h)
   {
      if(h->streaming != RGB133_STRM_NOT_STARTED &&
         h->streaming != RGB133_STRM_STOPPED &&
         h->streaming != RGB133_STRM_OFF &&
         h->streaming != RGB133_STRM_UNKNOWN)
         return true;
      else
         return false;
   }
   else
      return false;
}

BOOL rgb133_streaming_valid(struct rgb133_handle* h)
{
   if(h)
   {
      if(h->streaming == RGB133_STRM_ON &&
         h->streaming == RGB133_STRM_OFF)
         return true;
      else
         return false;
   }
   else
      return false;
}

BOOL rgb133_streaming_on(struct rgb133_handle* h)
{
   if(h)
   {
      if(h->streaming == RGB133_STRM_ON)
         return true;
      else
         return false;
   }
   else
      return false;
}

void rgb133_set_streaming(struct rgb133_handle* h, BOOL value)
{
   if(h)
      h->streaming = value;
}

enum _rgb133_strm_state rgb133_get_streaming(struct rgb133_handle* h)
{
   if(h)
      return h->streaming;
   else
      return RGB133_STRM_UNKNOWN;
}

RGB133_KERNEL_DMA* rgb133_buffer_get_kernel_dma(struct rgb133_unified_buffer** ppBuf, ULONG plane)
{
   if(ppBuf)
   {
      if((*ppBuf))
         return &(*ppBuf)->kernel_dma[plane];
      else
         return 0;
   }
   else
      return 0;
}

void rgb133_invalidate_buffers(struct rgb133_handle* h)
{
   int i = 0;

   if(h->buffers != NULL)
   {
      if(h->buffers[0] != NULL)
      {
         for(i=0; i<h->numbuffers; i++)
         {
            if(h->buffers[i] != NULL)
            {
               h->buffers[i]->init = false;
            }
         }
      }
   }
}

