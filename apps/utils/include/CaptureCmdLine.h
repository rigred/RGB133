/*
 * CaptureCmdLine.h
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Declares capture app command line structures and
 * functions.
 *
 */

#define MAX_LEN 256

/*!
 * Command line arguments structure.
 */
typedef struct _sCmdLineArgs
{
   int  valid;

   int  nonblocking;

   char device[MAX_LEN];

   char output_width[MAX_LEN];
   char output_height[MAX_LEN];
   char output_pixfmt[MAX_LEN];

   char output_io[MAX_LEN];

   char frame_rate[MAX_LEN];

   char composing[MAX_LEN];
   char cropping[MAX_LEN];
   int  capture_count;

   int  timestamp;

   char delay[MAX_LEN];

   int  livestream;

   int ganging_type;
} sCmdLineArgs, *psCmdLineArgs;

/*!
 * Function to parse the command line arguments.
 * @param  argc is the number of command line arguments
 * @param  argv is an array of command line arguments
 * @param  pCmdLineArgs is a preallocated sCmdLineArgs structure
 *         which can hold the process arguments
 * @return 0 on success otherwise negative value.
 */
int ParseCommandLine(int argc, char* argv[], psCmdLineArgs pCmdLineArgs);
