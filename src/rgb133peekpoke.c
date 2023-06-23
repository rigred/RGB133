/*
 * rgb133peekpoke.c
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
#include "rgb133capapi.h"
#include "rgb133.h"
#include "rgb133debug.h"
#include "rgb133kernel.h"

#include "rgb133peekpoke.h"

int rgb133_peekpoke_peek(struct file* file, void* priv, struct _srgb133_peek* psPeek)
{
   struct rgb133_handle* h = file->private_data;
   int ret = 0;

   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_peek: START - arg(0x%p)\n", psPeek));
   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_peek: device(%d) - subdevice(%d) - address(0x%x)\n",
      psPeek->device, psPeek->subdevice, psPeek->address));

   ret = CapturePeekRegister(h, psPeek);

   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_peek: END - status(%d) - data(0x%x): %d\n",
      psPeek->status, psPeek->data, ret));
   return ret;
}

int rgb133_peekpoke_poke(struct file* file, void* priv, struct _srgb133_poke* psPoke)
{
   struct rgb133_handle* h = file->private_data;
   int ret = 0;

   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_poke: START - arg(0x%p)\n", psPoke));
   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_poke: device(%d) - subdevice(%d) - address(0x%x) - data(0x%x)\n",
      psPoke->device, psPoke->subdevice, psPoke->address, psPoke->data));

   ret = CapturePokeRegister(h, psPoke);

   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_peek: END - status(%d): %d\n",
      psPoke->status, ret));
   return 0;
}

int rgb133_peekpoke_dump(struct file* file, void* priv, struct _srgb133_dump* psDump)
{
   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_dump: START - arg(0x%p)\n", psDump));
   RGB133PRINT((RGB133_LINUX_DBG_TODO, "rgb133_peekpoke_dump: END\n"));
   return 0;
}


