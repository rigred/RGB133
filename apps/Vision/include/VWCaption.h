
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef CAPTION_H_
#define CAPTION_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QString;
QT_END_NAMESPACE

class VWData;

class VisionWinCaption : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinCaption(const QString &title, VWData* data, QWidget *parent = 0);

    QString getCaption( );

signals:

public slots:

private slots:
   void captionChanged(const QString& caption);

private:
   VWData       *m_pData;

   QLineEdit    *m_pLineEdit;
   QPushButton  *m_pButton;
};

#endif /*CAPTION_H_*/
