/*
 * rgb133sndcard.h
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

#ifndef _RGB133SNDCARD_H
#define _RGB133SNDCARD_H

#include "rgb_api_types.h"

int SndCardGetSize(void);
int SndCardBuild(PRGB133DEVAPI _dev, int index, const char* id, PMODULEAPI _module, int extra_size, void** _ppcard);
int SndCardFree(void* card);
int SndCardRemove(void* card);
void SndCardSetNames(void* _card, const char* name, const char* audio_type, unsigned int node_nr);
int SndCardRegister(void* card);
char* SndCardGetShortName(void* ptr);

void SndPcmNewDataNotify(void* _chip);

int AudioUtilsGetSize_rgb133_audio_data(void);
int AudioUtilsGetSize_rgb133_pcm_chip(void);
void* AudioUtilsGetPtr_rgb133_audio_data(void* card);
void AudioUtilsSetDevice(void* ptr, void* _rgb133);
void AudioUtilsSetStructAudioChannel(void* ptr, void* value);
void AudioUtilsSetCard(void* ptr, void* card);
void* AudioUtilsGetPtrPcmChip(void* ptr);
void AudioUtilsSetPcmChip(void* ptr, void* value);
void AudioUtilsSetLinuxAudio(void* ptr, void* value);
void AudioUtilsSetCardtoPcmChip(void* ptr, void* card);
void AudioUtilsSetCardtorgb133dev(void* _rgb133, void* _card, int channel);
void AudioUtilsGetCardFromrgb133dev(void** _pcard, void* _rgb133, int channel);
void AudioUtilsSetInitSuccess(void* ptr, int init_success);
void AudioUtilsSetChannel(void* ptr, int channel);
void AudioUtilsSetPDE(void* ptr, void* value);
void AudioUtilsSethandleAudioCapture(void* ptr, unsigned int handle);
void AudioUtilsStoreConfigToChip(void* _chip, struct rgb133_audio_config config);

void AudioUtilsSetAudioTimer(void* _chip, void* pThread);
PAUDIOCHANNELAPI AudioUtilsGetAudioChannelPtr(void* arg);

int PcmCreateNew(void* ptr);

int SndCardMixerCreateNew(void* ptr);

#endif /* _RGB133SNDCARD_H */
