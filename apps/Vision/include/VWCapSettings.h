
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef CAPTION_H_
#define CAPTION_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QRadioButton;
class QPushButton;
QT_END_NAMESPACE

class VWData;

class VisionWinCapSettings : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinCapSettings(const QString &title, VWData* data, QWidget *parent = 0);

signals:

public slots:
   void capSettingsTypeChanged( );

private:
   VWData        *m_pData;

   QRadioButton  *m_pSharedRadioButton;
   QRadioButton  *m_pUniqueRadioButton;
   QPushButton   *m_pPushButton;
};

#endif /*CAPTION_H_*/
