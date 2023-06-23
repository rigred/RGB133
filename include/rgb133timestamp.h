/*
 * rgb133timestamp.h
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

#ifndef RGB133_TIMESTAMP__H
#define RGB133_TIMESTAMP__H

typedef enum _eTimestamp
{
   RGB133_LINUX_TIMESTAMP_ACQ,
   RGB133_LINUX_TIMESTAMP_Q,
   RGB133_LINUX_TIMESTAMP_DMA,
   RGB133_LINUX_TIMESTAMP_DQ,
} eTimestamp;

typedef struct _sTimestamp
{
   unsigned long sec;
   unsigned long usec;
} sTimestamp, *psTimestamp;

#endif /* RGB133_TIMESTAMP__H */
