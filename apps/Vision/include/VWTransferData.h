
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef TRANSFERDATA_H
#define TRANSFERDATA_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QRadioButton;
QT_END_NAMESPACE

class VisionWinTransferData : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinTransferData(const QString &title, QWidget *parent = 0);

signals:

public slots:

private slots:

private:
    QRadioButton* radioButtonDirect;
    QRadioButton* radioButtonSystem;
};

#endif /* TRANSFERDATA_H */
