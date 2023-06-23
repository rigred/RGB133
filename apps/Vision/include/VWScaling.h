
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SCALING_H
#define SCALING_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QRadioButton;
QT_END_NAMESPACE

class VisionWinScaling : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinScaling(const QString &title, QWidget *parent = 0);

signals:

public slots:

private slots:

private:
    QRadioButton* radioButtonFast;
    QRadioButton* radioButtonSlow;
};

#endif /* SCALING_H */
