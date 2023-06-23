
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASPECTRATIO_H_
#define ASPECTRATIO_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QRadioButton;
class QSpinBox;
class QLabel;
QT_END_NAMESPACE

class VWData;

class VisionWinAspectRatio : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinAspectRatio(const QString &title, QWidget *parent = 0, VWData* data = 0);

    float getSpecificAR( );

signals:

public slots:

private slots:
   void aspectRatioTypeChanged( );

private:
   VWData        *m_pData;

   QRadioButton* radioButtonNoMaintain;
   QRadioButton* radioButtonMaintainSrc;
   QRadioButton* radioButtonMaintainFixed;
   QSpinBox*     widthRatioSpinBox;
   QSpinBox*     heightRatioSpinBox;
   QLabel*       ratioDividerLabel;
};
#endif /*ASPECTRATIO_H_*/
