
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef COLOURADJ_H
#define COLOURADJ_H

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QScrollBar;
class QComboBox;
class QPushButton;
class QStackedWidget;
QT_END_NAMESPACE

class VWData;
class VisionWinColourBalDialog;

typedef enum _eVWStackVidType
{
   VW_COL_ADJ_STACK_DVI_VID = 0,
   VW_COL_ADJ_STACK_NO_SIG = 0,
   VW_COL_ADJ_STACK_ANALOG,
} eVWStackVidType;

class VisionWinColourAdjVid : public QWidget
{
    Q_OBJECT

public:
    VisionWinColourAdjVid(VWData* data, QWidget *parent = 0);

    void setDefaults( );

signals:

public slots:
   void vidTypeChanged(int type);

private slots:

private:
   QWidget     *p;
   VWData      *m_pData;

   QLabel      *m_pBrightnessValue;
   QLabel      *m_pContrastValue;

   QScrollBar  *m_pBrightnessScrollBar;
   QScrollBar  *m_pContrastScrollBar;
};

class VisionWinColourAdjVidExt : public QWidget
{
    Q_OBJECT

public:
    VisionWinColourAdjVidExt(VWData* data, QWidget *parent = 0);

    void setDefaults( );

signals:

public slots:
   void vidTypeChanged(int type);

private slots:

private:
   QWidget     *p;
   VWData      *m_pData;

   QLabel      *m_pBrightnessValue;
   QLabel      *m_pContrastValue;
   QLabel      *m_pSaturationValue;
   QLabel      *m_pHueValue;

   QScrollBar  *m_pBrightnessScrollBar;
   QScrollBar  *m_pContrastScrollBar;
   QScrollBar  *m_pSaturationScrollBar;
   QScrollBar  *m_pHueScrollBar;
};

class VisionWinColourAdj : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinColourAdj(const QString &title, VWData* data, QWidget *parent = 0);

signals:
   void vidTypeChanged(int type);
   void stackChanged(int index);

public slots:
   void inputPropertiesReset( );

   void colAdjTypeChanged();
   void pressed(void);

   void brightnessChanged(int);
   void contrastChanged(int);
   void saturationChanged(int);
   void hueChanged(int);

private slots:

private:
   QWidget                   *p;
   VWData                    *m_pData;

   QStackedWidget            *m_pStack;

   VisionWinColourAdjVid     *m_pVidAdj;
   VisionWinColourAdjVidExt  *m_pVidAdjExt;

   VisionWinColourBalDialog  *m_pDialog;
   QPushButton               *m_pButton;
};

#endif /* COLOURADJ_H */
