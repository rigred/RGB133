
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <QtWidgets>

#include "VWConfig.h"
#include "VisionWindow.h"
#include "VisionWindowDialog.h"
#include "VWData.h"
#include "VWWindowTab.h"
#include "VWInputTab.h"
#include "VWSaveChangesDialog.h"

VisionWindowDialog::VisionWindowDialog(QMainWindow *parent, VWData* data, eVWDialogTab tab)
    : p(parent), m_pData(data)
{
   m_pData->getLatestValues();
   m_pSavedData = new VWData(m_pData);

   m_pTabWidget = new QTabWidget;
   m_pWindowTab = new WindowTab(m_pData, this);
   m_pInputTab = new InputTab(m_pData, this);
   m_pTabWidget->addTab(m_pWindowTab, "Window");
   m_pTabWidget->addTab(m_pInputTab, "Input");
   m_pTabWidget->setCurrentIndex(tab);

   m_pInfoLabel = new QLabel(tr("<i>Choose a menu option, or right-click to "
                           "invoke a context menu</i>"));
   m_pInfoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
   m_pInfoLabel->setAlignment(Qt::AlignCenter);

   m_pButtonBox = new QDialogButtonBox(
           QDialogButtonBox::Ok | QDialogButtonBox::Apply |
           QDialogButtonBox::Cancel | QDialogButtonBox::Help);
   m_pApplyButton = m_pButtonBox->button(QDialogButtonBox::Apply);
   m_pApplyButton->setDisabled(true);

   connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accepted()));
   connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(cancel()));
   connect(m_pApplyButton, SIGNAL(clicked()), this, SLOT(apply()));
   connect(m_pData, SIGNAL(enableApply(bool)), this, SLOT(enableApply(bool)));
   connect(m_pData, SIGNAL(inputValuesChanged()), this, SLOT(resaveData()));
   connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(rereadControls()));

   QVBoxLayout *layout = new QVBoxLayout;
   layout->setMargin(0);
   layout->addWidget(m_pTabWidget);
   layout->addWidget(m_pButtonBox);
   setLayout(layout);

   connect(m_pData, SIGNAL(windowPropertiesRestore()), this, SLOT(windowPropertiesRestore()));
   connect(m_pData, SIGNAL(inputPropertiesRestore()), this, SLOT(inputPropertiesRestore()));
}

VisionWindowDialog::~VisionWindowDialog()
{
}

QRect& VisionWindowDialog::getCurrentFramePosition() const
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   return vw->getCurrentFramePosition();
}

QRect& VisionWindowDialog::getMaxWindowPosition() const
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   return vw->getMaxWindowPosition();
}

QRect& VisionWindowDialog::getCurrentClientSize() const
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   return vw->getCurrentClientSize();
}

//Protected methods
void VisionWindowDialog::applyChanges(bool save)
{
   unsigned int flags = m_pData->getModified();
   if(flags)
   {
      // Apply changes
      if(flags & VW_MOD_POSITION)
         positionChanged();
      if(flags & VW_MOD_SIZE)
         sizeChanged();
      if(flags & VW_MOD_STYLE)
         windowChanged();
      if(flags & VW_MOD_CAPTION)
         titleChanged();
      if(flags & VW_MOD_CAPTURE_RATE)
         rateChanged();
      if(flags & VW_MOD_PIXEL_FORMAT)
         pixelFormatChanged();
      if(flags & VW_MOD_LIVE_STREAM)
         liveStreamChanged();

      if(save)
         m_pData->setModified(VW_MOD_NONE);
   }
}

void VisionWindowDialog::apply( )
{
   applyChanges(false);
   enableApply(false);
}

void VisionWindowDialog::accepted( )
{
   applyChanges(true);
   accept();
}

//Public slots
void VisionWindowDialog::enableApply(bool enable)
{
   m_pApplyButton->setDisabled(!enable);
}

void VisionWindowDialog::applyCaptureRate(bool active)
{
   rateRestore(active);
}

//Private slots
void VisionWindowDialog::cancel( )
{
   VWSaveChangesDialog save;
   if(m_pData->getModified())
   {
      if(save.exec() != QDialog::Accepted)
      {
         *m_pData = *m_pSavedData;
         m_pData->setModified(VW_MOD_NONE);
      }
   }
   close();
}

void VisionWindowDialog::positionChanged( )
{
   QPoint pos = m_pWindowTab->getPosition();

   VisionWindow* vw = static_cast<VisionWindow*>(p);

   vw->moveFrame(pos);
}

void VisionWindowDialog::positionRestore( )
{
   QPoint pos(m_pData->getPosition().first, m_pData->getPosition().second);

   VisionWindow* vw = static_cast<VisionWindow*>(p);

   vw->moveFrame(pos);
}

bool VisionWindowDialog::hasWidthChanged(QRect to, QRect from)
{
   if(to.width() != from.width())
   {
      return true;
   }
   return false;
}

bool VisionWindowDialog::hasHeightChanged(QRect to, QRect from)
{
   if(to.height() != from.height())
   {
      return true;
   }
   return false;
}

void VisionWindowDialog::sizeChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   QRect sz = m_pWindowTab->getSize();
   QRect scaledSize = sz;

   float fUseAR = 0.0;

   if(m_pData->getAspectRatioType() != VW_DO_NOT_MAINTAIN_AR)
   {
      if(m_pData->getAspectRatioType() == VW_MAINTAIN_SRC_AR)
         fUseAR = m_pData->getSourceAR();
      else
         fUseAR = m_pWindowTab->getSpecificAR();

      if(fUseAR)
      {
         if(hasWidthChanged(sz, m_pData->getClientPosition()))
         {
            // Width has changed
            scaledSize.setHeight(sz.width()/fUseAR);
         }
         else if(hasHeightChanged(sz, m_pData->getClientPosition()))
         {
            // Height has changed (but not width)
            scaledSize.setWidth(sz.height()*fUseAR);
         }
      }
   }
   m_pData->setClientPosition(scaledSize);
   m_pWindowTab->updateSize(scaledSize);
   vw->resizeFrame(m_pData->getClientPosition());
}

void VisionWindowDialog::sizeRestore( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   QRect sz = m_pData->getClientPosition();

   vw->resizeFrame(sz);
}

void VisionWindowDialog::windowChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   bool showMenu = m_pWindowTab->getShowMenu();
   bool onTop = m_pWindowTab->getAlwaysOnTop();

   vw->showMenu(showMenu);
   vw->onTop(onTop);
}

void VisionWindowDialog::windowRestore( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   bool showMenu = m_pData->getShowMenu();
   bool onTop = m_pData->getAlwaysOnTop();

   vw->showMenu(showMenu);
   vw->onTop(onTop);
}

void VisionWindowDialog::titleChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   QString title = m_pWindowTab->getCaption();

   vw->setTitle(m_pData->getRealCaption(title));
}

void VisionWindowDialog::titleRestore( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   QString title = m_pData->getCaption();

   vw->setTitle(m_pData->getRealCaption(title));
}

void VisionWindowDialog::rateChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   int activeRate = m_pWindowTab->getActiveCaptureRate();
   int inactiveRate = m_pWindowTab->getInactiveCaptureRate();

   if(m_pData->getInactiveRateEnabled())
      vw->setCaptureRate(inactiveRate, m_pData->getVidTimingRefresh());
   else
      vw->setCaptureRate(activeRate, m_pData->getVidTimingRefresh());
}

void VisionWindowDialog::rateRestore(bool active)
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   int activeRate = m_pData->getActiveCaptureRate();
   int inactiveRate = m_pData->getInactiveCaptureRate();

   if(active)
      vw->setCaptureRate(activeRate, m_pData->getVidTimingRefresh());
   else
      vw->setCaptureRate(inactiveRate, m_pData->getVidTimingRefresh());
}

void VisionWindowDialog::pixelFormatChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);

   vw->setPixelFormat();
}

void VisionWindowDialog::liveStreamChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   eVWLiveStream liveStream = m_pData->getLiveStream();

   vw->setLiveStream(liveStream);
}

// At the moment the need is only to read LiveStream from the driver
void VisionWindowDialog::rereadControls( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   eVWLiveStream liveStream;

   vw->getLiveStream((unsigned long *)&liveStream);
   m_pData->setLiveStream(liveStream);
   m_pData->updateLiveStream(liveStream);
}

void VisionWindowDialog::vidTimingRestore( )
{
   m_pData->setVidTimingWidth(m_pData->getVidTimingWidth(), true);
   m_pData->setVidTimingHeight(m_pData->getVidTimingHeight(), true);
}

void VisionWindowDialog::vidAdjRestore( )
{
   m_pData->setHorPos(m_pData->getVidAdjHorPos(), true);
   m_pData->setHorSize(m_pData->getVidAdjHorSize(), true);
   m_pData->setPhase(m_pData->getVidAdjPhase(), true);
   m_pData->setVertPos(m_pData->getVidAdjVertPos(), true);
   m_pData->setBlackLevel(m_pData->getVidAdjBlackLevel(), true);
}

void VisionWindowDialog::rotationRestore( )
{
   //int activeRate = m_pData->getActiveCaptureRate();
}

void VisionWindowDialog::captureSettingsRestore( )
{
   //int activeRate = m_pData->getActiveCaptureRate();
}

void VisionWindowDialog::croppingRestore( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   sVWAOI aoi;

   m_pData->getCropping(&aoi);
   m_pData->setCropping(aoi, m_pData->getCroppingType());

   vw->setCropping(aoi);
}

void VisionWindowDialog::colAdjRestore( )
{
   m_pData->setBrightness(m_pData->getColAdjBrightness(), true);
   m_pData->setContrast(m_pData->getColAdjContrast(), true);
   m_pData->setSaturation(m_pData->getColAdjSaturation(), true);
   m_pData->setHue(m_pData->getColAdjHue(), true);
}

// Private Slots
void VisionWindowDialog::resaveData( )
{
   m_pSavedData->getInputProperties() = m_pData->getInputProperties();
}

void VisionWindowDialog::windowPropertiesRestore( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   unsigned int flags = m_pData->getModified();

   if(flags & VW_MOD_POSITION)
      positionRestore();
   if(flags & VW_MOD_SIZE)
      sizeRestore();
   if(flags & VW_MOD_STYLE)
      windowRestore();
   if(flags & VW_MOD_CAPTION)
      titleRestore();
   if(flags & VW_MOD_CAPTURE_RATE)
      rateRestore(vw->inFocus());
   if(flags & VW_MOD_PIXEL_FORMAT)
      pixelFormatChanged();
}

void VisionWindowDialog::inputPropertiesRestore( )
{
   unsigned int flags = m_pData->getModified();

   if(flags & VW_MOD_VID_TIMINGS)
      vidTimingRestore();
   if(flags & VW_MOD_VID_ADJ)
      vidAdjRestore();
   if(flags & VW_MOD_ROTATION)
      rotationRestore();
   if(flags & VW_MOD_CAPTURE_SETTINGS)
      captureSettingsRestore();
   if(flags & VW_MOD_CROPPING)
      croppingRestore();
   if(flags & VW_MOD_COLOUR_ADJ)
      colAdjRestore();
}
