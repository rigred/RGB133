
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWSIZE_H_
#define VWSIZE_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QLabel;
class QCheckBox;
class QRect;
QT_END_NAMESPACE

class VWData;

class VisionWinSize : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinSize(const QString &title, QRect &pos, QPair<int, int>& positionMax,
          VWData* data, QWidget *parent = 0);

    QWidget* getParent() const { return p; };

    QRect getSize() const;
    void updateSize(QRect& sz);

signals:
    void valueChanged(int value);

public slots:
    void widthSetValue(int value);
    void heightSetValue(int value);

private slots:
   void sizeChanged( );

private:
    QWidget    *p;
    VWData     *m_pData;

    QLabel     *m_pWidthLabel;
    QLabel     *m_pHeightLabel;
    QLabel     *m_pExBorderLabel;
    QSpinBox   *m_pWidthSpinBox;
    QSpinBox   *m_pHeightSpinBox;
    QCheckBox  *m_pExBorderCheckBox;
};

#endif /* VWSIZE_H_ */
