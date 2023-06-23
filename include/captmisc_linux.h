/*
 * captmisc_linux.h
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

#ifndef CAPTMISC_LINUX_H_
#define CAPTMISC_LINUX_H_

const char* GetStringColourDomain(unsigned long domainWin);
const char* GetStringAnalogType(unsigned long flagAnalog);
const char* GetStringVideoType(unsigned long videoStandard);
const char* GetStringDisplayPortType(unsigned long flagDisplayPort);
const char* GetStringDigitalType(unsigned long flagDVI);
const char* GetStringSDIType(unsigned long flagSDI);

#endif /* CAPTMISC_LINUX_H_ */
