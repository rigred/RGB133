/*
 * rgb133edid.h
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

#ifndef __RGB133_EDID__H
#define __RGB133_EDID__H

typedef struct _rgb133EdidOps {
   unsigned long Bytes;          // Size of EDID
   char          Edid[256];      // The EDID (with room for extension block)
   unsigned long bGetDefault;    // ???
   unsigned long Channel;        // Which channel (Input) on the card to retrieve.
} rgb133EdidOps, *prgb133EdidOps;

#endif /* __RGB133_EDID__H */
