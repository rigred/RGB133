
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWCAPTURETHREAD_H_
#define VWCAPTURETHREAD_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <linux/videodev2.h>

#include "rgb133control.h"

QT_BEGIN_NAMESPACE
class QImage;
class QString;
QT_END_NAMESPACE

class VWData;

typedef struct _sInternalBuffer
{
   void    *pStart;
   size_t   length;
} sIntBuffer, *psIntBuffer;

class VWCaptureThread : public QThread
{
    Q_OBJECT

public:
    VWCaptureThread(QObject *parent = 0, VWData* data = 0);
    ~VWCaptureThread( );

    void init( );
    void uninit( );

    void startCapture( );
    void stopCapture( );

    bool isPaused( ) { return m_bPaused; };
    void setPaused(bool pause);

    void setCaptureRate(int rate, double freq);

    int getCurrentClient( ) { return m_client; };

    void getCropping(sVWAOI *pAOI);
    void setCropping(sVWAOI AOI);

    void getLiveStream(unsigned long *livestream);
    void setLiveStream(unsigned long livestream);

signals:
   void capturedImage(const QImage &image);
   void restartCapture( );

protected:
   void run( );

private:
   VWData                      *m_pData;

   int                          m_client;

   struct v4l2_format           fmt;
   struct v4l2_buffer           buf;
   struct v4l2_requestbuffers   req;
   enum v4l2_buf_type           type;
   fd_set                       fds;
   struct timeval               tv;
   int                          r;
   unsigned int                 n_buffers;

   psIntBuffer                  pBuffers;

   bool                         devam;
   bool                         m_bPaused;

   QMutex                      *m_pMutex;
   QWaitCondition               condition;

};

#endif /* VWVIDEOWIDGET_H_ */
