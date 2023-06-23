
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INVALIDINPUT_H_
#define INVALIDINPUT_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
QT_END_NAMESPACE

class VisionWinInvalidInput : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinInvalidInput(const QString &title, QWidget *parent = 0);

signals:

public slots:

private:
    QLabel*       label;
    QLabel*       resolution;
    QLineEdit*    lineEdit;
};

#endif /*INVALIDINPUT_H_*/
