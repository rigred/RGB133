
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef CAPTURERATE_H
#define CAPTURERATE_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QRadioButton;
class QComboBox;
QT_END_NAMESPACE

class VWData;

class VisionWinCaptureRate : public QGroupBox
{
    Q_OBJECT

public:
   VisionWinCaptureRate(const QString &title, VWData* data, QWidget *parent = 0);

   int getActiveCaptureRate( );
   int getInactiveCaptureRate( );

signals:

public slots:

private slots:
   void enableInactive();

   void activeRateChanged(int index);
   void inactiveRateChanged(int index);

private:
   VWData        *m_pData;

   bool           m_bDisabled;

   QRadioButton  *m_pSameRadioButton;
   QRadioButton  *m_pSpecificRadioButton;
   QLabel        *m_pRateLabel;
   QLabel        *m_pInactiveLabel;
   QComboBox     *m_pActiveCapRate;
   QComboBox     *m_pInactiveCapRate;
};

#endif /* CAPTURERATE_H */
