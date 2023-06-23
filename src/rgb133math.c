/*
 * rgb133math.c
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

#include <asm/div64.h>

long long __divdi3(long long a, long long b)
{
   do_div(a, b);
   return a;
}

unsigned long long __udivdi3(unsigned long long a, unsigned long long b)
{
   do_div(a, b);
   return a;
}

unsigned long long __umoddi3(unsigned long long a, unsigned long long b)
{
   return do_div(a, b);
}
