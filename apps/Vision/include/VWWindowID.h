
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef WINDOWID_H_
#define WINDOWID_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QLabel;
QT_END_NAMESPACE

class VisionWinWindowID : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinWindowID(const QString &title, QWidget *parent = 0);

signals:
    void valueChanged(int value);

public slots:
    void idSetValue(int value);

private:
    QLabel*    idLabel;
    QSpinBox*  idSpinBox;
};

#endif /*WINDOWID_H_*/
