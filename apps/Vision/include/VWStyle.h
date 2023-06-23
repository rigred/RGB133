
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef STYLE_H
#define STYLE_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QWidget;
class QRadioButton;
class QCheckBox;
QT_END_NAMESPACE

class VWData;

class VisionWinStyle : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinStyle(const QString &title, VWData* data, QWidget *parent = 0);

    bool getShowMenu( );
    bool getAlwaysOnTop( );

signals:

public slots:

private slots:
   void showMenu(int show);
   void onTop(int top);

private:
   QWidget       *p;
   VWData        *m_pData;
   QRadioButton  *m_pRadioButtonBorTitle;
   QRadioButton  *m_pRadioButtonBor;
   QRadioButton  *m_pRadioButtonNoBorNoTitle;
   QCheckBox     *m_pShowMenuCheckBox;
   QCheckBox     *m_pOnTopCheckBox;
};

#endif /* STYLE_H */
