/*
 * Capture.c
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Implements the core capture application functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <linux/ioctl.h>

#include <fcntl.h>
#include <string.h>

#include <sys/mman.h>

#include "Capture.h"
#include "CaptureThread.h"

#include "CaptureDebug.h"

#include "rgb133control.h"
#include "rgb133v4l2.h"

const char* defaultDevice = "/dev/video0";

const struct
{
    unsigned int i_v4l2;
    int i_fourcc;
    int i_rmask;
    int i_gmask;
    int i_bmask;
} v4l2chroma_to_fourcc[] =
{
      /* Raw data types */
      { V4L2_PIX_FMT_GREY,    v4l2_fourcc('G','R','E','Y'), 0, 0, 0 },
      { V4L2_PIX_FMT_Y800,    v4l2_fourcc('Y','8','0','0'), 0, 0, 0 },
      { V4L2_PIX_FMT_Y8,      v4l2_fourcc('Y','8',' ',' '), 0, 0, 0 },
      { V4L2_PIX_FMT_RGB555,  v4l2_fourcc('R','V','1','5'), 0x001f,0x03e0,0x7c00 },
      { V4L2_PIX_FMT_RGB565,  v4l2_fourcc('R','V','1','6'), 0x001f,0x07e0,0xf800 },
      { V4L2_PIX_FMT_RGB24,   v4l2_fourcc('R','V','2','4'), 0xff0000,0xff00,0xff },
      { V4L2_PIX_FMT_RGB32,   v4l2_fourcc('R','V','3','2'), 0xff0000,0xff00,0xff },
      { V4L2_PIX_FMT_YUYV,    v4l2_fourcc('Y','U','Y','2'), 0, 0, 0 },
      { V4L2_PIX_FMT_UYVY,    v4l2_fourcc('U','Y','V','Y'), 0, 0, 0 },
      { V4L2_PIX_FMT_NV12,    v4l2_fourcc('N','V','1','2'), 0, 0, 0 },
      { V4L2_PIX_FMT_YVU420,  v4l2_fourcc('Y','V','1','2'), 0, 0, 0 },
      { V4L2_PIX_FMT_RGB10,   v4l2_fourcc('A','R','3','0'), 0, 0, 0 },
      { V4L2_PIX_FMT_Y410,    v4l2_fourcc('A','Y','3','0'), 0, 0, 0 },
      { 0, 0, 0, 0, 0 }
};

/*!
 * Implementation of IsSupportedPixelFormat.
 */
int IsSupportedPixelFormat(char* pixfmt)
{
   int len = -1;
   unsigned int pixelformat = 0;
   int supported = 0;
   int i = 0;

   if(pixfmt == 0)
      return 0;

   len = strlen(pixfmt);
   if(len <= 0 ||
      len > 4)
   {
      CapError("Invalid fourcc pixel format: %s\n", pixfmt);
      return 0;
   }

   /* Pad with spaces if necessary */
   if(len < 4)
      for(i=4; i!=len; i--)
         pixfmt[i-1] = ' ';

   pixelformat = v4l2_fourcc(pixfmt[0], pixfmt[1], pixfmt[2], pixfmt[3]);

   i = 0;
   while(v4l2chroma_to_fourcc[i].i_v4l2)
   {
      if(pixelformat == v4l2chroma_to_fourcc[i].i_fourcc)
      {
         supported++;
         break;
      }
      i++;
   }

   if(supported)
      return v4l2chroma_to_fourcc[i].i_v4l2;
   else
      return 0;
}

/*!
 * Implementation of EnumerateCaptureInputs.
 */
int EnumerateCaptureInputs(psCapture pCapture)
{
   struct v4l2_input input;
   int index = 0;

   /* Find all inputs */
   memset(&input, 0, sizeof(input));
   while(ioctl(pCapture->fd, VIDIOC_ENUMINPUT, &input) >= 0)
   {
       pCapture->inputs++;
       input.index = pCapture->inputs;
   }

   /* Allocate memory to hold enumerated struct for each input */
   pCapture->pInputs = malloc(pCapture->inputs * sizeof(struct v4l2_input));
   if(pCapture->pInputs == 0)
      return -1;

   /* Enumerate all inputs */
   for(index = 0; index < pCapture->inputs; index++)
   {
      pCapture->pInputs[index].index = index;

      if(ioctl(pCapture->fd, VIDIOC_ENUMINPUT, &pCapture->pInputs[index]))
      {
         CapError("Failed to Enumerate Input[%d]\n", index);
         return -1;
      }
      CapDebug("Enumerated Input[%d](%s): Type(%d)\n",
            index,
            pCapture->pInputs[index].name,
            pCapture->pInputs[index].type);
   }
   return 0;
}

/*!
 * Implementation of EnumerateVideoStandards.
 */
int EnumerateVideoStandards(psCapture pCapture)
{
   struct v4l2_standard standards;
   int index = 0;

   standards.index = 0;
   pCapture->standard = 0;

   /* Find all standards */
   while(ioctl(pCapture->fd, VIDIOC_ENUMSTD, &standards) >= 0)
   {
      pCapture->standards++;
      standards.index = pCapture->standards;
   }

   pCapture->pStandards = malloc(pCapture->standards * sizeof(struct v4l2_standard));
   if(pCapture->pStandards == 0)
      return -1;

   for(index = 0; index < pCapture->standards; index++)
   {
      pCapture->pStandards[index].index = index;

      if(ioctl( pCapture->fd, VIDIOC_ENUMSTD, &pCapture->pStandards[index]))
      {
         CapError("Failed to Enumerate Standard[%d]\n", index);
         return -1;
      }
      CapDebug("Enumerated Standard[%d](%s)\n",
            index,
            pCapture->pStandards[index].name);
   }
   return 0;
}

/*!
 * Implementation of EnumerateCaptureFormats.
 */
int EnumerateCaptureFormats(psCapture pCapture)
{
   struct v4l2_fmtdesc format;
   int index = 0, i = 0;

   /* Initialise structure */
   memset( &format, 0, sizeof(format) );
   format.index = index;
   format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

   /* Find all formats */
   while(ioctl(pCapture->fd, VIDIOC_ENUM_FMT, &format) >= 0)
   {
      index++;
      format.index = index;
   }

   pCapture->formats = index;

   pCapture->pFormats = malloc(pCapture->formats * sizeof(struct v4l2_fmtdesc));
   if(pCapture->pFormats == 0)
      return -1;

   for(index = 0; index < pCapture->formats; index++)
   {
      pCapture->pFormats[index].index = index;
      pCapture->pFormats[index].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if(ioctl(pCapture->fd, VIDIOC_ENUM_FMT, &pCapture->pFormats[index]) < 0)
      {
         CapError("Failed to get format[%d] description\n", index);
         return -1;
      }

      {
         char fourcc_v4l2[5];
         memset( &fourcc_v4l2, 0, sizeof(fourcc_v4l2));
         rgb133_fourcc_to_char(pCapture->pFormats[index].pixelformat, &fourcc_v4l2);
         int format_supported = 0;
         for(i=0; v4l2chroma_to_fourcc[i].i_v4l2 != 0; i++)
         {
            if( v4l2chroma_to_fourcc[i].i_v4l2 == pCapture->pFormats[index].pixelformat )
            {
               format_supported = 1;

               char fourcc[5];
               memset(&fourcc, 0, sizeof(fourcc));
               rgb133_fourcc_to_char( v4l2chroma_to_fourcc[i].i_fourcc, &fourcc );
               CapDebug("Supported Chroma  [%.2d]:   %4s [%s, %s]\n",
                     index,
                     fourcc, pCapture->pFormats[index].description,
                     fourcc_v4l2);

            }
         }
         if(!format_supported)
         {
            CapDebug("Unsupported Chroma[%.2d]:   %4s [%s]\n",
                  index, fourcc_v4l2,
                  pCapture->pFormats[index].description);
         }
      }
   }

   return 0;
}

/*!
 * Implementation of OpenCaptureDevice.
 */
int OpenCaptureDevice(char* device, psCapture pCapture, BOOL blocking)
{
   char* realDevice = 0;
   unsigned long flags = O_RDWR;

   if(device[0])
   {
      CapLog("Opening capture device: %s\n", device);
      realDevice = device;
   }
   else
   {
      CapLog("Opening default capture device: %s\n", defaultDevice);
      realDevice = (char*)defaultDevice;
   }

   if(blocking)
   {
      flags |= O_NONBLOCK;
      CapError("Opening device with non-blocking IO (0x%lx)\n", flags);
   }

   /* Open the device */
   if((pCapture->fd = open(realDevice, flags)) < 0)
   {
      CapError("Failed to open \"%s\"\n", realDevice);
      goto open_failed;
   }

   /* Query the capture device capabilites */
   CapMessage("Query device capabilities into %p\n", &pCapture->caps);
   if(ioctl(pCapture->fd, VIDIOC_QUERYCAP, &pCapture->caps) < 0)
   {
      CapError("Failed to query device capabilities.\n");
      goto open_failed;
   }

   /* Enumerate Capture Device Capabilities */
   if( pCapture->caps.capabilities & V4L2_CAP_VIDEO_CAPTURE )
   {
      CapLog("Enumerate Capture Inputs\n");
      if(EnumerateCaptureInputs(pCapture))
      {
         CapError("Failed to enumerate capture inputs: %d\n", pCapture->inputs);
         goto open_failed;
      }
      CapLog("Enumerate Video Standards\n");
      if(EnumerateVideoStandards(pCapture))
      {
         CapError("Failed to enumerate video standards: %d\n", pCapture->standards);
         goto open_failed;
      }
      CapLog("Enumerate Capture Formats\n");
      if(EnumerateCaptureFormats(pCapture))
      {
         CapError("Failed to enumerate capture formats: %d\n", pCapture->formats);
         goto open_failed;
      }
   }
   else
   {
      CapError("Only Video Captures are supported by this application.\n");
      goto open_failed;
   }

   /* Check the io method */
   if( !(pCapture->caps.capabilities & V4L2_CAP_READWRITE) )
   {
      CapError("Capture device does not supoprt read io\n");
      goto open_failed;
   }

   /* Try and find default resolution if not specified */
   memset( &pCapture->fmt, 0, sizeof(pCapture->fmt) );
   pCapture->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

   if( pCapture->output_width == 0 || pCapture->output_height == 0 )
   {
      /* Use current width and height settings */
      CapMessage("Get Current source width and height.\n");
      if(ioctl(pCapture->fd, VIDIOC_G_FMT, &pCapture->fmt) < 0)
      {
         CapError("Failed to get current source width and height\n");
         goto open_failed;
      }

      pCapture->output_width  = pCapture->fmt.fmt.pix.width;
      pCapture->output_height = pCapture->fmt.fmt.pix.height;

      if( pCapture->fmt.fmt.pix.field == V4L2_FIELD_ALTERNATE )
      {
         pCapture->output_height = pCapture->output_height * 2;
      }
   }
   else if( pCapture->output_width < 0 || pCapture->output_height < 0 )
   {
      CapMessage("Using Optimal width and height.\n");
   }
   else
   {
       /* The width and height have come from the command line (or defaults) */
      CapMessage("Using Specified width and height -> %lux%lu\n",
            pCapture->output_width, pCapture->output_height);
   }

   pCapture->fmt.fmt.pix.width = pCapture->output_width;
   pCapture->fmt.fmt.pix.height = pCapture->output_height;
   pCapture->fmt.fmt.pix.field = V4L2_FIELD_NONE;

   /* Setup Capture Format */
   pCapture->fmt.fmt.pix.pixelformat = pCapture->output_pixfmt;
   if(ioctl(pCapture->fd, VIDIOC_S_FMT, &pCapture->fmt) < 0)
   {
      CapError("Failed to set capture format: %lux%lu for 0x%x (%c%c%c%c)\n",
            pCapture->output_width, pCapture->output_height,
            pCapture->output_pixfmt,
            ((char*)&pCapture->output_pixfmt)[0],
            ((char*)&pCapture->output_pixfmt)[1],
            ((char*)&pCapture->output_pixfmt)[2],
            ((char*)&pCapture->output_pixfmt)[3]);
      goto open_failed;
   }
   CapDebug("Set capture format: %lux%lu for 0x%x (%c%c%c%c)\n",
         pCapture->output_width, pCapture->output_height,
         pCapture->output_pixfmt,
         ((char*)&pCapture->output_pixfmt)[0],
         ((char*)&pCapture->output_pixfmt)[1],
         ((char*)&pCapture->output_pixfmt)[2],
         ((char*)&pCapture->output_pixfmt)[3]);

   /* Read back set width and height settings */
   CapMessage("read back set output buffer width and height.\n");
   if(ioctl(pCapture->fd, VIDIOC_G_FMT, &pCapture->fmt) < 0)
   {
      CapError("Failed to get current output buffer width and height\n");
      goto open_failed;
   }

   CapLog("Capture format: %ux%u for 0x%x (%c%c%c%c)\n",
         pCapture->fmt.fmt.pix.width, pCapture->fmt.fmt.pix.height,
         pCapture->fmt.fmt.pix.pixelformat,
         ((char*)&pCapture->fmt.fmt.pix.pixelformat)[0],
         ((char*)&pCapture->fmt.fmt.pix.pixelformat)[1],
         ((char*)&pCapture->fmt.fmt.pix.pixelformat)[2],
         ((char*)&pCapture->fmt.fmt.pix.pixelformat)[3]);

   /* Get the source width and height settings */
   memset( &pCapture->src_fmt, 0, sizeof(pCapture->src_fmt) );
   pCapture->src_fmt.type = V4L2_BUF_TYPE_CAPTURE_SOURCE;
   CapMessage("read source width and height.\n");
   if((ioctl(pCapture->fd, RGB133_VIDIOC_G_SRC_FMT, &pCapture->src_fmt)) < 0)
   {
      CapError("Failed to get source input format through proprietary ioctl\n");
      if(ioctl(pCapture->fd, VIDIOC_G_FMT, &pCapture->src_fmt) < 0)
      {
         CapError("Failed to get source input format using ioctl\n");
         goto open_failed;
      }
      else
      {
         CapLog("Source format (ioctl): ");
      }
   }
   else
   {
      CapLog("Source format (proprietary ioctl): ");
   }

   CapLog("%ux%u @ %.2fHz for 0x%x (%c%c%c%c)\n",
         pCapture->src_fmt.fmt.pix.width, pCapture->src_fmt.fmt.pix.height,
         (float)((float)pCapture->src_fmt.fmt.pix.priv / 1000.0),
         pCapture->src_fmt.fmt.pix.pixelformat,
         ((char*)&pCapture->src_fmt.fmt.pix.pixelformat)[0],
         ((char*)&pCapture->src_fmt.fmt.pix.pixelformat)[1],
         ((char*)&pCapture->src_fmt.fmt.pix.pixelformat)[2],
         ((char*)&pCapture->src_fmt.fmt.pix.pixelformat)[3]);

   return 0;

open_failed:
   if(pCapture->fd >= 0)
      CloseCaptureDevice(pCapture->fd, pCapture);
   return -1;
}

/*!
 * Implementation of OpenCaptureDeviceControl.
 */
int OpenCaptureDeviceControl(psCapture pCapture, const char* ctrlDevice)
{
   CapDebug("Opening capture device control: %s\n", ctrlDevice);
   /* Open the device */
   if((pCapture->ctrlFd = open(ctrlDevice, O_RDWR)) < 0)
      return -1;

   return 0;
}

/*!
 * Implementation of InitialiseReadCapture.
 */
int InitialiseReadCapture(psCapture pCapture, unsigned int buffer_size)
{
   if(buffer_size == 0)
   {
      CapError("Invalid buffer initialisation size: %u bytes\n", buffer_size);
      return -1;
   }

   pCapture->pBuffers[0].DataLength = buffer_size;
   pCapture->pBuffers[0].pData = malloc(buffer_size);
   if(pCapture->pBuffers[0].pData == 0)
      return -1;

   CapMessage("Read buffer(%p) of %u bytes initialised.\n",
         pCapture->pBuffers[0].pData, pCapture->pBuffers[0].DataLength);
   return 0;
}

/*!
 * Implementation of UninitialiseReadCapture.
 */
int UninitialiseReadCapture(psCapture pCapture)
{
   if(pCapture->pBuffers[0].pData != 0)
   {
      free(pCapture->pBuffers[0].pData);
      pCapture->pBuffers[0].pData = 0;
      pCapture->pBuffers[0].DataLength = 0;
   }
   return 0;
}

/*!
 * Implementation of InitialiseMmapCapture.
 */
int InitialiseMmapCapture(psCapture pCapture, unsigned int buffer_size)
{
   int ret = 0;

   memset( &pCapture->req, 0, sizeof(pCapture->req) );
   pCapture->req.count = NUM_STREAMING_BUFFERS;
   pCapture->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   pCapture->req.memory = V4L2_MEMORY_MMAP;

   if((ret = ioctl(pCapture->fd, VIDIOC_REQBUFS, &pCapture->req)) < 0)
   {
       CapError("Device does not support mmap IO: %d\n", ret);
       return -1;
   }

   if( pCapture->req.count < NUM_STREAMING_BUFFERS )
   {
      CapError("Insufficient buffers (%d, expected %d) for mmap IO\n", pCapture->req.count, NUM_STREAMING_BUFFERS);
      return -1;
   }

   if(buffer_size == 0)
   {
      CapError("Invalid buffer initialisation size: %u bytes\n", buffer_size);
      return -1;
   }

   for(pCapture->buffers = 0; pCapture->buffers < pCapture->req.count; ++pCapture->buffers )
   {
      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = pCapture->buffers;

      if((ret = ioctl(pCapture->fd, VIDIOC_QUERYBUF, &buf)) < 0)
      {
         CapError("VIDIOC_QUERYBUF failed: %d\n", ret);
         return -1;
      }

      pCapture->pBuffers[pCapture->buffers].DataLength = buf.length;
      pCapture->pBuffers[pCapture->buffers].pData =
            mmap( NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, pCapture->fd, buf.m.offset );

      CapMessage("Mmap buffer[%d](%p) of %u bytes mmap'd.\n", pCapture->buffers,
            pCapture->pBuffers[pCapture->buffers].pData,
            pCapture->pBuffers[pCapture->buffers].DataLength);

      if( pCapture->pBuffers[pCapture->buffers].pData == MAP_FAILED )
      {
         CapError("Failed to mmap buffer[%d]\n", pCapture->buffers);
         return -1;
      }
   }

   return 0;
}

/*!
 * Implementation of InitialiseUserPtrCapture.
 */
int InitialiseUserPtrCapture(psCapture pCapture, unsigned int buffer_size)
{
   int ret = 0;

   memset( &pCapture->req, 0, sizeof(pCapture->req) );
   pCapture->req.count = NUM_STREAMING_BUFFERS;
   pCapture->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   pCapture->req.memory = V4L2_MEMORY_USERPTR;

   if(buffer_size == 0)
   {
      CapError("Invalid buffer initialisation size: %u bytes\n", buffer_size);
      return -1;
   }

   if((ret = ioctl(pCapture->fd, VIDIOC_REQBUFS, &pCapture->req)) < 0)
   {
       CapError("Device does not support userptr IO: %d\n", ret);
       return -1;
   }

   for(pCapture->buffers = 0; pCapture->buffers < pCapture->req.count; ++pCapture->buffers)
   {
      pCapture->pBuffers[pCapture->buffers].pData = malloc(buffer_size);
      if(!pCapture->pBuffers[pCapture->buffers].pData)
      {
         int i;
         // If allocation fails, free buffers we managed to allocate, then return
         for(i = (pCapture->buffers - 1); i >= 0; --i)
         {
            free(pCapture->pBuffers[i].pData);
            pCapture->pBuffers[i].pData = 0;
            pCapture->pBuffers[i].DataLength = 0;
         }
         CapError("Failed to allocate buffer %d for userptr io\n", pCapture->buffers);
         return -1;
      }

      pCapture->pBuffers[pCapture->buffers].DataLength = buffer_size;

      CapMessage("Userptr buffer[%d](%p) of %u bytes allocated.\n",
            pCapture->buffers,
            pCapture->pBuffers[pCapture->buffers].pData,
            pCapture->pBuffers[pCapture->buffers].DataLength);
   }

   return 0;
}

/*!
 * Implementation of InitialiseMmapCapture.
 */
int UninitialiseMmapCapture(psCapture pCapture)
{
   unsigned int i = 0;
   for(i=0; i<pCapture->buffers; i++)
   {
      if(pCapture->pBuffers[i].pData != 0)
      {
         CapMessage("Uninitialise mmap buffer[%d](%p) of %u bytes.\n", i,
               pCapture->pBuffers[i].pData,
               pCapture->pBuffers[i].DataLength);
         if(munmap(pCapture->pBuffers[i].pData, pCapture->pBuffers[i].DataLength))
         {
            CapError("Failed to munmap buffer[%d]", i);
         }
         pCapture->pBuffers[i].pData = 0;
         pCapture->pBuffers[i].DataLength = 0;
      }
   }
   return 0;
}

/*!
 * Implementation of UninitialiseUserptrCapture.
 */
int UninitialiseUserptrCapture(psCapture pCapture)
{
   unsigned int i = 0;
   for(i=0; i<pCapture->buffers; i++)
   {
      if(pCapture->pBuffers[i].pData != 0)
      {
         CapMessage("Uninitialise userptr buffer[%d](%p) of %u bytes.\n", i,
               pCapture->pBuffers[i].pData,
               pCapture->pBuffers[i].DataLength);

         free(pCapture->pBuffers[i].pData);
         pCapture->pBuffers[i].pData = 0;
         pCapture->pBuffers[i].DataLength = 0;
      }
   }

   return 0;
}

#define NUMERATOR 1001
#define DIVISOR 10

/*!
 * Implementation of InitialiseCapture.
 */
int InitialiseCapture(psCapture pCapture)
{
   struct v4l2_selection crop, compose;
   unsigned int d1 = 0;
   unsigned int d2 = 0;

   int rc = 0;

   /* Initialise internals */
   pCapture->inputs = 0;
   pCapture->standards = 0;

   /* Setup the capture rate, if rate is 0.0
    * don't set, just use the default which is the input rate
    */
   if(pCapture->fFrameRate > 0.0)
   {
      /* Convert the fp frame rate into a numerator and denominator for
       * use in the v4l struct
       *  - Assume numerator is 1001 (??)
       *
       *  - Calculate denominator
       *
       *     n                       d
       *     - =   FrameTime   and   - = fps
       *     d                       n
       *
       *     d = (n * fps)
       */
      pCapture->StrmParm.parm.capture.timeperframe.numerator = NUMERATOR;

      /* Multiply by 10 to increase precision, then add half lowest value and
       * divide by 10 to obtain the 'real' denominator.
       */
      pCapture->StrmParm.parm.capture.timeperframe.denominator = ((NUMERATOR * DIVISOR * pCapture->fFrameRate) + (DIVISOR / 2)) / DIVISOR;

      pCapture->StrmParm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if((rc = ioctl(pCapture->fd, VIDIOC_S_PARM, &pCapture->StrmParm)) < 0)
      {
         CapError("Failed to set capture rate to %.2f: %d\n",
               pCapture->fFrameRate, rc);
         return rc;
      }
   }

   /* Composing targets - layout of the image inside the output buffer */
   if(pCapture->ComposingActive)
   {
      memset(&compose, 0, sizeof(compose));
      compose.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      compose.target   = V4L2_SEL_TGT_COMPOSE;
      compose.r.left   = pCapture->ComposeLeft;
      compose.r.top    = pCapture->ComposeTop;
      compose.r.width  = pCapture->ComposeWidth;
      compose.r.height = pCapture->ComposeHeight;

      CapLog("Set composing rectangle(%d,%d,%d,%d)\n",
            compose.r.left, compose.r.top, compose.r.width, compose.r.height);
      if((rc = ioctl(pCapture->fd, VIDIOC_S_SELECTION, &compose)) != 0)
      {
         CapError("Failed to set composing rectangle(%d,%d,%d,%d): %d\n",
               compose.r.left, compose.r.top, compose.r.width, compose.r.height, rc);
         return rc;
      }
   }

   memset(&compose, 0, sizeof(compose));
   compose.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   compose.target = V4L2_SEL_TGT_COMPOSE;
   if((rc = ioctl(pCapture->fd, VIDIOC_G_SELECTION, &compose)) != 0)
   {
      CapError("Failed to get composing: %d\n", rc);
      return rc;
   }
   CapLog("Current composing rectangle is (%d,%d,%d,%d)\n",
         compose.r.left, compose.r.top, compose.r.width, compose.r.height);

   /* Cropping targets */
   /* Cropping bounds - all valid crop rectangles fit inside the crop bounds rectangle */
   memset(&crop, 0, sizeof(crop));
   crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   crop.target = V4L2_SEL_TGT_CROP_BOUNDS;
   if((rc = ioctl(pCapture->fd, VIDIOC_G_SELECTION, &crop)) != 0)
   {
      CapError("Failed to get cropping bounds: %d\n", rc);
      return rc;
   }
   CapMessage("Bounds of the crop rectangle are (%d,%d,%d,%d)\n",
         crop.r.left, crop.r.top, crop.r.width, crop.r.height);

   /* Default cropping rectangle - covers the entire picture */
   memset(&crop, 0, sizeof(crop));
   crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   crop.target = V4L2_SEL_TGT_CROP_DEFAULT;
   if((rc = ioctl(pCapture->fd, VIDIOC_G_SELECTION, &crop)) != 0)
   {
      CapError("Failed to get default cropping: %d\n", rc);
      return rc;
   }
   CapMessage("Default crop rectangle is (%d,%d,%d,%d)\n",
         crop.r.left, crop.r.top, crop.r.width, crop.r.height);

   /* Active cropping rectangle */
   if(pCapture->CroppingActive)
   {
      memset(&crop, 0, sizeof(crop));
      crop.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      crop.target   = V4L2_SEL_TGT_CROP;
      crop.r.left   = pCapture->CroppingLeft;
      crop.r.top    = pCapture->CroppingTop;
      crop.r.width  = pCapture->CroppingWidth;
      crop.r.height = pCapture->CroppingHeight;

      CapLog("Set cropping(%d,%d,%d,%d)\n",
            crop.r.left, crop.r.top, crop.r.width, crop.r.height);
      if((rc = ioctl(pCapture->fd, VIDIOC_S_SELECTION, &crop)) != 0)
      {
         CapError("Failed to set cropping(%d,%d,%d,%d): %d\n",
               crop.r.left, crop.r.top, crop.r.width, crop.r.height, rc);
         return rc;
      }
   }

   memset(&crop, 0, sizeof(crop));
   crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   crop.target = V4L2_SEL_TGT_CROP;
   if((rc = ioctl(pCapture->fd, VIDIOC_G_SELECTION, &crop)) != 0)
   {
      CapError("Failed to get cropping: %d\n", rc);
      return rc;
   }
   CapLog("Current crop rectangle is (%d,%d,%d,%d)\n",
         crop.r.left, crop.r.top, crop.r.width, crop.r.height);

   if(pCapture->livestream)
   {
      struct v4l2_queryctrl queryctrl;
      struct v4l2_control control;

      memset(&queryctrl, 0, sizeof(queryctrl));
      queryctrl.id = RGB133_V4L2_CID_LIVESTREAM;

      if ((rc = ioctl(pCapture->fd, VIDIOC_QUERYCTRL, &queryctrl)) == -1)
      {
         if (errno != EINVAL)
         {
            CapError("VIDIOC_QUERYCTRL failed for RGB133_V4L2_CID_LIVESTREAM, error(0x%x)\n", rc);
            return rc;
         }
         else
         {
            CapError("RGB133_V4L2_CID_LIVESTREAM is not supported\n");
         }
      }
      else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      {
         CapError("RGB133_V4L2_CID_LIVESTREAM is not supported\n");
      }
      else
      {
         memset(&control, 0, sizeof (control));
         control.id = RGB133_V4L2_CID_LIVESTREAM;
         control.value = pCapture->livestream;

         if ((rc = ioctl(pCapture->fd, VIDIOC_S_CTRL, &control)) == -1)
         {
            perror("VIDIOC_S_CTRL failed for RGB133_V4L2_CID_LIVESTREAM: ");
            return rc;
         }
      }

      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_LIVESTREAM;

      if ((rc = ioctl(pCapture->fd, VIDIOC_G_CTRL, &control)) == -1)
      {
         perror("VIDIOC_G_CTRL failed for RGB133_V4L2_CID_LIVESTREAM: ");
         return rc;
      }

      CapError("RGB133_V4L2_CID_LIVESTREAM current value: %d\n", control.value);
   }

   return rc;
}

/*!
 * Implementation of InitialiseCapture.
 */
int StartCapture(psCapture pCapture)
{
   psCaptureThreadArgs pCaptureThreadArgs = 0;
   CapDebug("Start Capture Thread...\n");
   if(CreateCaptureThread(pCapture, pCaptureThreadArgs))
      return -1;

   CapDebug("Wait Capture Thread...\n");
   JoinCaptureThread(pCapture);

   CapDebug("Joined Capture Thread...\n");
   FreeCaptureThreadArgs(&pCaptureThreadArgs);

   return 0;
}

/*!
 * Implementation of CloseCaptureDevice.
 */
void CloseCaptureDevice(int fd, psCapture pCapture)
{
   CapDebug("Closing capture device fd: %d\n", fd);

   /* Check for allocated memory */
   if(pCapture->pInputs)
   {
      free(pCapture->pInputs);
      pCapture->pInputs = 0;
   }
   if(pCapture->pStandards)
   {
      free(pCapture->pStandards);
      pCapture->pStandards = 0;
   }
   if(pCapture->pFormats)
   {
      free(pCapture->pFormats);
      pCapture->pFormats = 0;
   }

   /* close the handle */
   close(fd);
}

/*!
 * Implementation of CloseCaptureDeviceControl.
 */
void CloseCaptureDeviceControl(int fd)
{
   CapDebug("Closing capture device control fd: %d\n", fd);

   /* close the handle */
   close(fd);
}
