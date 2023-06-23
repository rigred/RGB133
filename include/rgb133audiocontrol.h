/*
 * rgb133audiocontrol.h
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

#ifndef _RGB133AUDIOCONTROL_H
#define _RGB133AUDIOCONTROL_H

int rgb133_snd_mixer_create_new(struct rgb133_pcm_chip* chip);

/* Control interface callbacks declarations */
int rgb133_snd_control_mute_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *control_info);
int rgb133_snd_control_volume_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *control_info);
int rgb133_snd_control_mute_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value);
int rgb133_snd_control_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value);
int rgb133_snd_control_mute_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value);
int rgb133_snd_control_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *control_value);

#endif /* _RGB133AUDIOCONTROL_H */
