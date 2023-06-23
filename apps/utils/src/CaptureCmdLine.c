/*
 * CaptureCmdLine.c
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Implements the capture application command line parsing functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>

#include "CaptureCmdLine.h"
#include "CaptureTypes.h"

#include "CaptureDebug.h"

/*!
 * Implementation of ParseCommandLine.
 */
int ParseCommandLine(int argc, char* argv[], psCmdLineArgs pCmdLineArgs)
{
   int c = 0;
   int is_valid = 0;

   while((c = getopt(argc, argv, "C:c:d:h:i:np:r:w:j:tz:lg:")) != -1)
   {
      switch(c)
      {
         case 'C':
            memset(pCmdLineArgs->composing, 0, MAX_LEN);
            memcpy(pCmdLineArgs->composing, optarg, strlen(optarg));
            break;
         case 'c':
            memset(pCmdLineArgs->cropping, 0, MAX_LEN);
            memcpy(pCmdLineArgs->cropping, optarg, strlen(optarg));
            break;
         case 'd':
            memset(pCmdLineArgs->device, 0, MAX_LEN);
            memcpy(pCmdLineArgs->device, optarg, strlen(optarg));
            break;
         case 'h':
            memset(pCmdLineArgs->output_height, 0, MAX_LEN);
            memcpy(pCmdLineArgs->output_height, optarg, strlen(optarg));
            break;
         case 'i':
            memset(pCmdLineArgs->output_io, 0, MAX_LEN);
            memcpy(pCmdLineArgs->output_io, optarg, strlen(optarg));
            break;
         case 'n':
            pCmdLineArgs->nonblocking = 1;
            break;
         case 'p':
            memset(pCmdLineArgs->output_pixfmt, 0, MAX_LEN);
            memcpy(pCmdLineArgs->output_pixfmt, optarg, strlen(optarg));
            break;
         case 'r':
            memset(pCmdLineArgs->frame_rate, 0, MAX_LEN);
            memcpy(pCmdLineArgs->frame_rate, optarg, strlen(optarg));
            break;
         case 'w':
            memset(pCmdLineArgs->output_width, 0, MAX_LEN);
            memcpy(pCmdLineArgs->output_width, optarg, strlen(optarg));
            break;
         case 'j':
            sscanf(optarg,"%d",&pCmdLineArgs->capture_count);
            break;
         case 't':
            pCmdLineArgs->timestamp = 1;
            break;
         case 'z':
            memset(pCmdLineArgs->delay, 0, MAX_LEN);
            memcpy(pCmdLineArgs->delay, optarg, strlen(optarg));
            break;
         case 'l':
            pCmdLineArgs->livestream = 1;
            break;
         case 'g':
            sscanf(optarg,"%d",&pCmdLineArgs->ganging_type);
            break;
         case'?':
            if(optopt == 'C' ||
               optopt == 'c' ||
               optopt == 'd' ||
               optopt == 'h' ||
               optopt == 'i' ||
               optopt == 'p' ||
               optopt == 'r' ||
               optopt == 'w' ||
               optopt == 'z' ||
               optopt == 'g')
            {
               CapError("Option '-%c' required an argument.\n", optopt);
            }
            break;
         default:
            return -1;
      }
   }
   return 0;
}

