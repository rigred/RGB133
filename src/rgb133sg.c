/*
 * rgb133sg.c
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

#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>

#include "rgb133config.h"
#include "rgb133.h"
#include "rgb133debug.h"
#include "rgb133kernel.h"
#include "rgb133sg.h"

#ifndef RGB133_CONFIG_HAVE_SG_SET_PAGE
void sg_set_page(struct scatterlist *sg, struct page *page, unsigned int len, unsigned int offset)
{
   sg->page   = page;
   sg->offset = offset;
   sg->length = len;
}
#endif

#ifndef RGB133_CONFIG_HAVE_SG_INIT_TABLE
void sg_init_table(struct scatterlist * sg, unsigned int nents)
{
   memset(sg, 0, sizeof(struct scatterlist)*nents);
}
#endif

void rgb133_get_page_info(RGB133_DMA_PAGE_INFO* dma_page, unsigned long first, unsigned long size)
{
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_get_page_info: first(0x%p), size(%lu)\n",
      (void*)first, size));
   dma_page->uaddr = first & PAGE_MASK;
   dma_page->offset = first & ~PAGE_MASK;
   dma_page->tail = 1 + ((first+size-1) & ~PAGE_MASK);
   dma_page->first = (first & PAGE_MASK) >> PAGE_SHIFT;
   dma_page->last = ((first+size-1) & PAGE_MASK) >> PAGE_SHIFT;
   dma_page->page_count = dma_page->last - dma_page->first + 1;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_get_page_info: dma_page...first(0x%lx), last(0x%lx), tail(0x%lx), page_count(%d)\n",
      dma_page->first, dma_page->last, dma_page->last, dma_page->page_count));
   if(dma_page->page_count == 1)
      dma_page->tail -= dma_page->offset;
}

int rgb133_kernel_dma_fill_sg_list(RGB133_KERNEL_DMA* dma,
                                  RGB133_DMA_PAGE_INFO* dma_page,
                                  int map_offset)
{
   int i, j=10, offset;
   unsigned long flags;

   if(map_offset < 0)
      return map_offset;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_kernel_dma_fill_sg_list: START\n"));
   offset = dma_page->offset;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_kernel_dma_fill_sg_list: offset(%d) - page_count(%d)\n",
      offset, dma_page->page_count));
   for(i=0; i<dma_page->page_count; i++)
   {
      unsigned int len = (i == dma_page->page_count - 1) ?
         dma_page->tail : PAGE_SIZE - offset;

      if(j-->0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_kernel_dma_fill_sg_list(normal): map_offset(%d) sg_set_page(0x%p, 0x%p, %u, %d)\n",
            map_offset, &dma->pSGList[map_offset], dma->pMap[map_offset], len, offset));
      }

      sg_set_page(&dma->pSGList[map_offset], dma->pMap[map_offset], len, offset);

      offset = 0;
      map_offset++;
   }
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_kernel_dma_fill_sg_list: END\n"));
   return map_offset;
}

void rgb133_kernel_dma_fill_sg_array(RGB133_KERNEL_DMA* dma, unsigned long buffer_offset,
                                   unsigned int buffer_offset_2, unsigned int split)
{
   int i, j=5;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_kernel_dma_fill_sg_array: dma(0x%p) - buffer_offset(0x%lx)\n",
      dma, buffer_offset));
   for(i=0; i<dma->SG_length; i++)
   {
      struct scatterlist* sg = &dma->pSGList[i];
      dma->pSGArray[i].size = cpu_to_le64(sg_dma_len(sg));
      dma->pSGArray[i].src = cpu_to_le64(sg_dma_address(sg));
      dma->pSGArray[i].dst = cpu_to_le64(buffer_offset);
      buffer_offset += sg_dma_len(sg);

      if(j-->0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_kernel_dma_fill_sg_array: dma_addr_src(0x%llx)\n", (unsigned long long)sg_dma_address(sg)));
         RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_kernel_dma_fill_sg_array: src(0x%Lx), dst(0x%Lx), size(%Ld) - buffer_offset(0x%lx)\n",
            dma->pSGArray[i].src, dma->pSGArray[i].dst, dma->pSGArray[i].size, buffer_offset));
      }

      split -= sg_dma_len(sg);
      if(split == 0)
         buffer_offset = buffer_offset_2;
   }
}

void rgb133_kernel_dma_sync_for_device(struct pci_dev* pdev, struct scatterlist* pSGList, unsigned long page_count)
{
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_kernel_dma_sync_for_device: sync DMA with pdev(0x%p), SGList(0x%p), page_count(%lu)\n",
      pdev, pSGList, page_count));
   dma_sync_sg_for_device(&pdev->dev, pSGList, page_count, DMA_FROM_DEVICE);
}

void rgb133_kernel_init_sg_entries(RGB133_KERNEL_DMA* kernel_dma, int entries)
{
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_kernel_init_sg_entries: START\n"));
   // removed +1 on all allocations, and used the sg_init_table() function to avoid Oops.
   kernel_dma->pMap = KernelVzalloc(sizeof(struct page*) * entries);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_init_sg_entries: alloc %d bytes to kernel_dma->pMap(0x%p)\n",
         sizeof(struct page*) * entries, kernel_dma->pMap));
   kernel_dma->pBouncemap = KernelVzalloc(sizeof(struct page*) * entries);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_init_sg_entries: alloc %d bytes to kernel_dma->pBouncemap(0x%p)\n",
         sizeof(struct page*) * entries, kernel_dma->pBouncemap));
   kernel_dma->pSGArray = KernelVzalloc(sizeof(RGB133_SG_ELEMENT) * entries);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_init_sg_entries: alloc %d bytes to kernel_dma->pSGArray(0x%p)\n",
         sizeof(RGB133_SG_ELEMENT) * entries, kernel_dma->pSGArray));
   kernel_dma->pSGList = KernelVzalloc(sizeof(struct scatterlist) * entries);
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_init_sg_entries: alloc %d bytes to kernel_dma->pSGList(0x%p)\n",
         sizeof(struct scatterlist) * entries, kernel_dma->pSGList));
   sg_init_table(kernel_dma->pSGList, entries);
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_kernel_init_sg_entries: START\n"));
}

void rgb133_kernel_uninit_sg_entries(RGB133_KERNEL_DMA* kernel_dma)
{
   if(kernel_dma->pMap)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_uninit_sg_entries: free kernel_dma->pMap(0x%p)\n",
            kernel_dma->pMap));
      KernelVfree(kernel_dma->pMap);
   }
   if(kernel_dma->pBouncemap)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_uninit_sg_entries: free kernel_dma->pBouncemap(0x%p)\n",
            kernel_dma->pBouncemap));
      KernelVfree(kernel_dma->pBouncemap);
   }
   if(kernel_dma->pSGArray)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_uninit_sg_entries: free kernel_dma->pSGArray(0x%p)\n",
            kernel_dma->pSGArray));
      KernelVfree(kernel_dma->pSGArray);
   }
   if(kernel_dma->pSGList)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_kernel_uninit_sg_entries: free kernel_dma->pSGList(0x%p)\n",
            kernel_dma->pSGList));
      KernelVfree(kernel_dma->pSGList);
   }
   kernel_dma->pMap = 0;
   kernel_dma->pBouncemap = 0;
   kernel_dma->pSGArray = 0;
   kernel_dma->pSGList = 0;
}

int rgb133_init_kernel_dma(RGB133_DMA_PAGE_INFO* dma_info, RGB133_KERNEL_DMA* kernel_dma, PMDL pMdl, OSPROCESSOR_MODE AccessMode)
{
   unsigned long uaddr = (unsigned long)pMdl->StartVa;
   unsigned long size = pMdl->Size;

   int i;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_init_kernel_dma: START pMdl(0x%p) - uaddr(0x%lx) - size(%ld)\n", pMdl, uaddr, size));


   if(kernel_dma == NULL || dma_info == NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_init_kernel_dma: invalid dma arguments\n"));
      return -EINVAL;
   }

   /* Fill user DMA info */
   rgb133_get_page_info(dma_info, uaddr, size);

   if(dma_info->page_count <= 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_init_kernel_dma: failed to get DMA page info\n"));
      return -EINVAL;
   }

   kernel_dma->page_count = dma_info->page_count;
   kernel_dma->SG_length = 0;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_init_kernel_dma: Alloc %d SG entries\n",
         kernel_dma->page_count));
   rgb133_kernel_init_sg_entries(kernel_dma, kernel_dma->page_count);

   pMdl->map = kernel_dma->pMap;
   MmProbeAndLockPages(pMdl, AccessMode, IoWriteAccess);

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_init_kernel_dma: Fill SGList - user_dma(0x%p) - dma_info(0x%p)\n",
         kernel_dma, dma_info));
   if(rgb133_kernel_dma_fill_sg_list(kernel_dma, dma_info, 0) < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_init_kernel_dma: rgb133_kernel_dma_fill_sg_list failed\n"));
      if(AccessMode == UserMode)
      {
         for(i=0; i<kernel_dma->page_count; i++)
         {
            page_cache_release(kernel_dma->pMap[i]);
         }
      }
      kernel_dma->page_count = 0;
      return -EINVAL;
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_init_kernel_dma: END\n"));
   return 0;
}

void rgb133_uninit_kernel_dma(RGB133_DMA_PAGE_INFO* dma_info, RGB133_KERNEL_DMA* kernel_dma, PMDL pMdl, OSPROCESSOR_MODE AccessMode)
{
   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_uninit_kernel_dma: START\n"));

   if(kernel_dma == NULL || dma_info == NULL)
      return;

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_uninit_kernel_dma: pMdl(0x%p) - check and release %d pages from map(0x%p)\n",
      pMdl, kernel_dma->page_count, kernel_dma->pMap));

   if(kernel_dma->page_count > 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_uninit_kernel_dma: release mapped pages\n"));
      MmUnlockPages(pMdl);
   }
   else
   {
      RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_uninit_kernel_dma: no need to release mapped pages\n"));
   }

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_uninit_kernel_dma: Unalloc %d SG entries\n",
         kernel_dma->page_count));
   rgb133_kernel_uninit_sg_entries(kernel_dma);

   RGB133PRINT((RGB133_LINUX_DBG_DEBUG, "rgb133_uninit_kernel_dma: END\n"));
}
