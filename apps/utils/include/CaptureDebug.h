/*
 * CaptureCmdLine.h
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Declares capture app debug macros.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef CAPDEBUG
#define CapDebug(fmt, arg...) \
do \
{ \
   printf("%s: " fmt, __func__, ## arg); \
} while(0);

#define CapMessage(fmt, arg...) \
do \
{ \
   printf("%s: " fmt, __func__, ## arg); \
} while(0);

#define CapLog(fmt, arg...) \
do \
{ \
   printf("%s: " fmt, __func__, ## arg); \
} while(0);

#else
#define CapDebug(fmt, arg...)
#define CapMessage(fmt, arg...)

#define CapLog(fmt, arg...) \
   do \
   { \
      printf(fmt, ## arg); \
   } while(0);

#endif

#define CapError(fmt, arg...) \
   do \
   { \
      printf(fmt, ## arg); \
   } while(0);
