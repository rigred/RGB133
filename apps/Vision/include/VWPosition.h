
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
class QPoint;
QT_END_NAMESPACE

class VWData;

class VisionWinPosition : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinPosition(const QString &title,
          QPair<int, int>& position, QPair<int, int>& positionMax,
          VWData* data, QWidget *parent = 0);

    QPoint getPosition() const;

signals:
    void valueChanged(int value);

public slots:
    void topSetValue(int value);
    void leftSetValue(int value);

private slots:
   void positionChanged( );

private:
    QWidget   *p;
    VWData    *m_pData;

    QLabel    *m_pTopLabel;
    QLabel    *m_pLeftLabel;
    QSpinBox  *m_pTopSpinBox;
    QSpinBox  *m_pLeftSpinBox;

};

#endif /*VIDTIMING_H_*/
