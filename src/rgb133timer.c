/*
 * rgb133timer.c
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
#include <linux/timer.h>

#include "rgb133config.h"
#include "rgb_api_types.h"
#include "rgb133timer.h"
#include "rgb133debug.h"
#include "rgb133capapi.h"

#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
void rgb133_timer_setup(struct timer_list* pTimer, void (*function)(struct timer_list*), unsigned int flags)
{
   timer_setup(pTimer, function, flags);
}
#else
void rgb133_init_timer(struct timer_list* pTimer)
{
   init_timer(pTimer);
}

void rgb133_setup_timer(struct timer_list* pTimer, void (*function)(unsigned long), unsigned long data)
{
   setup_timer(pTimer, function, data);
}
#endif

int rgb133_mod_timer(PTIMERAPI _pTimer, unsigned long expires_ms)
{
   struct timer_list* pTimer = (struct timer_list*)_pTimer;
   unsigned long expires = jiffies + msecs_to_jiffies(expires_ms);
   return mod_timer(pTimer, expires);
}

void rgb133_del_timer(struct timer_list* pTimer)
{
   del_timer(pTimer);
}

int rgb133_timer_pending(struct timer_list* pTimer)
{
   return timer_pending(pTimer);
}

#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
void rgb133_clock_reset_passive_timer(struct timer_list* pTimer)
{
#else
void rgb133_clock_reset_passive_timer(unsigned long data)
{
   PTIMERAPI pTimer = (PTIMERAPI)data;
#endif
   CaptureClockResetPassiveTimer(pTimer);
}
