
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>

#include <stdio.h>

#include "VisionWindowDialog.h"
#include "VWVideoWidget.h"

QT_BEGIN_NAMESPACE
class QRect;
class QBool;
QT_END_NAMESPACE

#define VW_DEFAULT_X        100
#define VW_DEFAULT_Y        100
#define VW_DEFAULT_WIDTH    1920
#define VW_DEFAULT_HEIGHT   1080
#define VW_MIN_WIDTH          1
#define VW_MIN_HEIGHT         1
#define VW_MAX_WIDTH       4096
#define VW_MAX_HEIGHT      4096

class VisionWindowDialog;
class VWAboutDialog;
class VWData;

class VisionWindow : public QMainWindow
{
   Q_OBJECT

public:
   VisionWindow();
   ~VisionWindow();

   // Activation methods
   bool isOk() { return m_bOk; };

   // Focus methods
   bool isHandlingFocusEvents( ) { return m_bHandleFocusEvents; };
   void handleFocusEvents(bool handle) { m_bHandleFocusEvents = handle; };
   bool inFocus( ) { return m_bInFocus; };

   // Accessor methods
   QRect& getCurrentFramePosition();
   QRect& getMaxWindowPosition();
   QRect& getCurrentClientSize();

   void getCropping(sVWAOI *pAOI) { m_pVideoWidget->getCropping(pAOI); };
   void setCropping(sVWAOI AOI) { m_pVideoWidget->setCropping(AOI); };

   // Setter methods
   void setWindowFramePosition(const QPoint &pos);
   void setWindowFramePosition(int x, int y);
   void setWindowFrameSize(int width, int height);

   void moveFrame(QPoint &pos);
   void resizeFrame(QRect &pos);

   // Helper methods
   void setRestartVideo( ) { m_pVideoWidget->setRestartVideo(); };
   void waitForVideoRestart( ) { m_pVideoWidget->waitForVideoRestart(); };
   void restartVideo( ) { m_pVideoWidget->restartVideo(); };
   void stopVideo( ) { m_pVideoWidget->stopVideo(); };

   void showMenu(bool show);
   void onTop(bool top);

   void setTitle(const QString& caption);

   void setCaptureRate(int rate, double freq);

   void setPixelFormat( );

   void getLiveStream(unsigned long *livestream);
   void setLiveStream(unsigned long livestream);

signals:
   void applyCaptureRate(bool active);

public slots:
   void openWinProp( );
   void openInpProp( );
   void pauseOrRun( );
   void showAbout( );

protected:
   void mousePressEvent(QMouseEvent *event);
   void keyPressEvent(QKeyEvent *event);
   void moveEvent(QMoveEvent *event);
   void resizeEvent(QResizeEvent *event);
   void contextMenuEvent(QContextMenuEvent *event);
   void focusInEvent(QFocusEvent* event);
   void focusOutEvent(QFocusEvent* event);

private slots:
   void newSourceIndex();

private:
   // Private constructor actions
   void createActions();
   void createMenus();

   // Menus and Actions
   QMenu               *m_pFileMenu;
   QMenu               *m_pWindowMenu;
   QMenu               *m_pHelpMenu;
   QAction             *m_pExitAct;
   QAction             *m_pWinPropAct;
   QAction             *m_pInpPropAct;
   QAction             *m_pSaveImgAsAct;
   QAction             *m_pSaveSettingsAct;
   QAction             *m_pSaveSettingsAsAct;
   QAction             *m_pPauseRunAct;
   QAction             *m_pPrintAct;
   QAction             *m_pPageSetupAct;
   QAction             *m_pAboutAct;

   bool                 m_bPaused;

   // Window Dialogs
   VisionWindowDialog  *m_pVWDialog;
   VWAboutDialog       *m_pAboutDialog;
   bool                 m_bInFocus;
   bool                 m_bHandleFocusEvents;

   // QImage Video Widget
   VWVideoWidget       *m_pVideoWidget;

   // Data class
   VWData              *m_pData;

   bool                 m_bOk;

   QRect                m_frame;
   QRect                m_frameMax;
   QRect                m_pos;

   int                  m_title;
   int                  m_menu;
   int                  m_borders;
};

#endif /*MAINWINDOW_H_*/
