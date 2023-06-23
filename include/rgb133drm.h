/*
 * rgb133drm.h
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

#ifndef __RGB133_SECURE__H
#define __RGB133_SECURE__H

typedef struct _rgb133drm {
      unsigned long EnableHDCP;
} rgb133drm;

LONG SecureMemoryTargetCount(VOID);

BOOLEAN GetLaneWidthAndLinkSpeed(PRGB133DEVAPI _rgb133dev, OSRESOURCE *PCIResource, USHORT *LinkSpeed, USHORT *LinkWidth);

#endif /* __RGB133_SECURE__H */
