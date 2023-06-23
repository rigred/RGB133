/*
 * rgb133audiocontrol.c
 *
 * Copyright (c) 2015 Datapath Limited All rights reserved.
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
#include <linux/wait.h>
#include <sound/asound.h>
#include <sound/control.h>
#ifdef RGB133_CONFIG_HAVE_SNDRV_CARDS_IN_DRIVER
#include <sound/driver.h>
#endif
#include <sound/core.h>

#include "rgb_linux_types.h"
#include "rgb_api_types.h"
#include "rgb133audioapi.h"
#include "rgb133audio.h"
#include "rgb133audiocontrol.h"
#include "rgb133kernel.h"
#include "rgb133debug.h"
#include "rgb133.h"

/* Templates for initialising audio controls for digital channel, SDI and analog (balanced and unbalanced) */
static struct snd_kcontrol_new rgb133_snd_controls_digital[] = {
   // Digital controls
   {
      .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
      .name = "Digital_Mute Capture Route",
      .index = 0,
      .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
      .private_value = ALSA_CONTROL_INDEX_DIG_MUTE,
      .info = rgb133_snd_control_mute_info,
      .get = rgb133_snd_control_mute_get,
      .put = rgb133_snd_control_mute_put,
   }
};

static struct snd_kcontrol_new rgb133_snd_controls_sdi[] = {
   // SDI controls
   {
      .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
      .name = "SDI_Mute Capture Route",
      .index = 0,
      .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
      .private_value = ALSA_CONTROL_INDEX_SDI_MUTE,
      .info = rgb133_snd_control_mute_info,
      .get = rgb133_snd_control_mute_get,
      .put = rgb133_snd_control_mute_put,
   }
};

static struct snd_kcontrol_new rgb133_snd_controls_analog[] = {
   // Common analog controls
   {
      .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
      .name = "Analog_Mute Capture Route",
      .index = 0,
      .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
      .private_value = ALSA_CONTROL_INDEX_ANA_MUTE,
      .info = rgb133_snd_control_mute_info,
      .get = rgb133_snd_control_mute_get,
      .put = rgb133_snd_control_mute_put,
   },
   // Unbalanced analog controls
   {
      .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
      .name = "Analog_Unbalanced_Vol Capture Volume",
      .index = 0,
      .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
      .private_value = ALSA_CONTROL_INDEX_ANA_UNBAL_VOLUME,
      .info = rgb133_snd_control_volume_info,
      .get = rgb133_snd_control_volume_get,
      .put = rgb133_snd_control_volume_put,
   },
   // Balanced analog controls
   {
      .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
      .name = "Analog_Balanced_Vol Capture Volume",
      .index = 0,
      .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
      .private_value = ALSA_CONTROL_INDEX_ANA_BAL_VOLUME,
      .info = rgb133_snd_control_volume_info,
      .get = rgb133_snd_control_volume_get,
      .put = rgb133_snd_control_volume_put,
   }
};

int rgb133_snd_mixer_create_new(struct rgb133_pcm_chip* chip)
{
   struct snd_card *card = chip->card;
   struct snd_kcontrol *control;
   int channel_type = AudioGetChannelType(chip->linux_audio->pAudio);
   int capture_type = AudioGetCaptureType(chip->linux_audio->pAudio);
   int i, error;

   strcpy(card->mixername, DRIVER_NAME);

   /* Create all controls and assign them to sound card */
   if((channel_type == LINUXAUDIOCAPTURESOURCE_HDMI) ||
      (channel_type == LINUXAUDIOCAPTURESOURCE_HDMI_OR_ANALOGUE && capture_type == LINUXAUDIOCAPTURETYPE_EMBEDDED))
   {
      // digital controls
      for(i=0; i<ARRAY_SIZE(rgb133_snd_controls_digital); i++)
      {
         control = snd_ctl_new1(&rgb133_snd_controls_digital[i], chip);
         if(!control)
            return -ENOMEM;

         //PG: at the moment, we are always creating one device per card and we allow a single substream to be opened at a time
         control->id.device = 0;
         control->id.subdevice = 0;

         error = snd_ctl_add(card, control);
         if(error < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_snd_mixer_create_new(1): snd_ctl_add failed with error %d\n", error));
            return error;
         }
      }
   }
   else if((channel_type == LINUXAUDIOCAPTURESOURCE_ANALOGUE) ||
           (channel_type == LINUXAUDIOCAPTURESOURCE_HDMI_OR_ANALOGUE && capture_type == LINUXAUDIOCAPTURETYPE_EXTERNAL) ||
           (channel_type == LINUXAUDIOCAPTURESOURCE_SDI_OR_ANALOGUE && capture_type == LINUXAUDIOCAPTURETYPE_EXTERNAL))
   {
      // analog controls
      for(i=0; i<ARRAY_SIZE(rgb133_snd_controls_analog); i++)
      {
         control = snd_ctl_new1(&rgb133_snd_controls_analog[i], chip);
         if(!control)
            return -ENOMEM;

         //PG: at the moment, we are always creating one device per card and we allow a single substream to be opened at a time
         control->id.device = 0;
         control->id.subdevice = 0;

         error = snd_ctl_add(card, control);
         if(error < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_snd_mixer_create_new(2): snd_ctl_add failed with error %d\n", error));
            return error;
         }
      }
   }
   else if((channel_type == LINUXAUDIOCAPTURESOURCE_SDI) ||
           (channel_type == LINUXAUDIOCAPTURESOURCE_SDI_OR_ANALOGUE && capture_type == LINUXAUDIOCAPTURETYPE_EMBEDDED))
   {
      // SDI audio controls
      for(i=0; i<ARRAY_SIZE(rgb133_snd_controls_sdi); i++)
      {
         control = snd_ctl_new1(&rgb133_snd_controls_sdi[i], chip);
         if(!control)
            return -ENOMEM;

         //PG: at the moment, we are always creating one device per card and we allow a single substream to be opened at a time
         control->id.device = 0;
         control->id.subdevice = 0;

         error = snd_ctl_add(card, control);
         if(error < 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_INIT, "rgb133_snd_mixer_create_new(3): snd_ctl_add failed for SDI control with error %d\n", error));
            return error;
         }
      }
   }
   else
      return -EINVAL;

   RGB133PRINT((RGB133_LINUX_DBG_INOUT, "rgb133_snd_mixer_create_new: Created new mixer and added controls to card(0x%p)\n", card));

   return 0;
}

/* Control interface callbacks */
int rgb133_snd_control_mute_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *control_info)
{
   switch(kcontrol->private_value)
   {
      case ALSA_CONTROL_INDEX_DIG_MUTE:
      case ALSA_CONTROL_INDEX_SDI_MUTE:
      {
         // these match mute states definitions for digital in rgb133audioapi.h
         static const char* texts[2] = {"MUTE OFF", "MUTE ON"};

         // fill struct snd_ctl_elem_info with all needed information
         control_info->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
         control_info->count = ALSA_CONTROL_ELEMENTS_1;
         control_info->value.enumerated.items = ALSA_CONTROL_ENUM_TYPE_2_STATES;
         if (control_info->value.enumerated.item > ALSA_CONTROL_ENUM_TYPE_2_STATES - 1)
            control_info->value.enumerated.item = ALSA_CONTROL_ENUM_TYPE_2_STATES - 1;
         strcpy(control_info->value.enumerated.name,
         texts[control_info->value.enumerated.item]);
         break;
      }
      case ALSA_CONTROL_INDEX_ANA_MUTE:
      {
         // these match mute states definitions for analog in rgb133audioapi.h
         static const char* texts[4] = {"MUTE NONE", "MUTE UNBALANCED", "MUTE BALANCED", "MUTE BOTH"};

         // fill struct snd_ctl_elem_info with all needed information
         control_info->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
         control_info->count = ALSA_CONTROL_ELEMENTS_1;
         control_info->value.enumerated.items = ALSA_CONTROL_ENUM_TYPE_4_STATES;
         if (control_info->value.enumerated.item > ALSA_CONTROL_ENUM_TYPE_4_STATES - 1)
            control_info->value.enumerated.item = ALSA_CONTROL_ENUM_TYPE_4_STATES - 1;
         strcpy(control_info->value.enumerated.name,
         texts[control_info->value.enumerated.item]);
         break;
      }
      default:
         return -EINVAL;
   }

   return 0;
}

int rgb133_snd_control_volume_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *control_info)
{
   struct rgb133_pcm_chip *chip = snd_kcontrol_chip(kcontrol);

   control_info->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
   control_info->count = ALSA_CONTROL_ELEMENTS_1;

   switch(kcontrol->private_value)
   {
   case ALSA_CONTROL_INDEX_ANA_UNBAL_VOLUME:
      control_info->value.integer.min = chip->linux_audio_config.analog_min_gain_unbalanced;
      control_info->value.integer.max = chip->linux_audio_config.analog_max_gain_unbalanced;
      break;
   case ALSA_CONTROL_INDEX_ANA_BAL_VOLUME:
      control_info->value.integer.min = chip->linux_audio_config.analog_min_gain_balanced;
      control_info->value.integer.max = chip->linux_audio_config.analog_max_gain_balanced;
      break;
   default:
      return -EINVAL;
   }

   return 0;
}

int rgb133_snd_control_mute_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value)
{
   struct rgb133_pcm_chip *chip = snd_kcontrol_chip(kcontrol);

   switch(kcontrol->private_value)
   {
   case ALSA_CONTROL_INDEX_DIG_MUTE:
      control_value->value.enumerated.item[0] = chip->linux_audio_config.digital_mute;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_mute_get: got digital mute(%d)\n",
            control_value->value.enumerated.item[0]));
      break;
   case ALSA_CONTROL_INDEX_ANA_MUTE:
      control_value->value.enumerated.item[0] = chip->linux_audio_config.analog_mute;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_mute_get: got analog mute(%d)\n",
            control_value->value.enumerated.item[0]));
      break;
   case ALSA_CONTROL_INDEX_SDI_MUTE:
      control_value->value.enumerated.item[0] = chip->linux_audio_config.sdi_mute;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_mute_get: got sdi mute(%d)\n",
            control_value->value.enumerated.item[0]));
      break;
   default:
      return -EINVAL;
   }

   return 0;
}

int rgb133_snd_control_mute_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value)
{
   struct rgb133_pcm_chip *chip = snd_kcontrol_chip(kcontrol);
   PDEAPI pDE = chip->linux_audio->rgb133_device->pDE;
   int changed = 0;
   int rc;

   switch(kcontrol->private_value)
   {
   case ALSA_CONTROL_INDEX_DIG_MUTE:
      if(chip->linux_audio_config.digital_mute != control_value->value.enumerated.item[0])
      {
         if((rc = AudioSetControlMute(pDE, chip->index, control_value->value.enumerated.item[0])) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_snd_control_mute_put: Failed to set mute control for digital channel(%d)\n", chip->index));
         }
         else
         {
            // successfully set; save to our struct with current config
            chip->linux_audio_config.digital_mute = control_value->value.enumerated.item[0];
            changed = 1;
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_mute_put: set digital mute(%d)\n",
                  chip->linux_audio_config.digital_mute));
         }
      }
      break;
   case ALSA_CONTROL_INDEX_ANA_MUTE:
      if(chip->linux_audio_config.analog_mute != control_value->value.enumerated.item[0])
      {
         if((rc = AudioSetControlMute(pDE, chip->index, control_value->value.enumerated.item[0])) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_snd_control_mute_put: Failed to set mute control for analog channel(%d)\n", chip->index));
         }
         else
         {
            // successfully set; save to our struct with current config
            chip->linux_audio_config.analog_mute = control_value->value.enumerated.item[0];
            changed = 1;
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_mute_put: set analog mute(%d)\n",
                  chip->linux_audio_config.analog_mute));
         }
      }
      break;
   case ALSA_CONTROL_INDEX_SDI_MUTE:
      if(chip->linux_audio_config.sdi_mute != control_value->value.enumerated.item[0])
      {
         if((rc = AudioSetControlMute(pDE, chip->index, control_value->value.enumerated.item[0])) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_snd_control_mute_put: Failed to set mute control for SDI channel(%d)\n", chip->index));
         }
         else
         {
            // successfully set; save to our struct with current config
            chip->linux_audio_config.sdi_mute = control_value->value.enumerated.item[0];
            changed = 1;
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_mute_put: set sdi mute(%d)\n",
                  chip->linux_audio_config.sdi_mute));
         }
      }
      break;
   default:
      return -EINVAL;
   }

   return changed;
}

int rgb133_snd_control_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value)
{
   struct rgb133_pcm_chip *chip = snd_kcontrol_chip(kcontrol);

   switch(kcontrol->private_value)
   {
   case ALSA_CONTROL_INDEX_ANA_UNBAL_VOLUME:
      control_value->value.integer.value[0] = chip->linux_audio_config.analog_cur_gain_unbalanced;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_volume_get: got unbalanced gain(%d)\n",
            control_value->value.integer.value[0]));
      break;
   case ALSA_CONTROL_INDEX_ANA_BAL_VOLUME:
      control_value->value.integer.value[0] = chip->linux_audio_config.analog_cur_gain_balanced;
      RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_volume_get: got balanced gain(%d)\n",
            control_value->value.integer.value[0]));
      break;
   default:
      return -EINVAL;
   }

   return 0;
}

int rgb133_snd_control_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value)
{
   struct rgb133_pcm_chip *chip = snd_kcontrol_chip(kcontrol);
   PDEAPI pDE = chip->linux_audio->rgb133_device->pDE;
   int changed = 0;
   int rc;

   switch(kcontrol->private_value)
   {
   case ALSA_CONTROL_INDEX_ANA_UNBAL_VOLUME:
      if(chip->linux_audio_config.analog_cur_gain_unbalanced != control_value->value.integer.value[0])
      {
         if ((rc = AudioSetControlVolume(pDE, chip->index, LINUXAUDIOCAPTURESOURCE_ANALOG_UNBALANCED, control_value->value.integer.value[0])) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_snd_control_volume_put: Failed to set volume for unbalanced analog channel(%d)\n", chip->index));
         }
         else
         {
            // successfully set; save to our struct with current config
            chip->linux_audio_config.analog_cur_gain_unbalanced = control_value->value.integer.value[0];
            changed = 1;
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_volume_put: set unbalanced gain(%d)\n",
                  chip->linux_audio_config.analog_cur_gain_unbalanced));
         }
      }
      break;
   case ALSA_CONTROL_INDEX_ANA_BAL_VOLUME:
      if(chip->linux_audio_config.analog_cur_gain_balanced != control_value->value.integer.value[0])
      {
         if ((rc = AudioSetControlVolume(pDE, chip->index, LINUXAUDIOCAPTURESOURCE_ANALOG_BALANCED, control_value->value.integer.value[0])) != 0)
         {
            RGB133PRINT((RGB133_LINUX_DBG_ERROR, "rgb133_snd_control_volume_put: Failed to set volume for balanced analog channel(%d)\n", chip->index));
         }
         else
         {
            // successfully set; save to our struct with current config
            chip->linux_audio_config.analog_cur_gain_balanced = control_value->value.integer.value[0];
            changed = 1;
            RGB133PRINT((RGB133_LINUX_DBG_LOG, "rgb133_snd_control_volume_put: set balanced gain(%d)\n",
                  chip->linux_audio_config.analog_cur_gain_balanced));
         }
      }
      break;
   default:
      return -EINVAL;
   }

   return 0;
}
