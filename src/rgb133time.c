/*
 * rgb133time.c
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

#include <linux/jiffies.h>

#include "rgb133config.h"

unsigned long TimeGetJiffies(void)
{
   return jiffies;
}

int TimeJiffiesToMsecs(unsigned long jiffies_in)
{
   return jiffies_to_msecs(jiffies_in);
}

