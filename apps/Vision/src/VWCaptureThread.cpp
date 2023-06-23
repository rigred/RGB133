
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QApplication>
#include <QDataStream>
#include <QString>
#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <QFile>

#include <typeinfo>
#include <unistd.h>

#include "VWConfig.h"
#include "VWCaptureThread.h"
#include "VWData.h"
#include "VisionWindow.h"

#include "rgb133v4l2.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

VWCaptureThread::VWCaptureThread(QObject *parent, VWData* data):
   QThread(parent), m_pData(data)
{
   static struct v4l2_control ctrl;
   int fd = m_pData->getVideoDeviceHandle();

   ctrl.id = RGB133_V4L2_CID_CLIENT_ID;
   ioctl(fd, VIDIOC_G_CTRL, &ctrl);
   m_client = ctrl.value;

   CLEAR(fmt);
   CLEAR(buf);
   CLEAR(req);
   type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   FD_ZERO(&fds);
   CLEAR(tv);
   r = -1;
   n_buffers = 0;
   devam = false;
   m_bPaused = false;

   m_pMutex = new QMutex(QMutex::NonRecursive);
   m_pMutex->lock();
}


VWCaptureThread::~VWCaptureThread( )
{
   if(m_pMutex)
     m_pMutex = 0;
}

void VWCaptureThread::run( )
{
   int fd = m_pData->getVideoDeviceHandle();
   unsigned long pix_fmt = 0;
   QImage::Format q_fmt = QImage::Format_Invalid;
   unsigned int loop = 0;
   unsigned int width = VW_DEFAULT_WIDTH;
   unsigned int height = VW_DEFAULT_HEIGHT;

   if(fd < 0)
   {
      qDebug() << "VWCaptureThread::run: device handle is invalid: " << fd;
      return;
   }

   this->setLiveStream(m_pData->getLiveStream());

   CLEAR(fmt);
   fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   fmt.fmt.pix.width = width;
   fmt.fmt.pix.height = height;
   switch(m_pData->getPixelFormat())
   {
      case VW_PIX_FMT_RGB555:
         fmt.fmt.pix.pixelformat = pix_fmt = V4L2_PIX_FMT_RGB555;
         q_fmt = QImage::Format_RGB555;
         break;
      case VW_PIX_FMT_RGB565:
         fmt.fmt.pix.pixelformat = pix_fmt = V4L2_PIX_FMT_RGB565;
         q_fmt = QImage::Format_RGB16;
         break;
      case VW_PIX_FMT_YUY2:
         fmt.fmt.pix.pixelformat = pix_fmt = V4L2_PIX_FMT_YUYV;
         break;
      case VW_PIX_FMT_RGB888:
      case VW_PIX_FMT_AUTO:
      default:
         fmt.fmt.pix.pixelformat = pix_fmt = V4L2_PIX_FMT_RGB24;
         q_fmt = QImage::Format_RGB888;
         break;
   }
   if(ioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
   {
      qDebug() << "VWCaptureThread::run: Failed to set pixel format: " << hex << fmt.fmt.pix.pixelformat;
      return;
   }
   if(fmt.fmt.pix.width != width || fmt.fmt.pix.height != height)
   {
      qDebug() << "VWCaptureThread::run: Image delivered at: " << fmt.fmt.pix.width
               << "x" << fmt.fmt.pix.height;
   }

   CLEAR(req);
   req.count = 2;
   req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   req.memory = V4L2_MEMORY_MMAP;
   ioctl(fd, VIDIOC_REQBUFS, &req);

   pBuffers = (psIntBuffer)calloc(req.count, sizeof(*pBuffers));
   n_buffers = 0;
   for(loop=0; loop<req.count; loop++)
   {
      CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = loop;

      ioctl(fd, VIDIOC_QUERYBUF, &buf);

      pBuffers[loop].length = buf.length;
      pBuffers[loop].pStart = mmap(NULL, buf.length,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, buf.m.offset);
      if(pBuffers[loop].pStart == MAP_FAILED)
      {
         qDebug() << "VWCaptureThread::run: mmap failed...";
         return;
      }
   }
   n_buffers = loop;

   for(unsigned int i=0; i<n_buffers; i++)
   {
      CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      ioctl(fd, VIDIOC_QBUF, &buf);
   }
   type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   ioctl(fd, VIDIOC_STREAMON, &type);

   while(devam)
   {
      if(m_bPaused)
         usleep(250000);

      m_pMutex->lock();
      if(!devam)
      {
         qDebug() << "VWCaptureThread::run: instructed to exit...";
         m_pMutex->unlock();
         break;
      }

      do
      {
         FD_ZERO(&fds);
         FD_SET(fd, &fds);

         r = select(fd+1, &fds, NULL, NULL, NULL);
      } while((r == -1) && (errno == EINTR));
      if(r == -1)
      {
         qDebug() << "VWCaptureThread::run: Select failed";
         return;
      }

      CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      ioctl(fd, VIDIOC_DQBUF, &buf);

      const unsigned char* pPtr = (const unsigned char*)(pBuffers[buf.index].pStart);
      QImage qq(const_cast<const unsigned char*>(pPtr),
            fmt.fmt.pix.width, fmt.fmt.pix.height, q_fmt);
      qq.detach();
      emit capturedImage(qq);

      ioctl(fd, VIDIOC_QBUF, &buf);
      m_pMutex->unlock();
   }

   type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   ioctl(fd, VIDIOC_STREAMOFF, &type);
   for(unsigned int i=0; i<n_buffers; ++i)
   {
      if(munmap(pBuffers[i].pStart, pBuffers[i].length))
      {
         perror("VWCaptureThread::run: munmap failed");
         printf("VWCaptureThread::run: munmap failed for buffer[%d](%p), length(%zu): errno(%d)\n",
               i, pBuffers[i].pStart, pBuffers[i].length, errno);
      }
   }

   if(pBuffers)
   {
      memset(pBuffers, 0, req.count*sizeof(*pBuffers));
      free(pBuffers);
      pBuffers = 0;
   }
  
   m_pData->closeVideoDevice(fd);
}

void VWCaptureThread::startCapture( )
{
   devam = true;
   this->start();
   m_pMutex->unlock();
}

void VWCaptureThread::stopCapture( )
{
   devam = false;
   if(!m_bPaused)
   {
      m_pMutex->lock();
      m_pMutex->unlock();
   }
   else
      m_pMutex->unlock();
}

void VWCaptureThread::setPaused(bool pause)
{
   m_bPaused = pause;
   if(pause)
      m_pMutex->lock();
   else
      m_pMutex->unlock();
}

void VWCaptureThread::setCaptureRate(int rate, double freq)
{
   int fd = m_pData->getVideoDeviceHandle();
   struct v4l2_streamparm strm;
   unsigned long numerator = 0;
   unsigned long denominator = 0;

   // Rate will be in %, freq in Hz
   numerator = 1000;
   denominator = ((double)(((double)rate * freq) / (double)100) * (double)1000);

   strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   strm.parm.capture.timeperframe.numerator = numerator;
   strm.parm.capture.timeperframe.denominator = denominator;
   ioctl(fd, VIDIOC_S_PARM, &strm);
}

void VWCaptureThread::getCropping(sVWAOI *pAOI)
{
   int fd = m_pData->getVideoDeviceHandle();
   struct v4l2_rect rect;
#ifdef VW_CONFIG_HAVE_SELECTION_API
   struct v4l2_selection selection;

   memset(&selection, 0, sizeof(selection));
   selection.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   selection.target = V4L2_SEL_TGT_CROP;
   ioctl(fd, VIDIOC_G_SELECTION, &selection);
   rect = selection.r;
#else /* !VW_CONFIG_HAVE_SELECTION_API */
   struct v4l2_crop crop;

   memset(&crop, 0, sizeof(crop));
   crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   ioctl(fd, VIDIOC_G_CROP, &crop);
   rect = crop.c;
#endif /* VW_CONFIG_HAVE_SELECTION_API */

   pAOI->top = rect.top;
   pAOI->left = rect.left;
   pAOI->right = rect.width;
   pAOI->bottom = rect.height;
}

void VWCaptureThread::setCropping(sVWAOI AOI)
{
   int fd = m_pData->getVideoDeviceHandle();
#ifdef VW_CONFIG_HAVE_SELECTION_API
   struct v4l2_selection selection;

   selection.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   selection.target = V4L2_SEL_TGT_CROP;
   selection.r.top = AOI.top;
   selection.r.left = AOI.left;
   selection.r.width = AOI.right;
   selection.r.height = AOI.bottom;
   ioctl(fd, VIDIOC_S_SELECTION, &selection);
#else /* !VW_CONFIG_HAVE_SELECTION_API */
   struct v4l2_crop crop;

   crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   crop.c.top = AOI.top;
   crop.c.left = AOI.left;
   crop.c.width = AOI.right;
   crop.c.height = AOI.bottom;
   ioctl(fd, VIDIOC_S_CROP, &crop);
#endif /* VW_CONFIG_HAVE_SELECTION_API */
}

void VWCaptureThread::getLiveStream(unsigned long *livestream)
{
   int fd = m_pData->getVideoDeviceHandle();
   struct v4l2_control control;
   int rc = 0;

   memset(&control, 0, sizeof(control));
   control.id = RGB133_V4L2_CID_LIVESTREAM;
   if ((rc = ioctl(fd, VIDIOC_G_CTRL, &control)) == -1)
   {
      perror("VIDIOC_G_CTRL failed for RGB133_V4L2_CID_LIVESTREAM: ");
      return;
   }
   *livestream = control.value;
}

void VWCaptureThread::setLiveStream(unsigned long livestream)
{
   int fd = m_pData->getVideoDeviceHandle();
   struct v4l2_control control;
   int rc = 0;

   memset(&control, 0, sizeof(control));
   control.id = RGB133_V4L2_CID_LIVESTREAM;
   control.value = livestream;
   if ((rc = ioctl(fd, VIDIOC_S_CTRL, &control)) == -1)
   {
      perror("VIDIOC_S_CTRL failed for RGB133_V4L2_CID_LIVESTREAM: ");
   }
}

