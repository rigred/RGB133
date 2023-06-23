
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef CAPTUREFORMAT_H
#define CAPTUREFORMAT_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QRadioButton;
QT_END_NAMESPACE

class VWData;

class VisionWinCaptureFormat : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinCaptureFormat(const QString &title, VWData* data, QWidget *parent = 0);

signals:

public slots:
   void setPixFmt( );

private slots:

private:
   VWData       *m_pData;

    QRadioButton* radioButtonAuto;
    QRadioButton* radioButton555;
    QRadioButton* radioButton565;
    QRadioButton* radioButton888;
};

#endif /* CAPTUREFORMAT_H */
