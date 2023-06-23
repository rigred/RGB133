/*
 * rgb133drm.c
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

#include "rgb133config.h"
#include "rgb133.h"

#include "OSAPI_TYPES.h"
#include "OSAPI.h"

extern unsigned long rgb133_hdcp;

LONG SecureMemoryTargetCount(VOID)
{
   /* If HDCP is enabled */
   return rgb133_hdcp;
}

typedef union _PCI_EXPRESS_LINK_STATUS_REGISTER {
  struct {
    USHORT LinkSpeed  :4;
    USHORT LinkWidth  :6;
    USHORT Undefined  :1;
    USHORT LinkTraining  :1;
    USHORT SlotClockConfig  :1;
    USHORT DataLinkLayerActive  :1;
    USHORT Rsvd  :2;
  };
  USHORT AsUInt16;
} PCI_EXPRESS_LINK_STATUS_REGISTER, *PPCI_EXPRESS_LINK_STATUS_REGISTER;

BOOLEAN GetLaneWidthAndLinkSpeed(PRGB133DEVAPI _rgb133dev, OSRESOURCE *PCIResource, USHORT *LinkSpeed, USHORT *LinkWidth)
{
   struct rgb133_dev* rgb133dev = (struct rgb133_dev*)_rgb133dev;
   struct pci_dev* pdev = NULL;
   int PciExprCapAddr = 0;
   BOOLEAN bReturn = FALSE;

   /* sanity checks */
   if (!rgb133dev)
      return FALSE;

   pdev = rgb133dev->core.pci;
   if (!pdev)
      return FALSE;

   KeEnterCriticalRegion();
   OSAcquireResourceExclusiveLite(PCIResource, TRUE);

   if ((PciExprCapAddr = pci_find_capability(pdev, PCI_CAP_ID_EXP)) != 0)
   {
      PCI_EXPRESS_LINK_STATUS_REGISTER LinkStatus;

      if (pci_read_config_word(pdev, PciExprCapAddr + PCI_EXP_LNKSTA, &(LinkStatus.AsUInt16)) == 0)
      {
        // RGB133PRINT((RGB133_LINUX_DBG_INIT, "GetLaneWidthAndLinkSpeed: read PCI Express status register for %s "
           //    "LinkStatus.LinkSpeed(%d) LinkStatus.LinkWidth(%d)\n",
             //  rgb133dev->core.name, LinkStatus.LinkSpeed, LinkStatus.LinkWidth));

         if (LinkStatus.LinkSpeed == 0)
            LinkStatus.LinkSpeed = 1;
         if (LinkStatus.LinkWidth == 0)
            LinkStatus.LinkWidth = 1;

         *LinkSpeed = LinkStatus.LinkSpeed;
         *LinkWidth = LinkStatus.LinkWidth;

         bReturn = TRUE;
      }
   }

   OSReleaseResourceLite(PCIResource);
   KeLeaveCriticalRegion();
   return bReturn;
}
