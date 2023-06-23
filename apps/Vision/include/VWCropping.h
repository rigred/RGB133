
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef CROPPING_H
#define CROPPING_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QRadioButton;
class QSpinBox;
QT_END_NAMESPACE

class VWData;

#define VW_OVERSCAN_S1_TOP      6
#define VW_OVERSCAN_S1_LEFT    14
#define VW_OVERSCAN_S1_WIDTH  708
#define VW_OVERSCAN_S1_HEIGHT 480

#define VW_OVERSCAN_S2_TOP      4
#define VW_OVERSCAN_S2_LEFT    14
#define VW_OVERSCAN_S2_WIDTH  690
#define VW_OVERSCAN_S2_HEIGHT 566

class VisionWinCropping : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinCropping(const QString &title, VWData* data, QWidget *parent = 0);

signals:

public slots:
   void inputPropertiesReset();
   void croppingTypeChanged();

private slots:
   void checkCroppingStatus(bool value);
   void croppingValuesChanged( );

private:
   QWidget       *p;
   VWData        *m_pData;

   bool           m_bIgnore;

   QRadioButton  *m_pOffRadioButton;
   QRadioButton  *m_pOnRadioButton;
   QRadioButton  *m_pOverscanRadioButton;
   QLabel        *m_pTopLabel;
   QLabel        *m_pWidthLabel;
   QLabel        *m_pLeftLabel;
   QLabel        *m_pHeightLabel;
   QSpinBox      *m_pTopSpinBox;
   QSpinBox      *m_pWidthSpinBox;
   QSpinBox      *m_pLeftSpinBox;
   QSpinBox      *m_pHeightSpinBox;
};

#endif /* CROPPING_H */
