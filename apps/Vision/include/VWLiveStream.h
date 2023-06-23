
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QGroupBox>

#include "VWData.h"

QT_BEGIN_NAMESPACE
class QRadioButton;
QT_END_NAMESPACE

class VWData;

class VisionWinLiveStream : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinLiveStream(const QString &title, VWData* data, QWidget *parent = 0);

signals:

public slots:
   void setLiveStream( );
   void updateLiveStream(eVWLiveStream liveStream);

private slots:

private:
    VWData         *m_pData;

    QRadioButton* radioButtonOff;
    QRadioButton* radioButtonOn;
};

#endif /* LIVESTREAM_H */
