/*
 * rgb133audio.c
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
#include "rgb133kernel.h"
#include "rgb133debug.h"

#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include "rgb133.h"
#include "rgb133audioapi.h"
#define IN_RGB133AUDIO_C
#include "rgb133audio.h"

#define byte_pos(x)	((x) / HZ)
#define frac_pos(x)	((x) * HZ)

// maximum size of the buffer in bytes (matches the size of CommonBuffer to which audio is DMAed)
#define BUF_MAX 32768

static struct snd_pcm_hardware rgb133_mychip_capture_hw = {
   .info =            (SNDRV_PCM_INFO_INTERLEAVED |
                       SNDRV_PCM_INFO_BLOCK_TRANSFER |
                       SNDRV_PCM_INFO_MMAP_VALID),
   .formats =          SNDRV_PCM_FMTBIT_S16,
   .rates =           (SNDRV_PCM_RATE_32000 |     // samples per second
                       SNDRV_PCM_RATE_44100 |
                       SNDRV_PCM_RATE_48000 |
                       SNDRV_PCM_RATE_96000),
   .rate_min =         32000,
   .rate_max =         96000,
   .channels_min =     2,
   .channels_max =     2,
   .buffer_bytes_max = BUF_MAX,
   .period_bytes_min = 16384,
   .period_bytes_max = BUF_MAX,
   .periods_min =      2,
   .periods_max =     (BUF_MAX/16384),
};

struct pcm_stream_private_data {
   struct rgb133_pcm_chip * chip;
};

struct substream_private_data {
   unsigned long running;
};

struct snd_pcm_ops snd_rgb133_pcm_capture_ops = {
   .open =        rgb133_mychip_capture_open,
   .close =       rgb133_mychip_capture_close,
   .ioctl =       snd_pcm_lib_ioctl,
   .hw_params =   rgb133_mychip_pcm_hw_params,
   .hw_free =     rgb133_mychip_pcm_hw_free,
   .prepare =     rgb133_mychip_pcm_prepare,
   .trigger =     rgb133_mychip_pcm_trigger,
#ifdef RGB133_CONFIG_HAVE_SND_PCM_SGBUF_OPS_PAGE
   .page    =     snd_pcm_sgbuf_ops_page,
#else /* !RGB133_CONFIG_HAVE_SND_PCM_SGBUF_OPS_PAGE */
   /* The helper is no longer referred after the recent code refactoring in the kernel and in alsa libs */
   .page    =     NULL,
#endif /* RGB133_CONFIG_HAVE_SND_PCM_SGBUF_OPS_PAGE */
   .pointer =     rgb133_mychip_pcm_pointer,
};

int rgb133_create_new_pcm(struct rgb133_pcm_chip * chip)
{
   struct rgb133_dev* dev = chip->linux_audio->rgb133_device;
   struct snd_pcm *pcm;
   int err;

   ///// PG: Could we set device number as channel number? !!!
   err = snd_pcm_new(chip->card, "Audio Capture", 0, 0, 1, &pcm); // no playback, one capture.
   if (err < 0)
   {
      return err;
   }

   pcm->private_data = chip;
   strcpy(pcm->name, chip->card->longname);
   // By this user apps can filter out non-Datapath devices
   strcat(chip->card->longname, " DGC dada");
   chip->pcm = pcm;
   pcm->info_flags = 0;
   pcm->dev_subclass = SNDRV_PCM_SUBCLASS_GENERIC_MIX;

   snd_pcm_set_ops(chip->pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_rgb133_pcm_capture_ops);

#ifdef RGB133_CONFIG_HAVE_RETURN_FROM_SND_PCM_PREALLOCATE_PAGES
   err = snd_pcm_lib_preallocate_pages_for_all(pcm,
                                               SNDRV_DMA_TYPE_DEV_SG,
                                               &dev->core.pci->dev,
                                               64*1024,
                                               64*1024);

   if (err < 0)
   {
      RGB133PRINT((RGB133_LINUX_DBG_INIT, "snd_pcm_lib_preallocate_pages_for_all failed: %d\n", err));
      return err;
   }
#else
   snd_pcm_lib_preallocate_pages_for_all(pcm,
                                         SNDRV_DMA_TYPE_DEV_SG,
                                         &dev->core.pci->dev,
                                         64*1024,
                                         64*1024);
#endif

   chip->period_size = 0; // DMJ TODO: Manifestly wrong.
   chip->init_success = 1;

   return 0;
}

/* open pcm callback */
static int rgb133_mychip_capture_open(struct snd_pcm_substream *substream)
{
   struct rgb133_pcm_chip *chip = snd_pcm_substream_chip(substream);
   struct rgb133_dev* dev = chip->linux_audio->rgb133_device;
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct substream_private_data * substr_pd;

   PDEAPI pDE;
   int ret;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_mychip_capture_open!\n"));

   // First, complete any Audio initialisation required for this channel
   if(dev->audioInit[chip->index] == 0)
   {
      if((ret = LinuxAudioInitialisationPost((PDEAPI)chip->linux_audio->rgb133_device->pDE, chip->index)) != 0)
      {
         RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mychip_capture_open: Failed to initialise Audio for channel(%d)\n",
            chip->index));
         return -ENODEV;
      }
      dev->audioInit[chip->index] = 1;
   }

   substream->private_data = KernelVzalloc(sizeof(struct substream_private_data));
   if (substream->private_data == NULL)
   {
      return -ENOMEM;
   }
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_mychip_capture_open: alloc %d bytes to substream(0x%p)->private_data(0x%p)\n",
         sizeof(struct substream_private_data), substream, substream->private_data));

   substr_pd = (struct substream_private_data *)substream->private_data;
   
   substr_pd->running = 0;

   runtime->hw = rgb133_mychip_capture_hw;
   runtime->private_data = KernelVzalloc(sizeof(struct pcm_stream_private_data));
   RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_mychip_capture_open: alloc %d bytes to runtime(0x%p)->private_data(0x%p)\n",
         sizeof(struct pcm_stream_private_data), runtime, runtime->private_data));

   ((struct pcm_stream_private_data*)(runtime->private_data))->chip = chip; // Hook the chip back through.
   /* more hardware-initialization will be done here */
   /* Like setting up the interrupt for the data reception. */

   // step into capture open on the core side
   pDE = chip->linux_audio->rgb133_device->pDE;

   if ((ret = AudioDGCAudioOpenInput(pDE, (void*)chip, chip->index)))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mychip_capture_open: AudioDGCAudioOpenInput failed(%d)\n", ret));
      return ret;
   }

   // Hook the opened substream to our custom chip structure;
   // This actually triggers capture start in blocking mode as core driver only notifies of new data if chip->substream exists.
   chip->substream = substream;

   return 0;
}

/* close callback */
static int rgb133_mychip_capture_close(struct snd_pcm_substream *substream)
{
   // !!! Chip should not be acquired from substream->private_data but from substream->runtime->private_data;
   // It's because it gets reassigned in the .open callback
   //struct rgb133_pcm_chip *chip = snd_pcm_substream_chip(substream);
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct pcm_stream_private_data * str_pd = runtime->private_data;
   struct rgb133_pcm_chip *chip = str_pd->chip;
   PDEAPI pDE;
   int ret;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_mychip_capture_close!\n"));

   // Clear our custom chip structure of what we assigned to it at .open();
   // In blocking mode, this makes core driver stop sending notifications of new data.
   chip->substream = 0;

   // step into capture close on the core side
   pDE = chip->linux_audio->rgb133_device->pDE;

   if((ret = AudioDGCAudioCloseInput(pDE, chip->index, chip->hndlAudioCapture)))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mychip_capture_close: AudioDGCAudioCloseInput failed\n"));
      return ret;
   }

   if (substream->runtime->private_data != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_mychip_capture_close: free substream->runtime->private_data(0x%p)\n",
            substream->runtime->private_data));
      KernelVfree(substream->runtime->private_data);
   }
   if (substream->private_data != NULL)
   {
      RGB133PRINT((RGB133_LINUX_DBG_MEM, "rgb133_mychip_capture_close: free substream->private_data(0x%p)\n",
            substream->private_data));
      KernelVfree(substream->private_data);
   }

   return 0;
}

/* hw_params callback */
static int rgb133_mychip_pcm_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *hw_params)
{
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct pcm_stream_private_data * str_pd = runtime->private_data;
   struct rgb133_pcm_chip *chip = str_pd->chip;
   struct rgb133_audio_data * linux_audio = chip->linux_audio;
   void* pAudio = linux_audio->pAudio;
   PDEAPI pDE;
   int bps, ret;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_mychip_pcm_hw_params!\n"));

   // ALSA is not allocating DMA memory for us
   // It is allocated by the core driver so here we are only assigning the core's DMA buffer to our ALSA DMA buffer
   // Note that buffer sizes in hw_params must match the size of CommonBuffer (32768)
   runtime->dma_area = AudioGetDmaBuffer(pAudio);

   /* Extract the current configuration and set up the hardware */
   runtime->format = params_format(hw_params);
   runtime->rate = params_rate(hw_params);
   runtime->channels = params_channels(hw_params);

   // Set: rate, format, and number of channels to hardware
   // note that channels is the number of channels (that is: mono, stereo, ...) and chip->index is channel as analog or digital
   pDE = chip->linux_audio->rgb133_device->pDE;
   if ((ret = AudioDGCAudioSetCaps(pDE, runtime->format, runtime->rate, runtime->channels, chip->index)))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mychip_pcm_hw_params: AudioDGCAudioSetCaps failed(%d)\n", ret));
      return -EPERM;
   }

   return 0;
}

/* hw_free callback */
static int rgb133_mychip_pcm_hw_free(struct snd_pcm_substream *substream)
{
   struct snd_pcm_runtime *runtime = substream->runtime;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_mychip_pcm_hw_free!\n"));

   // DMA memory is managed by the core driver;
   // We do not have to deallocate it here. Just NULL the pointer so that it will scream if the pointer is used later
   runtime->dma_area = NULL;

   return 0;
}

/* prepare callback */
static int rgb133_mychip_pcm_prepare(struct snd_pcm_substream *substream)
{
   // Chip should not be acquired from substream->private_data but from substream->runtime->private_data;
   // It's because it gets reassigned in the .open callback!
   ////struct rgb133_pcm_chip *chip = snd_pcm_substream_chip(substream);
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct pcm_stream_private_data * str_pd = runtime->private_data;
   struct rgb133_pcm_chip *chip = str_pd->chip;
   PDEAPI pDE;
   int bps, ret;

   /* set up the hardware with the current configuration */
   // We may use rgb133_set_dma_setup in the future so let's not delete it
   rgb133_set_dma_setup(chip, runtime->dma_addr, chip->buffer_size, chip->period_size);
   // Set: rate, format, and number of channels to hardware
   pDE = chip->linux_audio->rgb133_device->pDE;
   if ((ret = AudioDGCAudioSetCaps(pDE, runtime->format, runtime->rate, runtime->channels, chip->index)))
   {
      RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_mychip_pcm_prepare: AudioDGCAudioSetCaps failed(%d)\n", ret));
      return -EPERM;
   }

   // Clear buffer.
   /// 0 is a debug value but as good as any other probably...
	memset(runtime->dma_area, 0, frames_to_bytes(runtime, runtime->buffer_size));

	bps = runtime->rate * runtime->channels; // params requested by user app (arecord, audacity)
	bps *= snd_pcm_format_width(runtime->format);
	bps /= 8;
	if (bps <= 0)
		return -EINVAL;

	RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_mychip_pcm_prepare:"
	      " bps: %u; runtime->buffer_size: %lu; runtime->period_size: %lu; runtime->rate: %u",
	      bps, runtime->buffer_size, runtime->period_size, runtime->rate));

   return 0;
}

/* trigger callback */
static int rgb133_mychip_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
   int ret = 0;
   struct substream_private_data * substr_pd = substream->private_data;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_mychip_pcm_trigger!\n"));

   switch (cmd) {
      case SNDRV_PCM_TRIGGER_START:
         /* start the PCM engine */
         if (!substr_pd->running)
         {
            // START the DMA here
            // This is optional for us - the way core driver works is it starts DMAing when the driver loads
            // Would we want to start/stop DMAing from inside the application, we can implement it in the future
         }
         substr_pd->running |= (1 << substream->stream);
         break;
      case SNDRV_PCM_TRIGGER_STOP:
         /* stop the PCM engine */
         substr_pd->running &= ~(1 << substream->stream);
         if (!substr_pd->running)
         {
            // STOP the DMA here
         }
         break;
#if 0
      /* DMJ: TODO: Support pause? */
      case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
         break;
      case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
         break;
#endif
      default:
         ret = -EINVAL;
   }

   return ret;
}

unsigned int rgb133_mychip_get_hw_pointer(struct rgb133_pcm_chip * chip)
{
   struct rgb133_audio_data * linux_audio = chip->linux_audio;
   void* pAudio = linux_audio->pAudio;

   return AudioGetDmaWritePointer(pAudio);
}

/* pointer callback */
static snd_pcm_uframes_t rgb133_mychip_pcm_pointer(struct snd_pcm_substream *substream)
{
   ////struct rgb133_pcm_chip *chip = snd_pcm_substream_chip(substream);
   // Chip should be acquired not from substream->private_data but from substream->runtime->private_data;
   // It's because it gets reassigned in the .open callback!
   struct snd_pcm_runtime *runtime = substream->runtime;
   struct pcm_stream_private_data * str_pd = runtime->private_data;
   struct rgb133_pcm_chip *chip = str_pd->chip;
   unsigned int current_ptr, last_ptr;

   /* get the current DMA pointer position from core driver */
   current_ptr = rgb133_mychip_get_hw_pointer(chip);

   return bytes_to_frames(runtime, current_ptr);
}

static int rgb133_set_dma_setup(struct rgb133_pcm_chip *chip, dma_addr_t dma_addr, snd_pcm_uframes_t buffer_size, snd_pcm_uframes_t period_size)
{
   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "In rgb133_set_dma_setup!\n"));
   return 0;
}
