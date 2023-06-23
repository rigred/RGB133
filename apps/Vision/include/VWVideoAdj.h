
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VIDEOADJ_H
#define VIDEOADJ_H

#include <QGroupBox>

#include "VWInputTab.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QScrollBar;
class QStackedWidget;
class QComboBox;
QT_END_NAMESPACE

class VWData;

typedef enum _eVWVidAdjType
{
   VW_VID_ADJ_DVI,
   VW_VID_ADJ_VIDEO,
   VW_VID_ADJ_ANALOG,
   VW_VID_ADJ_NO_SIG,
} eVWVidAdjType;

class VisionWinVideoAdjBase
{
public:
   virtual void setScrollBarValues(QScrollBar *pScrollBar, int min, int val, int max);
};

class VisionWinVideoAdjDvi : public QWidget
{
    Q_OBJECT

public:
    VisionWinVideoAdjDvi(void);

signals:

public slots:

private slots:

private:
   QLabel  *m_pDviMessage;
};

class VisionWinVideoAdjNoSignal : public QWidget
{
    Q_OBJECT

public:
    VisionWinVideoAdjNoSignal(void);

signals:

public slots:

private slots:

private:
   QLabel  *m_pNoSignalMessage;
};

class VisionWinVideoAdjVGA : public QWidget, VisionWinVideoAdjBase
{
    Q_OBJECT

public:
    VisionWinVideoAdjVGA(VWData* data, QWidget* parent = 0);

    void setDefaults( );

signals:

public slots:
   void vidTypeChanged(int type);

private slots:

private:
   QWidget     *p;
   VWData      *m_pData;

   QLabel      *m_pHorPosValue;
   QLabel      *m_pHorSizeValue;
   QLabel      *m_pPhaseValue;
   QLabel      *m_pVertPosValue;
   QLabel      *m_pBlackLevelValue;

   QScrollBar  *m_pHorPosScrollBar;
   QScrollBar  *m_pHorSizeScrollBar;
   QScrollBar  *m_pPhaseScrollBar;
   QScrollBar  *m_pVertPosScrollBar;
   QScrollBar  *m_pBlackLevelScrollBar;
};

class VisionWinVideoAdjVideo : public QWidget, VisionWinVideoAdjBase
{
    Q_OBJECT

public:
    VisionWinVideoAdjVideo(VWData* data, QWidget* parent = 0);

    void setDefaults( );

signals:

public slots:
   void vidTypeChanged(int type);

private slots:

private:
   QWidget     *p;
   VWData      *m_pData;

   QLabel      *m_pCbLabel;
   QComboBox   *m_pComboBox;

   QLabel      *m_pHorPosValue;
   QLabel      *m_pVertPosValue;

   QScrollBar  *m_pHorPosScrollBar;
   QScrollBar  *m_pVertPosScrollBar;
};

class VisionWinVideoAdj : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinVideoAdj(const QString &title, VWData* data, QWidget *parent = 0);

signals:
   void vidTypeChanged(int type);
   void stackChanged(int index);

public slots:
   void inputPropertiesReset();
   void vidAdjTypeChanged();
   void horPosChanged(int);
   void horSizeChanged(int);
   void phaseChanged(int);
   void vertPosChanged(int);
   void blacklevelChanged(int);

private slots:

private:
   QWidget                    *p;
   VWData                     *m_pData;

   int                         m_index;

   QStackedWidget             *m_pStack;
   QComboBox                  *m_pVidType;

   VisionWinVideoAdjVGA       *m_pVga;
   VisionWinVideoAdjVideo     *m_pVid;
   VisionWinVideoAdjDvi       *m_pDvi;
   VisionWinVideoAdjNoSignal  *m_pNosignal;
};

#endif /* VIDEOADJ_H */
