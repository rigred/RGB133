/*
 * CaptureMain.c
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Implements the capture application main function,
 * and helper functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <errno.h>

#include "CaptureThread.h"
#include "CaptureCmdLine.h"
#include "Capture.h"

#include "CaptureDebug.h"
#include "CaptureError.h"

#include "rgb133control.h"
#include "rgb133v4l2.h"

/* Static globals */
static sCapture Capture;

int CaptureStop = 0;

const char* ctrlDevices[] = { "/dev/video63", "/dev/video64" };

void usage(int argc, char* argv[])
{
   fprintf(stdout, "\nUsage: %s [OPTIONS]\n", argv[0]);
   fprintf(stdout, "Perform a capture from a video4linux capture device.\n\n");
   fprintf(stdout, "  -d: Specify video4linux device                                              (default: /dev/video0)\n");
   fprintf(stdout, "  -w: Specify output buffer width                                             (default: 640)\n");
   fprintf(stdout, "  -h: Specify output buffer height                                            (default: 480)\n");
   fprintf(stdout, "  -p: Specify output buffer pixel format                                      (default: RGB565)\n");
   fprintf(stdout, "  -i: Specify video4linux capture IO type, \"read/mmap/userptr\"                (default: \"read\")\n");
   fprintf(stdout, "  -r: Specify frame rate (in Hz, max 2 dp.)                                   (default: input source rate)\n");
   fprintf(stdout, "  -C: Specify image composition in output buffer (format: x,y,width,height)   (default: use entire buffer)\n");
   fprintf(stdout, "  -c: Specify cropping (format: x,y,width,height)                             (default: no cropping applied)\n");
   fprintf(stdout, "  -n: Use non-blocking IO                                                     (not supported by Vision / VisionLC driver yet)\n");
   fprintf(stdout, "  -t: Print timestamp information                                             (default: off)\n");
   fprintf(stdout, "  -l: Use liveStream                                                          (default: off)\n");
   fprintf(stdout, "  -g: Specify input ganging type\n");
   fprintf(stdout, "       0: \"off\", 1: \"2x2\", 2: \"4x1\", 3: \"1x4\",\n");
   fprintf(stdout, "       4: \"2x1\", 5: \"1x2\", 6: \"3x1\", 7: \"1x3\"                                 (default: off)\n");             
   fprintf(stdout, "  -z: Add delay in ms between dequeuing frames                                (default: off, no added delay)\n");
   fprintf(stdout, "  -j: Specify number of buffers to capture                                    (default: continuous capture)\n");
}

void signal_handler(int sig)
{
   signal(sig, SIG_IGN);

   /* Tell threads to stop */
   CapDebug("Signal threads to stop...\n");
   CaptureThreadStop = 1;

   signal(SIGINT, signal_handler);
}

enum {
   LEFT = 0,
   TOP,
   WIDTH,
   HEIGHT,
};

int TokenizeComposing(psCapture pCapture, char* pComposingBuffer, unsigned long length)
{
   unsigned char value[MAX_LEN];
   int values = LEFT;
   int i, pos;

   BOOL bEnd = 0;

   pCapture->ComposingActive = 0;

   pos = 0;
   while(!bEnd)
   {
      /* Get a value */
      for(i=0; pos<=length; i++,pos++)
      {
         if(pComposingBuffer[pos] == 0)
         {
            bEnd = 1;
            i++;
            break;
         }
         else if(pComposingBuffer[pos] != ',')
            value[i] = pComposingBuffer[pos];
         else if(pComposingBuffer[pos] == ',')
         {
            break;
         }
      }
      value[i] = 0;

      switch(values)
      {
         case TOP:
            pCapture->ComposeTop = atoi(value);
            values++;
            break;
         case LEFT:
            pCapture->ComposeLeft = atoi(value);
            values++;
            break;
         case WIDTH:
            pCapture->ComposeWidth = atoi(value);
            values++;
            break;
         case HEIGHT:
            pCapture->ComposeHeight = atoi(value);
            values++;
            break;
      }
      pos++;
   }

   if(values > HEIGHT)
      pCapture->ComposingActive = 1;
   else
      return -1;

   return 0;
}

int TokenizeCropping(psCapture pCapture, char* pCroppingBuffer, unsigned long length)
{
   unsigned char value[MAX_LEN];
   int values = LEFT;
   int i, pos;

   BOOL bEnd = 0;

   pCapture->CroppingActive = 0;

   pos = 0;
   while(!bEnd)
   {
      /* Get a value */
      for(i=0; pos<=length; i++,pos++)
      {
         if(pCroppingBuffer[pos] == 0)
         {
            bEnd = 1;
            i++;
            break;
         }
         else if(pCroppingBuffer[pos] != ',')
            value[i] = pCroppingBuffer[pos];
         else if(pCroppingBuffer[pos] == ',')
         {
            break;
         }
      }
      value[i] = 0;

      switch(values)
      {
         case TOP:
            pCapture->CroppingTop = atoi(value);
            values++;
            break;
         case LEFT:
            pCapture->CroppingLeft = atoi(value);
            values++;
            break;
         case WIDTH:
            pCapture->CroppingWidth = atoi(value);
            values++;
            break;
         case HEIGHT:
            pCapture->CroppingHeight = atoi(value);
            values++;
            break;
      }
      pos++;
   }

   if(values > HEIGHT)
      pCapture->CroppingActive = 1;
   else
      return -1;

   return 0;
}

int SetupValues(sCmdLineArgs CmdLineArgs, psCapture pCapture)
{
   int got_width = 0;

   if (CmdLineArgs.capture_count == 0)
      pCapture->capture_count = -1; /* -1 is continuous capture */
   else
   {
      pCapture->capture_count = CmdLineArgs.capture_count;
   }
   
   if(CmdLineArgs.output_width[0] == 0)
      pCapture->output_width = 640;
   else
   {
      pCapture->output_width = strtol(CmdLineArgs.output_width, 0, 10);
      if(pCapture->output_width < 0 ||
         pCapture->output_width > 4096)
         return -1;
      got_width = 1;
   }

   if(CmdLineArgs.output_height[0] == 0)
   {
      /* If we got a width, use a 4:3 aspect to get the default height */
      if(got_width)
      {
         pCapture->output_height = (pCapture->output_width * 3) / 4;
      }
      else
         pCapture->output_height = 480;
   }
   else
   {
      pCapture->output_height = strtol(CmdLineArgs.output_height, 0, 10);
      if(pCapture->output_height < 0 ||
         pCapture->output_height > 4096)
         return -1;
   }

   if(CmdLineArgs.output_pixfmt[0] == 0)
      pCapture->output_pixfmt = V4L2_PIX_FMT_RGB565;
   else
   {
      if((pCapture->output_pixfmt = IsSupportedPixelFormat(CmdLineArgs.output_pixfmt)) == 0)
      {
         CapError("Invalid pixel format: %s\n", CmdLineArgs.output_pixfmt);
         return -1;
      }
   }

   if(CmdLineArgs.output_io[0] == 0)
      pCapture->ioType = IO_READ;
   else
   {
      if(!strcmp(CmdLineArgs.output_io, "read"))
         pCapture->ioType = IO_READ;
      else if(!strcmp(CmdLineArgs.output_io, "mmap"))
         pCapture->ioType = IO_MMAP;
      else if(!strcmp(CmdLineArgs.output_io, "userptr"))
         pCapture->ioType = IO_USERPTR;
      else
      {
         CapError("Invalid IO method: %s\n", CmdLineArgs.output_io);
         return -1;
      }
   }

   if(CmdLineArgs.frame_rate[0] == 0)
   {
      pCapture->fFrameRate = 0.0;
   }
   else
   {
      char* endPtr = 0;
      pCapture->fFrameRate = strtof(CmdLineArgs.frame_rate, &endPtr);
      if((pCapture->fFrameRate == 0.0 && errno != 0) ||
          endPtr == CmdLineArgs.frame_rate ||
          endPtr != &CmdLineArgs.frame_rate[strlen(CmdLineArgs.frame_rate)])
      {
         CapError("Invalid frame rate: %s\n", CmdLineArgs.frame_rate);
         pCapture->fFrameRate = 0.0;
         return -1;
      }
      else if(pCapture->fFrameRate <= 0.0 ||
         pCapture->fFrameRate > 1000.0)
      {
         CapError("Invalid frame rate: %.2f\n", pCapture->fFrameRate);
         pCapture->fFrameRate = 0.0;
         return -1;
      }
   }

   if(CmdLineArgs.composing[0] != 0)
   {
      /* Tokenise the composing string */
      if(TokenizeComposing(pCapture, CmdLineArgs.composing, strlen(CmdLineArgs.composing)))
         return -1;
   }

   if(CmdLineArgs.cropping[0] != 0)
   {
      /* Tokenise the cropping string */
      if(TokenizeCropping(pCapture, CmdLineArgs.cropping, strlen(CmdLineArgs.cropping)))
         return -1;
   }

   if(CmdLineArgs.delay[0] == 0)
      pCapture->delay = 0;
   else
   {
      pCapture->delay = strtol(CmdLineArgs.delay, 0, 10);
      if(pCapture->delay < 0) /* Less than0ms doesn't make sense */
         pCapture->delay = 0;
      else if(pCapture->delay > 10000) /* Capped at 10s */
         pCapture->delay = 10000;
   }

   pCapture->timestamp = CmdLineArgs.timestamp;
   pCapture->livestream = CmdLineArgs.livestream;
   pCapture->ganging_type = CmdLineArgs.ganging_type;

   return 0;
}

int main(int argc, char* argv[])
{
   sCmdLineArgs CmdLineArgs;
   int i = 0;
   int openedCtrlDev = 0;
   int ret = 0;

   if ((argc == 1) ||
       (argc == 2 && !strcmp(argv[1], "--help")) ||
       (argc == 2 && !strcmp(argv[1], "-h")))
   {
      usage(argc, argv);
      return CAPERROR_NOERROR;
   }

   signal(SIGINT, signal_handler);

   /* Parse the command line */
   memset(&CmdLineArgs, 0, sizeof(sCmdLineArgs));
   CmdLineArgs.capture_count = -1; /* Set to infinite */
   if(ParseCommandLine(argc, argv, &CmdLineArgs))
   {
      usage(argc, argv);
      return -CAPERROR_CMDLINE;
   }

   /* Setup values (or defaults) */
   memset(&Capture, 0, sizeof(sCapture));
   if(SetupValues(CmdLineArgs, &Capture))
   {
      CapError("Failed to initilise arguments.\n\n");
      usage(argc, argv);
      return -CAPERROR_CMDLINEVALUES;
   }

   /* Open the capture device control channel */
   for(i=0; i< sizeof(ctrlDevices) / sizeof(const char*); i++)
   {
      if(OpenCaptureDeviceControl(&Capture, ctrlDevices[i]))
         openedCtrlDev = 0;
      else
      {
         openedCtrlDev = 1;
         break;
      }
   }
   if(!openedCtrlDev)
   {
      CapError("Failed to open capture device control for Vision or VisionLC driver.\n");
      CloseCaptureDevice(Capture.fd, &Capture);
      return -CAPERROR_OPENCONTROLDEVICE;
   }

   while(!CaptureStop) {

      CaptureStop = 1;

      /* Open the capture device */
      if(OpenCaptureDevice(CmdLineArgs.device, &Capture, CmdLineArgs.nonblocking))
      {
         CapError("Failed to open capture device.\n");
         return -CAPERROR_OPENDEVICE;
      }

      switch(Capture.ioType)
      {
         case IO_READ:
            CapLog("Initialise read IO buffers\n");
            if(InitialiseReadCapture(&Capture, Capture.fmt.fmt.pix.sizeimage))
            {
               CapError("Failed to initialise read buffer.\n");
               ret = -CAPERROR_INITCAPTURE;
               goto out;
            }
            break;
         case IO_MMAP:
            CapLog("Initialise mmap IO buffers\n");
            if(InitialiseMmapCapture(&Capture, Capture.fmt.fmt.pix.sizeimage))
            {
               CapError("Failed to initialise mmap buffers.\n");
               ret = -CAPERROR_INITCAPTURE;
               goto out;
            }
            break;
         case IO_USERPTR:
            CapLog("Initialise userptr IO buffers\n");
            if(InitialiseUserPtrCapture(&Capture, Capture.fmt.fmt.pix.sizeimage))
            {
               CapError("Failed to initialise userptr buffers.\n");
               ret = -CAPERROR_INITCAPTURE;
               goto out;
            }
            break;
         default:
            CapError("Invalid IO method: %d\n", Capture.ioType);
            ret = -CAPERROR_INITCAPTURE;
            goto out;
      }

      /* Set up video device */
      if(InitialiseCapture(&Capture))
      {
         CapError("Failed to set up video device.\n");
         ret = -CAPERROR_SETUPCAPTURE;
         goto out;
      }

      /* Start the video capture...
       * Happens in a separate thread.
       */
      if(StartCapture(&Capture))
      {
         CapError("Failed to start video capture.\n");
         ret = -CAPERROR_STARTCAPTURE;
         goto out;
      }

      switch(Capture.ioType)
      {
         case IO_READ:
            /* Initialise read pointers */
            if(UninitialiseReadCapture(&Capture))
            {
               CapError("Failed to initialise read buffer.\n");
               ret = -CAPERROR_UNINITCAPTURE;
               goto out;
            }
            break;
         case IO_MMAP:
            if(UninitialiseMmapCapture(&Capture))
            {
               CapError("Failed to uninitialise mmap buffers.\n");
               ret = -CAPERROR_UNINITCAPTURE;
               goto out;
            }
            break;
         case IO_USERPTR:
            if(UninitialiseUserptrCapture(&Capture))
            {
               CapError("Failed to uninitialise userptr buffers.\n");
               ret = -CAPERROR_UNINITCAPTURE;
               goto out;
            }
            break;
         default:
            CapError("Invalid IO method: %d\n", Capture.ioType);
            ret = -CAPERROR_UNINITCAPTURE;
            goto out;
      }
      CloseCaptureDevice(Capture.fd, &Capture);
   }

out:
   if(ret < 0)
      CloseCaptureDevice(Capture.fd, &Capture);

   CloseCaptureDeviceControl(Capture.ctrlFd);

   return CAPERROR_NOERROR;
}
