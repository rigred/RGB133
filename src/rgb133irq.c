/*
 * rgb133irq.c
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
#include <linux/module.h>
#include <linux/interrupt.h>

#include "rgb133config.h"
#include "rgb133.h"
#include "rgb133deviceapi.h"
#include "rgb133kernel.h"
#include "rgb133debug.h"
#include "rgb133irqapi.h"
#include "OSAPI_TYPES.h"
#include "rgb_windows_types.h"

/* Top level ISR ISR */
#ifdef RGB133_CONFIG_IRQ_HAS_PT_REGS
irqreturn_t rgb133_irq(unsigned int irq, void* dev_id, struct pt_regs* regs)
#else
irqreturn_t rgb133_irq(unsigned int irq, void* dev_id)
#endif
{
   int handled = 0;
   PKINTERRUPT Interrupt = NULL;
   PDEVICEAPI dev = (PDEVICEAPI)dev_id;
   
   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "rgb133_irq: Got an interrupt(%d), dev_id(0x%p)\n",
         irq, dev_id));
   
   handled = IRQ_ISR(Interrupt, dev);

   RGB133PRINT((RGB133_LINUX_DBG_IRQ, "rgb133_irq: exit\n"));

   return IRQ_RETVAL(handled);
}

