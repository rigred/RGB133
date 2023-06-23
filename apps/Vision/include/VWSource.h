
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SOURCE_H
#define SOURCE_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QScrollBar;
class QSlider;
class QLabel;
class QComboBox;
QT_END_NAMESPACE

class VWData;

class VisionWinSource : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinSource(const QString &title, VWData* data, QWidget *parent = 0);

signals:

public slots:

private:
    QWidget* p;

    VWData     *m_pData;

    QLabel *label;
    QComboBox* comboBox;

    int inputs;
};

#endif /* SOURCE_H */
