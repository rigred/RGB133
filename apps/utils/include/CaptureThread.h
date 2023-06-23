/*
 * CaptureThread.h
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Declares capture app worker thread structures and
 * functions.
 *
 */

#include <pthread.h>

struct _sCapture;

typedef struct _sCaptureThreadArgs
{
   /* Capture Structure */
   struct _sCapture *pCapture;
} sCaptureThreadArgs, *psCaptureThreadArgs;

extern int CaptureThreadStop;

int CreateCaptureThread(struct _sCapture *pCapture, psCaptureThreadArgs pCaptureThreadArgs);
void JoinCaptureThread(struct _sCapture *pCapture);
void FreeCaptureThreadArgs(psCaptureThreadArgs *pCaptureThreadArgs);
