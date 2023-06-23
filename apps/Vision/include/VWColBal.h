
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWCOLBAL_H_
#define VWCOLBAL_H_

#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QScrollBar;
QT_END_NAMESPACE

class VWData;

typedef enum _eVWColBal
{
   VW_COL_BAL_ALL,
   VW_COL_BAL_RED,
   VW_COL_BAL_GREEN,
   VW_COL_BAL_BLUE,
} eVWColBal;

typedef enum _eVWColBalType
{
   VW_COL_BAL_UNKNOWN,
   VW_COL_BAL_RED_BRIGHTNESS,
   VW_COL_BAL_GREEN_BRIGHTNESS,
   VW_COL_BAL_BLUE_BRIGHTNESS,
   VW_COL_BAL_RED_CONTRAST,
   VW_COL_BAL_GREEN_CONTRAST,
   VW_COL_BAL_BLUE_CONTRAST,
} eVWColBalType;

class VWData;

class VisionWinColBal : public QGroupBox
{
    Q_OBJECT

public:
    VisionWinColBal(const QString &title, eVWColBal col, VWData* data, QWidget *parent = 0);

    QScrollBar* getBrightnessScrollBar( ) { return m_pBrightnessScrollBar; };
    QScrollBar* getContrastScrollBar( ) { return m_pContrastScrollBar; };

    void rejected(eVWColBalType brType, eVWColBalType ctType);
    void reset(eVWColBalType brType, eVWColBalType ctType);

private:
    VWData      *m_pData;
    QScrollBar  *m_pBrightnessScrollBar;
    QScrollBar  *m_pContrastScrollBar;
    
    QLabel      *m_pBrightnessValue;
    QLabel      *m_pContrastValue;
};

#endif /*VWCOLBAL_H_*/
