
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWCOLOURBALDIALOG_H_
#define VWCOLOURBALDIALOG_H_

#include <QDialog>

QT_BEGIN_NAMESPACE
class QGroupBox;
class QLabel;
class QScrollBar;
class QDialogButtonBox;
QT_END_NAMESPACE

class VisionWinColBal;
class VWAboutDialog;
class VWData;

class VisionWinColourBalDialog : public QDialog
{
   Q_OBJECT

public:
   VisionWinColourBalDialog(const QString &fileName, VWData* data, QWidget *parent = 0);

private slots:
   void showAbout( );
   void accepted( );
   void rejected( );
   void reset( );

   void redColBalBrightnessChanged(int);
   void redColBalContrastChanged(int);
   void greenColBalBrightnessChanged(int);
   void greenColBalContrastChanged(int);
   void blueColBalBrightnessChanged(int);
   void blueColBalContrastChanged(int);

private:
   VWData            *m_pData;
   VWData            *m_pSavedData;

   VWAboutDialog     *m_pAboutDialog;
   QPushButton       *m_pResetButton;

   VisionWinColBal   *m_pAll;
   VisionWinColBal   *m_pRed;
   VisionWinColBal   *m_pGreen;
   VisionWinColBal   *m_pBlue;
   QDialogButtonBox  *m_pActionButtonBox;
};

#endif /*VWCOLOURBALDIALOG_H_*/
