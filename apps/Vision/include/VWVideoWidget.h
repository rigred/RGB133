
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWVIDEOWIDGET_H_
#define VWVIDEOWIDGET_H_

#include <QWidget>
#include <QPainter>
#include <QQueue>
#include <QMutex>

#include "VWCaptureThread.h"

QT_BEGIN_NAMESPACE
class QPixmap;
QT_END_NAMESPACE

class VWData;
class VWCaptureThread;

class VWVideoWidget : public QWidget
{
    Q_OBJECT

public:
    VWVideoWidget(QWidget *parent = 0, VWData* data = 0);
    ~VWVideoWidget( );

    void startVideo( );
    void stopVideo( );
    void pauseVideo( );
    void runVideo( );

    void getCropping(sVWAOI *pAOI) { m_pCaptureThread->getCropping(pAOI); };
    void setCropping(sVWAOI AOI) { m_pCaptureThread->setCropping(AOI); };

    void setCaptureRate(int rate, double freq);

    void setRestartVideo( );
    void waitForVideoRestart( );

    void getLiveStream(unsigned long *livestream) { m_pCaptureThread->getLiveStream(livestream); };
    void setLiveStream(unsigned long livestream) { m_pCaptureThread->setLiveStream(livestream); };

signals:

public slots:
   void setPicture(QImage);
   void restartVideo( );

protected:
   void paintEvent(QPaintEvent *pEvent);

private:
   VWData          *m_pData;

   // Outout data pixmap
   QPixmap          m_pixmap;

   // Capture thread worker
   VWCaptureThread *m_pCaptureThread;

   QMutex          *m_pMutex;
};

#endif /* VWVIDEOWIDGET_H_ */
