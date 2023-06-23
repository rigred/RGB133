
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VIDTIMING_H_
#define VIDTIMING_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QLabel;
class QCheckBox;
class QLineEdit;
QT_END_NAMESPACE

class VWData;

class VisionWinVidTimingBase
{
public:
   virtual void setSpinBoxValues(QSpinBox *pScrollBar, int min, int val, int max);
};

class VisionWinVidTiming : public QGroupBox, VisionWinVidTimingBase
{
    Q_OBJECT

public:
    VisionWinVidTiming(const QString &title, VWData* data, QWidget *parent = 0);

signals:

public slots:
   void inputPropertiesReset();
   void vidTimingTypeChanged();

private slots:
   void vidTimingWidthChanged();
   void vidTimingHeightChanged();

private:
   void setDefaults( );

   QWidget   *p;
   VWData    *m_pData;

   QLabel    *m_pWidthLabel;
   QLabel    *m_pHeightLabel;
   QLabel    *m_pRefreshLabel;
   QLabel    *m_pHzLabel;
   QSpinBox  *m_pWidthSpinBox;
   QSpinBox  *m_pHeightSpinBox;
   QLineEdit *m_pRefreshLineEdit;
};

#endif /*VIDTIMING_H_*/
