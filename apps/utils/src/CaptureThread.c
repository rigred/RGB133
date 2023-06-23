/*
 * CaptureThread.c
 *
 * Copyright (c) 2011 Datapath Limited
 *
 * This file forms part of the Vision / VisionLC driver capture
 * application sample source code.
 *
 * Purpose: Implements the capture application capture thread
 * functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <linux/ioctl.h>

#include <unistd.h>
#include <string.h>

#include <poll.h>

#include <errno.h>

#include <syslog.h>

#include "CaptureThread.h"
#include "Capture.h"

#include "CaptureDebug.h"

#include "rgb133control.h"
#include "rgb133v4l2.h"

extern int CaptureStop;
int CaptureThreadStop = 0;

/*!
 * Implementation of CaptureThreadFunc.
 */
void* CaptureThreadFunc(void* arg)
{
   psCaptureThreadArgs pCaptureThreadArgs = (psCaptureThreadArgs)arg;
   psCapture pCapture = pCaptureThreadArgs->pCapture;
   eIOType IOType = pCapture->ioType;

   struct v4l2_buffer buf;
   enum v4l2_buf_type buf_type;

   int i = 0;
   int ret = 0;
   int frame = 0;

   if(IOType == IO_MMAP)
   {
      CapError("TODO: Setup mmap options...\n");
   }

   switch(IOType)
   {
      case IO_READ:
         CapLog("Start capture on %s: %d\n", pCapture->caps.driver, pCapture->fd);
         CapMessage("pData(%p), DataLength(%u)\n", pCapture->pBuffers[0].pData, pCapture->pBuffers[0].DataLength);
         while(!CaptureThreadStop)
         {
            ret = read( pCapture->fd, pCapture->pBuffers[0].pData, pCapture->pBuffers[0].DataLength );
            if(ret < 0)
            {
               switch(errno)
               {
                  case EWOULDBLOCK:
                     //CapError("EWOULDBLOCK...\n");
                     break;
                  case EIO:
                     /* Could ignore EIO, see spec. */
                     /* fall through */
                  default:
                     CapError("Failed to read from Capture Device - %d: %s\n",
                           errno, pCapture->caps.driver);
                     return 0;
               }
            }
            frame++;
            if ((pCapture->capture_count > 0) && (frame >= pCapture->capture_count))
            {
               CapMessage("Captured %d frames, stopping, as requested.\n", pCapture->capture_count);
               CaptureThreadStop = 1;
            }
         }
         break;
      case IO_MMAP:
         CapLog("Start capture on %s: %d\n", pCapture->caps.driver, pCapture->fd);
         CapMessage("pData(%p), DataLength(%u)\n", pCapture->pBuffers[0].pData, pCapture->pBuffers[0].DataLength);

         /* Initial queue of the buffers */
         for(i=0; i<pCapture->buffers; ++i)
         {
             memset( &buf, 0, sizeof(buf) );
             buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             buf.memory = V4L2_MEMORY_MMAP;
             buf.index = i;

             CapError("Q buffer[%d]\n", buf.index);
             if((ret = ioctl(pCapture->fd, VIDIOC_QBUF, &buf)) < 0)
             {
                 CapError("VIDIOC_QBUF failed: %d\n", ret);
                 return 0;
             }
         }

         buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             CapError("STREAMON\n");
         if((ret = ioctl(pCapture->fd, VIDIOC_STREAMON, &buf_type)) < 0)
         {
             CapError("VIDIOC_STREAMON failed: %d\n", ret);
             return 0;
         }

         openlog("CaptureApp", 0, LOG_LOCAL0);

         while(!CaptureThreadStop)
         {
            struct pollfd fd;
            fd.fd = pCapture->fd;
            fd.events = POLLIN|POLLPRI;
            fd.revents = 0;

            /* Wait for data, max 500ms */
            if(poll(&fd, 1, 500) > 0)
            {
               if( fd.revents & (POLLIN|POLLPRI) )
               {
                  memset( &buf, 0, sizeof(buf) );
                  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                  buf.memory = V4L2_MEMORY_MMAP;

                  /* dequeue frame */
                  if((ret = ioctl(pCapture->fd, VIDIOC_DQBUF, &buf)) < 0)
                  {
                     switch( errno )
                     {
                     case EAGAIN:
                        continue;
                     case EIO:
                     default:
                        CapError("Failed to dequeue buffer[%d]: %d\n", buf.index, ret);
                        return 0;
                     }
                  }

                  if(pCapture->timestamp)
                  {
                     static int prev = -1, this = -1;
                     static struct timeval tv_this = {0, 0}, tv_prev = {0, 0};

                     //CapError("DQ buffer[%d].seq(%d) ts(%d.%06d)\n",
                     //      buf.index, buf.sequence,
                     //      buf.timestamp.tv_sec, buf.timestamp.tv_usec);
                     this = buf.sequence;
                     tv_this.tv_sec = buf.timestamp.tv_sec;
                     tv_this.tv_usec = buf.timestamp.tv_usec;

                     if(prev != -1)
                     {
                        if(this != (prev+1))
                        {
                           CapError("DQ buffer[%d].seq(%d)(%d) mismatch\n", buf.index, this, prev);
                        }
                     }
                     prev = this;

                     if(tv_prev.tv_sec != 0 &&
                        tv_prev.tv_usec != 0)
                     {
                        signed int diff_s = tv_this.tv_sec - tv_prev.tv_sec;
                        signed int diff_us = tv_this.tv_usec - tv_prev.tv_usec;
                        if(diff_us < 0)
                        {
                           diff_s--;
                           diff_us += 1000000;
                        }
                        if(diff_s ||
                           (diff_us < 16600 || diff_us > 16700))
                        {
                           CapError("DQ buffer[%d].ts(%d.%06d)(%d.%06d) validity(%d.%06d) frame(%d)\n",
                                 buf.index, (int)tv_this.tv_sec, (int)tv_this.tv_usec,
                                 (int)tv_prev.tv_sec, (int)tv_prev.tv_usec,
                                 diff_s, diff_us, frame);
                        }
                     }
                     tv_prev.tv_sec = tv_this.tv_sec;
                     tv_prev.tv_usec = tv_this.tv_usec;
                  }

                  CapMessage("Dequeued buffer index: %d\n", buf.index);

                  if(buf.index >= pCapture->buffers ) {
                     CapError("Invalid buffer index: %d\n", buf.index);
                     return 0;
                  }

                  frame++;
                  if ((pCapture->capture_count > 0) && (frame >= pCapture->capture_count))
                  {
                     CapMessage("Captured %d frames, stopping, as requested.\n", pCapture->capture_count);
                     CaptureThreadStop = 1;
                  }

                  /* Requeue buffer */
                  //syslog(LOG_ERR, "*** Requeueing Buffer ***\n");
                  CapMessage("Requeuing buffer %d back to driver.\n", buf.index);
                  //CapError("Q buffer[%d]\n", buf.index);
                  if((ret = ioctl(pCapture->fd, VIDIOC_QBUF, &buf)) < 0)
                  {
                     CapError("Failed to requeue buffer[%d]: %d\n", buf.index, ret);
                     return 0;
                  }
               }
            }
            if(pCapture->delay)
               usleep(pCapture->delay*1000);
         }

         buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             CapError("STREAMOFF\n");
         if((ret = ioctl(pCapture->fd, VIDIOC_STREAMOFF, &buf_type)) < 0)
         {
             CapError("VIDIOC_STREAMOFF failed: %d", ret);
         }

         /* Make sure every buffer is dequeued */
         for(i=0; i<pCapture->buffers; i++)
         {
             memset( &buf, 0, sizeof(buf) );
             buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             buf.memory = V4L2_MEMORY_MMAP;
             ioctl(pCapture->fd, VIDIOC_DQBUF, &buf);
             CapError("DQ buffer[%d]\n", buf.index);
         }

         break;
      case IO_USERPTR:
         CapLog("Start capture on %s: %d\n", pCapture->caps.driver, pCapture->fd);
         CapMessage("pData[0](%p), DataLength[0](%u), pData[1](%p), DataLength[1](%u)\n"
                    "pData[2](%p), DataLength[2](%u), pData[3](%p), DataLength[3](%u)\n",
               pCapture->pBuffers[0].pData, pCapture->pBuffers[0].DataLength,
               pCapture->pBuffers[1].pData, pCapture->pBuffers[1].DataLength,
               pCapture->pBuffers[2].pData, pCapture->pBuffers[2].DataLength,
               pCapture->pBuffers[3].pData, pCapture->pBuffers[3].DataLength);

         /* Initial queue of the buffers */
         for(i=0; i<pCapture->buffers; ++i)
         {
            memset( &buf, 0, sizeof(buf) );
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)pCapture->pBuffers[i].pData;
            buf.length = pCapture->pBuffers[i].DataLength;

            CapError("Q buffer[%d]\n", buf.index);
            if((ret = ioctl(pCapture->fd, VIDIOC_QBUF, &buf)) < 0)
            {
               CapError("VIDIOC_QBUF failed: %d\n", ret);
               return 0;
            }
         }

         buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             CapError("STREAMON\n");
         if((ret = ioctl(pCapture->fd, VIDIOC_STREAMON, &buf_type)) < 0)
         {
             CapError("VIDIOC_STREAMON failed: %d\n", ret);
             return 0;
         }

         while(!CaptureThreadStop)
         {
            struct pollfd fd;
            fd.fd = pCapture->fd;
            fd.events = POLLIN|POLLPRI;
            fd.revents = 0;

            /* Wait for data, max 500ms */
            if(poll(&fd, 1, 500) > 0)
            {
               if( fd.revents & (POLLIN|POLLPRI) )
               {
                  memset( &buf, 0, sizeof(buf) );
                  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                  buf.memory = V4L2_MEMORY_USERPTR;

                  /* dequeue frame */
                  if((ret = ioctl(pCapture->fd, VIDIOC_DQBUF, &buf)) < 0)
                  {
                     switch( errno )
                     {
                     case EAGAIN:
                        continue;
                     case EIO:
                     default:
                        CapError("Failed to dequeue buffer[%d]: %d\n", buf.index, ret);
                        return 0;
                     }
                  }

                  if(pCapture->timestamp)
                  {
                     static int prev = -1, this = -1;
                     static struct timeval tv_this = {0, 0}, tv_prev = {0, 0};

                     this = buf.sequence;
                     tv_this.tv_sec = buf.timestamp.tv_sec;
                     tv_this.tv_usec = buf.timestamp.tv_usec;

                     if(prev != -1)
                     {
                        if(this != (prev+1))
                        {
                           CapError("DQ buffer[%d].seq(%d)(%d) mismatch\n", buf.index, this, prev);
                        }
                     }
                     prev = this;

                     if(tv_prev.tv_sec != 0 &&
                        tv_prev.tv_usec != 0)
                     {
                        signed int diff_s = tv_this.tv_sec - tv_prev.tv_sec;
                        signed int diff_us = tv_this.tv_usec - tv_prev.tv_usec;
                        if(diff_us < 0)
                        {
                           diff_s--;
                           diff_us += 1000000;
                        }
                        if(diff_s ||
                           (diff_us < 16600 || diff_us > 16700))
                        {
                           CapError("DQ buffer[%d].ts(%d.%06d)(%d.%06d) validity(%d.%06d) frame(%d)\n",
                                 buf.index, (int)tv_this.tv_sec, (int)tv_this.tv_usec,
                                 (int)tv_prev.tv_sec, (int)tv_prev.tv_usec,
                                 diff_s, diff_us, frame);
                        }
                     }
                     tv_prev.tv_sec = tv_this.tv_sec;
                     tv_prev.tv_usec = tv_this.tv_usec;
                  }

                  CapMessage("Dequeued buffer index: %d\n", buf.index);

                  if(buf.index >= pCapture->buffers ) {
                     CapError("Invalid buffer index: %d\n", buf.index);
                     return 0;
                  }

                  frame++;
                  if ((pCapture->capture_count > 0) && (frame >= pCapture->capture_count))
                  {
                     CapMessage("Captured %d frames, stopping, as requested.\n", pCapture->capture_count);
                     CaptureThreadStop = 1;
                  }

                  /* Requeue buffer */
                  CapMessage("Requeuing buffer %d back to driver.\n", buf.index);
                  if((ret = ioctl(pCapture->fd, VIDIOC_QBUF, &buf)) < 0)
                  {
                     CapError("Failed to requeue buffer[%d]: %d\n", buf.index, ret);
                     return 0;
                  }
               }
            }
            if(pCapture->delay)
               usleep(pCapture->delay*1000);
         }

         buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             CapError("STREAMOFF\n");
         if((ret = ioctl(pCapture->fd, VIDIOC_STREAMOFF, &buf_type)) < 0)
         {
             CapError("VIDIOC_STREAMOFF failed: %d", ret);
         }

         /* Make sure every buffer is dequeued */
         for(i=0; i<pCapture->buffers; i++)
         {
             memset( &buf, 0, sizeof(buf) );
             buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
             buf.memory = V4L2_MEMORY_USERPTR;
             ioctl(pCapture->fd, VIDIOC_DQBUF, &buf);
             CapError("DQ buffer[%d]\n", buf.index);
         }

         break;
      default:
         CapError("Invalid IO method: %d\n", IOType);
         break;
   }

   CapMessage("Capture Thread Finished\n");

   if(CaptureThreadStop)
      CaptureStop = 1;
}

/*!
 * Implementation of CreateCaptureThread.
 */
int CreateCaptureThread(psCapture pCapture, psCaptureThreadArgs pCaptureThreadArgs)
{
   int ret = 0;

   if(pCaptureThreadArgs)
   {
      CapError("Capture Thread structure(%p) may be initialised, bailing out\n", pCaptureThreadArgs);
      return -1;
   }

   pCaptureThreadArgs = malloc(sizeof(sCaptureThreadArgs));
   if(pCaptureThreadArgs == 0)
      return -1;

   memset(pCaptureThreadArgs, 0, sizeof(sCaptureThreadArgs));
   pCaptureThreadArgs->pCapture = pCapture;
   ret = pthread_create(&pCapture->CaptureThread, NULL, CaptureThreadFunc, (void*)pCaptureThreadArgs);
   return 0;
}

/*!
 * Implementation of JoinCaptureThread.
 */
void JoinCaptureThread(psCapture pCapture)
{
   CapMessage("Waiting for capture thread to join...\n");
   pthread_join(pCapture->CaptureThread, 0);
   CapMessage("Capture thread has joined...\n");
}

/*!
 * Implementation of FreeCaptureThreadArgs.
 */
void FreeCaptureThreadArgs(psCaptureThreadArgs *pCaptureThreadArgs)
{
   if(pCaptureThreadArgs && *pCaptureThreadArgs)
   {
      free(*pCaptureThreadArgs);
      *pCaptureThreadArgs = 0;
   }
}
