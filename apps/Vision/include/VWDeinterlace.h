
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef DEINTERLACE_H
#define DEINTERLACE_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QRadioButton;
QT_END_NAMESPACE

class VisionWinDeinterlace : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinDeinterlace(const QString &title, QWidget *parent = 0);

signals:

public slots:

private slots:

private:
    QRadioButton* bobRadioButton;
    QRadioButton* weaveRadioButton;
    QRadioButton* noneRadioButton;
};

#endif /* DEINTERLACE_H */
