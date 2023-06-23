
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QApplication>
#include <QPaintEvent>
#include <QDebug>

#include "VWConfig.h"
#include "VWData.h"
#include "VWVideoWidget.h"

VWVideoWidget::VWVideoWidget(QWidget *parent, VWData* data) :
   QWidget(parent), m_pData(data)
{
   m_pCaptureThread = 0;

   setAutoFillBackground(true);

   m_pMutex = new QMutex(QMutex::NonRecursive);

   connect(m_pData, SIGNAL(restartVideo()), this, SLOT(restartVideo()));
}

VWVideoWidget::~VWVideoWidget( )
{
   stopVideo();
   if(m_pMutex)
   {
      m_pMutex->unlock();
      delete m_pMutex;
      m_pMutex = 0;
   }
}

void VWVideoWidget::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);
   painter.setPen(Qt::black);
   painter.setFont(QFont("Arial", 30));
   painter.drawText(rect(), Qt::AlignCenter, DRIVER_TAG);
   painter.drawPixmap(this->rect(), m_pixmap);
}

void VWVideoWidget::setPicture(QImage i)
{
   m_pixmap = QPixmap::fromImage(i);
   update();
}

void VWVideoWidget::startVideo( )
{
   if(m_pCaptureThread)
   {
      if(m_pCaptureThread->isRunning())
      {
         m_pCaptureThread->stopCapture();
      }
      m_pCaptureThread = 0;
   }

   m_pCaptureThread = new VWCaptureThread(this, m_pData);
   if(m_pCaptureThread == 0)
   {
      qDebug() << "VWVideoWidget::startVideo: failed to create capture thread!";
      return;
   }

   connect(m_pCaptureThread, SIGNAL(capturedImage(QImage)),
         this, SLOT(setPicture(QImage)));
   connect(m_pCaptureThread, SIGNAL(restartCapture()),
         this, SLOT(restartVideo()));

   m_pData->setCurrentClient(m_pCaptureThread->getCurrentClient());
   m_pCaptureThread->startCapture();
}

void VWVideoWidget::stopVideo( )
{
   if(m_pCaptureThread)
   {
      if(m_pCaptureThread->isRunning())
      {
         m_pCaptureThread->stopCapture();
         m_pCaptureThread->wait();
      }
   }
}

void VWVideoWidget::pauseVideo( )
{
   if(m_pCaptureThread)
   {
      if(m_pCaptureThread->isRunning())
      {
         if(!m_pCaptureThread->isPaused())
            m_pCaptureThread->setPaused(true);
      }
   }
}

void VWVideoWidget::runVideo( )
{
   if(m_pCaptureThread)
   {
      if(m_pCaptureThread->isRunning())
      {
         if(m_pCaptureThread->isPaused())
            m_pCaptureThread->setPaused(false);
      }
   }
}

void VWVideoWidget::setCaptureRate(int rate, double freq)
{
   if(m_pCaptureThread)
   {
      if(m_pCaptureThread->isRunning())
      {
         m_pCaptureThread->setCaptureRate(rate, freq);
      }
   }
}

void VWVideoWidget::setRestartVideo( )
{
   m_pMutex->lock();
}

void VWVideoWidget::waitForVideoRestart( )
{
   m_pMutex->lock();
   m_pMutex->unlock();
}

// Public slots
void VWVideoWidget::restartVideo( )
{
   stopVideo();
   startVideo();
   m_pMutex->unlock();
}
