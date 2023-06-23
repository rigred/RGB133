/*
 * rgb133sndcard.c
 *
 * Copyright (c) 2013 Datapath Limited All rights reserved.
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
#ifdef RGB133_CONFIG_HAVE_SNDRV_CARDS_IN_DRIVER
#include <sound/driver.h>
#endif
#include <sound/core.h>
#include <sound/asound.h>
#include <sound/pcm.h>
#include <sound/control.h>

#include "rgb_linux_types.h"
#include "rgb_api_types.h"
#include "rgb133audioapi.h"
#include "rgb133audio.h"
#include "rgb133audiocontrol.h"
#include "rgb133.h"
#include "rgb133kernel.h"

int SndCardGetSize(void)
{
   return sizeof(struct snd_card);
}

int SndCardBuild(PRGB133DEVAPI _dev, int index, const char* id, PMODULEAPI _module, int extra_size, void** _ppcard)
{
   struct module* module = (struct module*)_module;
   struct snd_card** ppcard = (struct snd_card**)_ppcard;
   struct snd_card* pcard = 0;
   struct rgb133_dev* dev = (struct rgb133_dev*)_dev;

// Here, we handle various functions for creating sound card with different kernels
#ifdef RGB133_CONFIG_HAVE_SND_CARD_NEW_4_PARS
   pcard = snd_card_new(index, id, module, extra_size);
   // Unlike snd_card_create(), this version of snd_card_new() does not return int, so we do it ourselves
   if (pcard == 0)
   {
      *ppcard = 0;
      return -1;
   }
   else
   {
      *ppcard = pcard;
      return 0;
   }
#else
#ifdef RGB133_CONFIG_HAVE_SND_CARD_NEW_6_PARS
   // new version of snd_card_new() requires passing a pointer to parent pci device
   return snd_card_new(&dev->core.pci->dev, index, id, module, extra_size, ppcard);
#else
   return snd_card_create(index, id, module, extra_size, ppcard);
#endif
#endif
}

int SndCardFree(void* card)
{
   if (!card)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "SndCardFree: sound card does not exist (card == %p), return\n", card));
      return -1;
   }
   else
   {
      snd_card_free((struct snd_card*)card);
      return 0;
   }
}

// This function acts similarly to SndCardFree() but it tends to call snd_card_free_when_closed() over snd_card_free;
// it helps in cases when alsa card is held so that we would wait in snd_card_free(), not being able to unload the driver;
// with snd_card_free_when_closed(), card is disconnected and card's resources are freed later
int SndCardRemove(void* card)
{
   if (!card)
   {
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "SndCardRemove: sound card does not exist (card == %p), return\n", card));
      return -1;
   }
   else
   {
#ifdef RGB133_CONFIG_HAVE_SND_CARD_FREE_WHEN_CLOSED
      return snd_card_free_when_closed((struct snd_card*)card);
#else
      snd_card_free((struct snd_card*)card);
      return 0;
#endif
   }
}

void SndCardSetNames(void* _card, const char* name, const char* audio_type, unsigned int node_nr)
{
   struct snd_card* card = (struct snd_card*)_card;
   char node_str[3] = {0};

   strlcpy(card->driver, DRIVER_TAG, sizeof(card->driver));
   strlcat(card->driver, " Driver", sizeof(card->driver));
   strlcpy(card->shortname, name, sizeof(card->shortname));
   strlcat(card->shortname, audio_type, sizeof(card->shortname));
   strlcpy(card->longname, name, sizeof(card->longname));
   strlcat(card->longname, audio_type, sizeof(card->longname));
   strlcat(card->longname, " Node:", sizeof(card->longname));
   snprintf(node_str, sizeof(node_str), "%d", node_nr);
   strlcat(card->longname, node_str, sizeof(card->longname));

   RGB133PRINT((RGB133_LINUX_DBG_LOG, "SndCardSetNames: Assigned card(0x%p) shortname(%s) longname(%s)\n\n",
         card, card->shortname, card->longname));
}

int AudioUtilsGetSize_rgb133_audio_data(void)
{
   return sizeof(struct rgb133_audio_data);
}

int AudioUtilsGetSize_rgb133_pcm_chip(void)
{
   return sizeof(struct rgb133_pcm_chip);
}

void* AudioUtilsGetPtr_rgb133_audio_data(void* _card)
{
   struct snd_card* card = (struct snd_card*)_card;
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)&card[1];

   return (void*)linux_audio;
}

void AudioUtilsSetDevice(void* ptr, void* _rgb133)
{
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)ptr;
   struct rgb133_dev* rgb133 = (struct rgb133_dev*)_rgb133;

   linux_audio->rgb133_device = rgb133;
}

void AudioUtilsSetStructAudioChannel(void* ptr, void* value)
{
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)ptr;

   linux_audio->pAudio = value;
}

void AudioUtilsSetCard(void* ptr, void* _card)
{
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)ptr;
   struct snd_card* card = (struct snd_card*)_card;

   linux_audio->card = card;
}

void* AudioUtilsGetPtrPcmChip(void* ptr)
{
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)ptr;
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip *)(((unsigned char *)linux_audio) + sizeof(struct rgb133_audio_data));

   return (void*)chip;
}

void AudioUtilsSetPcmChip(void* ptr, void* value)
{
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)ptr;
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)value;

   linux_audio->pcm_chip = chip;
}

void AudioUtilsSetLinuxAudio(void* ptr, void* value)
{
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)ptr;
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)value;

   chip->linux_audio = linux_audio;
}

void AudioUtilsSetCardtoPcmChip(void* ptr, void* _card)
{
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)ptr;
   struct snd_card* card = (struct snd_card*)_card;

   chip->card = card;
}

void AudioUtilsSetCardtorgb133dev(void* _rgb133, void* _card, int channel)
{
   struct snd_card* card = (struct snd_card*)_card;
   struct rgb133_dev* rgb133 = (struct rgb133_dev*)_rgb133;

   rgb133->snd_card[channel] = card;
}

void AudioUtilsGetCardFromrgb133dev(void** _pcard, void* _rgb133, int channel)
{
   struct snd_card** pcard = (struct snd_card**)_pcard;
   struct rgb133_dev* rgb133 = (struct rgb133_dev*)_rgb133;

   *pcard = rgb133->snd_card[channel];
}

void AudioUtilsSetInitSuccess(void* ptr, int init_success)
{
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)ptr;

   chip->init_success = init_success;
}

void AudioUtilsSetChannel(void* ptr, int channel)
{
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)ptr;

   chip->index = channel;
}

void AudioUtilsSetPDE(void* ptr, void* value)
{
   struct rgb133_audio_data* linux_audio = (struct rgb133_audio_data*)ptr;
   struct rgb133_dev* rgb133_device = linux_audio->rgb133_device;
   PDEAPI pDE = (PDEAPI)value;

   rgb133_device->pDE = pDE;
}

void AudioUtilsSethandleAudioCapture(void* ptr, unsigned int handle)
{
   struct rgb133_pcm_chip *chip = (struct rgb133_pcm_chip*)ptr;

   chip->hndlAudioCapture = handle;
}

int PcmCreateNew(void* ptr)
{
   return rgb133_create_new_pcm((struct rgb133_pcm_chip*)ptr);
}

int SndCardMixerCreateNew(void* ptr)
{
   return rgb133_snd_mixer_create_new((struct rgb133_pcm_chip*)ptr);
}

int SndCardRegister(void* card)
{
   return snd_card_register((struct snd_card*)card);
}

char* SndCardGetShortName(void* _card)
{
   struct snd_card* card = (struct snd_card*)_card;

   return card->shortname;
}

void SndPcmNewDataNotify(void* _chip)
{
   struct rgb133_pcm_chip * pcm_chip = (struct rgb133_pcm_chip *)_chip;

   // Note that without the check below, there could be occasional crashes when calling snd_pcm_period_elapsed on closed substream
   // ( we assign substream to pcm_chip on .open() and clear that field at .close() ).
   if (pcm_chip->substream)
      snd_pcm_period_elapsed(pcm_chip->substream);
}

void AudioUtilsStoreConfigToChip(void* _chip, struct rgb133_audio_config config)
{
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)_chip;

   chip->linux_audio_config.digital_mute = config.digital_mute;
   chip->linux_audio_config.analog_mute = config.analog_mute;
   chip->linux_audio_config.analog_min_gain_unbalanced = config.analog_min_gain_unbalanced;
   chip->linux_audio_config.analog_cur_gain_unbalanced = config.analog_cur_gain_unbalanced;
   chip->linux_audio_config.analog_max_gain_unbalanced = config.analog_max_gain_unbalanced;
   chip->linux_audio_config.analog_min_gain_balanced = config.analog_min_gain_balanced;
   chip->linux_audio_config.analog_cur_gain_balanced = config.analog_cur_gain_balanced;
   chip->linux_audio_config.analog_max_gain_balanced = config.analog_max_gain_balanced;
   chip->linux_audio_config.sdi_mute = config.sdi_mute;
}

void AudioUtilsSetAudioTimer(PHANDLE ppThread, void* _chip)
{
   struct rgb133_pcm_chip* chip = (struct rgb133_pcm_chip*)_chip;
   RGB133AUDIODATA* linux_audio;

   if(!_chip)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "AudioUtilsSetAudioThread: Failed to set audio thread; pcm chip is NULL\n"));
      return;
   }

   linux_audio = chip->linux_audio;
   if(!linux_audio)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "AudioUtilsSetAudioThread: Failed to set audio thread; linux_audio is NULL\n"));
      return;
   }

   *ppThread = (HANDLE)&linux_audio->rgb133_audio_timer;
}

PAUDIOCHANNELAPI AudioUtilsGetAudioChannelPtr(void* arg)
{
   PAUDIOCHANNELAPI pAudio = NULL;
   RGB133AUDIODATA* linux_audio;

   if(!arg)
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "AudioUtilsGetAudioChannelPtr: Failed to get audio channel; null argument\n"));
      return NULL;
   }

#ifdef RGB133_CONFIG_HAVE_NEW_TIMERS_API
   linux_audio = container_of((struct timer_list*)arg, RGB133AUDIODATA, rgb133_audio_timer);
   pAudio = linux_audio->pAudio;
#else
   pAudio = arg;
#endif

   return pAudio;
}
