/*
 * CaptureError.h
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Declares capture app error related definitions.
 *
 */

typedef enum
{
   CAPERROR_NOERROR = 0,
   CAPERROR_CMDLINE,
   CAPERROR_CMDLINEVALUES,
   CAPERROR_OPENDEVICE,
   CAPERROR_OPENCONTROLDEVICE,
   CAPERROR_INITCAPTURE,
   CAPERROR_UNINITCAPTURE,
   CAPERROR_STARTCAPTURE,
   CAPERROR_SETUPCAPTURE,
} CAPERROR;
