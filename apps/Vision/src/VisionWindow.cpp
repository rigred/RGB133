
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include <linux/videodev2.h>

#include "VWConfig.h"
#include "VisionWindow.h"
#include "VWInputSelectDialog.h"
#include "VWAboutDialog.h"
#include "VWVideoWidget.h"
#include "VWData.h"

VisionWindow::VisionWindow() : m_bOk(false)
{
   QWidget *widget = new QWidget;
   VWInputSelectDialog* inputSelect = 0;;
   int index = -1;

   setCentralWidget(widget);
   setFocusPolicy(Qt::StrongFocus);

   m_title = true;
   m_menu = true;
   m_borders = true;
   m_bPaused = false;
   m_bInFocus = false;
   m_bHandleFocusEvents = true;

   m_pVideoWidget = NULL;
   m_pVWDialog = NULL;
   m_pAboutDialog = NULL;
   m_pData = new VWData(this);

   inputSelect = new VWInputSelectDialog(this, m_pData);
   if(m_pData->getNumInputs() > 1)
      inputSelect->exec();

   index = inputSelect->getInputIndex();
   if(index != -1)
   {
      sVWAOI aoi;
      m_pData->setCurrentIndex(index);
      m_pData->startMonitor();
      
      m_pData->setRequestedIndexDeviceInput(index);
      m_pData->updateVideoNode();

      m_pData->openVideoDevice();

      m_pVideoWidget = new VWVideoWidget(this, m_pData);

      m_pVWDialog = NULL;

      QVBoxLayout *layout = new QVBoxLayout;
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(m_pVideoWidget);

      createActions();
      createMenus();

      setWindowTitle(m_pData->getRealCaption());

      setMinimumSize(VW_MIN_WIDTH, VW_MIN_HEIGHT);
      setMaximumSize(VW_MAX_WIDTH, VW_MAX_HEIGHT);
      m_frameMax = QRect(0, 0, VW_MAX_WIDTH, VW_MAX_HEIGHT);

      setGeometry(VW_DEFAULT_X, VW_DEFAULT_Y, VW_DEFAULT_WIDTH, VW_DEFAULT_HEIGHT);

      widget->setLayout(layout);

      m_pVideoWidget->startVideo();
      m_pVideoWidget->getCropping(&aoi);
      m_pData->setCropping(aoi, VW_CROPPING_UNSET);

      m_bOk = true;

      connect(m_pData, SIGNAL(newSourceIndex()), this, SLOT(newSourceIndex()));
   }
}

VisionWindow::~VisionWindow()
{
   m_pData->stopMonitor();

   if(m_pVideoWidget)
      m_pVideoWidget->stopVideo();
}

void VisionWindow::createActions()
{
    m_pExitAct = new QAction(tr("E&xit"), this);
    m_pExitAct->setShortcut(tr("Ctrl+Q"));
    m_pExitAct->setStatusTip(tr("Exit the application"));
    connect(m_pExitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_pWinPropAct = new QAction(tr("Window Properties"), this);
    m_pWinPropAct->setShortcut(tr("Ctrl+W"));
    m_pWinPropAct->setStatusTip(tr("Open the Window Properties tab"));
    connect(m_pWinPropAct, SIGNAL(triggered()),
          this, SLOT(openWinProp()));
    m_pInpPropAct = new QAction(tr("Input Properties"), this);
    m_pInpPropAct->setShortcut(tr("Ctrl+I"));
    m_pInpPropAct->setStatusTip(tr("Open the Input Properties tab"));
    connect(m_pInpPropAct, SIGNAL(triggered()),
          this, SLOT(openInpProp()));

    m_pSaveImgAsAct = new QAction(tr("Save Image As..."), this);
    m_pSaveImgAsAct->setStatusTip(tr("Save current captured frame to file"));
    m_pSaveImgAsAct->setEnabled(false);

    m_pSaveSettingsAct = new QAction(tr("Save Settings..."), this);
    m_pSaveSettingsAct->setStatusTip(tr("Save current window/input settings"));
    m_pSaveSettingsAct->setEnabled(false);
    m_pSaveSettingsAsAct = new QAction(tr("Save Settings As..."), this);
    m_pSaveSettingsAsAct->setStatusTip(tr("Save current window/input settings"));
    m_pSaveSettingsAsAct->setEnabled(false);

    m_pPauseRunAct = new QAction(tr("Pause"), this);
    m_pPauseRunAct->setStatusTip(tr("Pause/Run the current capture"));
    connect(m_pPauseRunAct, SIGNAL(triggered()),
          this, SLOT(pauseOrRun()));

    m_pPrintAct = new QAction(tr("Print..."), this);
    m_pPrintAct->setStatusTip(tr("Print the current window"));
    m_pPrintAct->setEnabled(false);
    m_pPageSetupAct = new QAction(tr("Page Setup..."), this);
    m_pPageSetupAct->setStatusTip(tr("Show the page setup dialog"));
    m_pPageSetupAct->setEnabled(false);

    m_pAboutAct = new QAction(tr("About"), this);
    m_pAboutAct->setStatusTip(tr("Application information"));
    connect(m_pAboutAct, SIGNAL(triggered()),
          this, SLOT(showAbout()));
}

void VisionWindow::createMenus()
{
    m_pFileMenu = menuBar()->addMenu(tr("&File"));
    m_pFileMenu->addAction(m_pSaveImgAsAct);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(m_pSaveSettingsAct);
    m_pFileMenu->addAction(m_pSaveSettingsAsAct);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(m_pPauseRunAct);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(m_pPrintAct);
    m_pFileMenu->addAction(m_pPageSetupAct);
    m_pFileMenu->addSeparator();
    m_pFileMenu->addAction(m_pExitAct);

    m_pWindowMenu = menuBar()->addMenu(tr("&Window"));
    m_pWindowMenu->addAction(m_pWinPropAct);
    m_pWindowMenu->addAction(m_pInpPropAct);

    m_pHelpMenu = menuBar()->addMenu(tr("&Help"));
    m_pHelpMenu->addAction(m_pAboutAct);
}

void VisionWindow::mousePressEvent(QMouseEvent *event)
{
   switch(event->button())
   {
      default:
         event->ignore();
         break;
   }
}

void VisionWindow::keyPressEvent(QKeyEvent* event)
{
   switch(event->key()) {
      case Qt::Key_Escape:
         close();
         break;
      default:
         event->ignore();
         break;
   }
}

void VisionWindow::moveEvent(QMoveEvent *event)
{
   setWindowFramePosition(event->pos());
}

void VisionWindow::resizeEvent(QResizeEvent *event)
{
   QRect geom = geometry();
   setWindowFrameSize(geom.width(), geom.height());
   setWindowFramePosition(geom.left(), geom.top());
}

void VisionWindow::contextMenuEvent(QContextMenuEvent *event)
{
   QMenu menu(this);
   m_bHandleFocusEvents = false;
   menu.addMenu(m_pFileMenu);
   menu.addSeparator();
   menu.addAction(m_pWinPropAct);
   menu.addAction(m_pInpPropAct);
   menu.addSeparator();
   menu.addMenu(m_pHelpMenu);
   menu.exec(event->globalPos());
   m_bHandleFocusEvents = true;
}

void VisionWindow::focusInEvent(QFocusEvent* event)
{
   if(m_bHandleFocusEvents)
   {
      m_bInFocus = true;
      // When we come into focus, do we have to adjust rate?
      if(m_pData->getInactiveRateEnabled())
         emit applyCaptureRate(true);
   }
}

void VisionWindow::focusOutEvent(QFocusEvent* event)
{
   if(m_bHandleFocusEvents)
   {
      m_bInFocus = false;
      // When we come into focus, do we have to adjust rate?
      if(m_pData->getInactiveRateEnabled())
         emit applyCaptureRate(false);
   }
}

// Private Slots
void VisionWindow::newSourceIndex()
{
   this->setTitle(m_pData->getRealCaption(m_pData->getCaption()));
}

// Accessor methods
QRect& VisionWindow::getCurrentFramePosition()
{
   m_frame = frameGeometry();
   return m_frame;
}

QRect& VisionWindow::getMaxWindowPosition()
{
   return m_frameMax;
}

QRect& VisionWindow::getCurrentClientSize()
{
   m_pos = geometry();
   return m_pos;
}

// Settor methods
void VisionWindow::setWindowFramePosition(const QPoint &pos)
{
   m_frame.moveLeft(pos.x()), m_frame.moveTop(pos.y());
}

void VisionWindow::setWindowFramePosition(int x, int y)
{
   m_frame.moveLeft(x), m_frame.moveTop(y);
}

void VisionWindow::setWindowFrameSize(int width, int height)
{
   m_pos.setWidth(width), m_pos.setHeight(height);
}

void VisionWindow::moveFrame(QPoint &pos)
{
   QRect geom = geometry();
   int xDelta = 0;
   int yDelta = 0;

   xDelta = pos.x() - m_frame.left();
   yDelta = pos.y() - m_frame.top();

   /* Move the deltas off the client position */
   setGeometry(geom.left()+xDelta, geom.top()+yDelta, geom.width(), geom.height());
}

void VisionWindow::resizeFrame(QRect &pos)
{
   QRect geom = geometry();
   int wDelta = 0;
   int hDelta = 0;

   wDelta = pos.width() - m_pos.width();
   hDelta = pos.height() - m_pos.height();

   /* Move the deltas off the client position */
   setGeometry(geom.left(), geom.top(), geom.width()+wDelta, geom.height()+hDelta);
}

// Helper methods
void VisionWindow::showMenu(bool show)
{
   if(menuBar()->isVisible())
   {
      if(!show)
      {
         menuBar()->setVisible(false);
      }
   }
   else
   {
      if(show)
         menuBar()->setVisible(true);
   }
}

void VisionWindow::onTop(bool top)
{
   Qt::WindowFlags flags = this->windowFlags();
   if(flags & (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint))
   {
      if(!top)
         this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
   }
   else
   {
      if(top)
         this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
   }
   this->show();
}

void VisionWindow::setTitle(const QString& caption)
{
   setWindowTitle(caption);
}

void VisionWindow::setCaptureRate(int rate, double freq)
{
   if(rate > 0)
      m_pVideoWidget->setCaptureRate(rate, freq);
}

void VisionWindow::setPixelFormat( )
{
   m_pVideoWidget->restartVideo();
}

void VisionWindow::getLiveStream(unsigned long *livestream)
{
   m_pVideoWidget->getLiveStream(livestream);
}

void VisionWindow::setLiveStream(unsigned long livestream)
{
   m_pVideoWidget->setLiveStream(livestream);
}

// Public slots
void VisionWindow::openWinProp( )
{
   m_bHandleFocusEvents = false;
   m_pVWDialog = new VisionWindowDialog(this, m_pData, WIN_PROP_TAB);
   connect(this, SIGNAL(applyCaptureRate(bool)), m_pVWDialog, SLOT(applyCaptureRate(bool)));
   m_pVWDialog->exec();
   m_bHandleFocusEvents = true;
}

void VisionWindow::openInpProp( )
{
   m_bHandleFocusEvents = false;
   m_pVWDialog = new VisionWindowDialog(this, m_pData, INP_PROP_TAB);
   connect(this, SIGNAL(applyCaptureRate(bool)), m_pVWDialog, SLOT(applyCaptureRate(bool)));
   m_pVWDialog->exec();
   m_bHandleFocusEvents = true;
}

void VisionWindow::pauseOrRun( )
{
   if(m_bPaused)
   {
      m_pPauseRunAct->setText(QString("Pause"));
      m_pVideoWidget->runVideo();
   }
   else
   {
      m_pPauseRunAct->setText(QString("Run"));
      m_pVideoWidget->pauseVideo();
   }
   m_bPaused = !m_bPaused;
}

void VisionWindow::showAbout( )
{
   m_bHandleFocusEvents = false;
   m_pAboutDialog = new VWAboutDialog(this, m_pData);
   m_pAboutDialog->exec();
   m_bHandleFocusEvents = true;
}

