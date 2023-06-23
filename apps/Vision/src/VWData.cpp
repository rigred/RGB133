
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <unistd.h>

#include "VWConfig.h"
#include "VisionWindow.h"
#include "VWData.h"

// Data Access
VWData::VWData(QObject *parent)
   : p(parent)
{
   m_inputProperties.initCtrl();
   /* Gather information on devices in this system */
   m_inputProperties.enumerateVideoDevices();

   m_video_handle = -1;
   m_node = QString();
   m_bExit = false;
}

// Overloaded operators
VWData::VWData(VWData* data)
{
   m_windowProperties = data->getWindowProperties();
   m_inputProperties = data->getInputProperties();
   m_video_handle = data->getVideoDeviceHandle();
   m_node = data->getVideoDeviceNode();
}

VWData::~VWData( )
{
   m_inputProperties.uninitCtrl();
}

void VWData::openVideoDevice( )
{
   unsigned int input = 0;

   m_video_handle = open(m_node.toLocal8Bit().constData(), O_RDWR, 0);
   if(m_video_handle < 0)
   {
      qDebug() << "VWData::openVideoDevice: Failed to open \"" << m_node << "\"";
      quit();
      return;
   }

   if((input = getCurrentInput()) != 0)
      ioctl(m_video_handle, VIDIOC_S_INPUT, &input);

   m_inputProperties.setVideoDeviceHandle(m_video_handle);
}

void VWData::closeVideoDevice(int fd)
{
   if(::close(fd))
   {
      printf("VWData::closeVideoDevice: Failed to close device on handle(%d): %d\n",
            m_video_handle, errno);
   }

   m_video_handle = -1;
   m_inputProperties.setVideoDeviceHandle(-1);
}

// Monitor methods
void VWData::startMonitor( )
{
   setExit(false);
   start();
}

void VWData::stopMonitor( )
{
   setExit(true);
   wait();
}

// Protected members
void VWData::run( )
{
   int retval;

   // Will be used to monitor for events, such as inputs changing.
   while(!m_bExit)
   {
      retval = m_inputProperties.select();
      if(retval == -1)
      {
         perror("VWData::run");
      }
      else if(retval)
      {
         // Read the device and channel
         int device = -1, channel = -1;
         m_inputProperties.readCtrlDeviceEvent(&device, &channel);
         sourceInputChanged();
      }
      else { /* timeout */ }
   }
}

// Public methods
void VWData::getLatestValues( )
{
   QRect frame = getCurrentFramePosition();
   QRect client = getCurrentClientSize();
   QRect max = getMaxWindowPosition();

   m_windowProperties.setPosition(QPair<int, int>(frame.top(), frame.left()));
   m_windowProperties.setPositionMin(QPair<int, int>(0, 0));
   m_windowProperties.setPositionMax(QPair<int, int>(max.height(), max.width()));
   m_windowProperties.setClientPosition(client);

   m_inputProperties.getLatestValues();
}


// Private Methods
QRect& VWData::getCurrentFramePosition() const
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   return vw->getCurrentFramePosition();
}

QRect& VWData::getMaxWindowPosition() const
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   return vw->getMaxWindowPosition();
}

QRect& VWData::getCurrentClientSize() const
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   return vw->getCurrentClientSize();
}

// Getter methods
QString VWData::getRealCaption(QString caption)
{
   QString inputVar("%input%");
   QString output;

   if(caption.contains(inputVar))
   {
      int indexOf = caption.indexOf(inputVar);
      int input = getCurrentIndex()+1;
      QTextStream(&output) << input;
      caption.replace(indexOf, inputVar.length(), output);
   }

   return caption;
}

QString VWData::getRealCaption( )
{
   QString caption = m_windowProperties.getCaption();
   return getRealCaption(caption);
}

void VWData::getCropping(sVWAOI *pAOI) const
{
   m_inputProperties.getCropping(pAOI);
}

/**********************************************************************/

// Window Properties Setter methods

// Input Properties Setter methods
void VWData::setPixelFormat(eVWPixelFormat fmt)
{
   if(fmt != m_windowProperties.getPixelFormat())
   {
      m_windowProperties.setPixelFormat(fmt);
   }
}

void VWData::setAspectRatioType(eVWAspectRatio ar)
{
   if(ar != m_windowProperties.getAspectRatioType())
   {
      m_windowProperties.setAspectRatioType(ar);
      emit aspectRatioTypeChanged();
   }
}

void VWData::setBorderTitleType(eVWBorderTitle bt)
{
   if(bt != m_windowProperties.getBorderTitleType())
   {
      m_windowProperties.setBorderTitleType(bt);
      emit borderTitleTypeChanged();
   }
}

void VWData::setShowMenu(bool show)
{
   if(show != m_windowProperties.getShowMenu())
      m_windowProperties.setShowMenu(show);
}


void VWData::setAlwaysOnTop(bool onTop)
{
   if(onTop != m_windowProperties.getAlwaysOnTop())
      m_windowProperties.setAlwaysOnTop(onTop);
}

void VWData::setCropping(sVWAOI AOI, eVWCroppingType type)
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);

   m_inputProperties.setCropping(AOI, type);
   if(type != VW_CROPPING_UNSET)
      vw->setCropping(AOI);
}

// Modification methods
unsigned int VWData::getModified( )
{
   return (m_windowProperties.getModified() | m_inputProperties.getModified());
}

unsigned int VWData::getModified(unsigned int mask)
{
   return ( mask & getModified() );
}

void VWData::setModified(unsigned int modified)
{
   if(modified)
   {
      if(modified & MOD_WINDOW_PROPERTIES_FLAGS)
      {
         m_windowProperties.setModified(modified & MOD_WINDOW_PROPERTIES_FLAGS);
         emit enableApply(true);
      }
      if(modified & MOD_INPUT_PROPERTIES_FLAGS)
         m_inputProperties.setModified(modified & MOD_INPUT_PROPERTIES_FLAGS);
   }
   else
   {
      m_windowProperties.setModified(0);
      m_inputProperties.setModified(0);
      emit enableApply(false);
   }
}

void VWData::clearModified(unsigned int modified)
{
   if(modified)
   {
      if(modified & MOD_WINDOW_PROPERTIES_FLAGS)
      {
         m_windowProperties.clearModified(modified);
      }
      if(modified & MOD_INPUT_PROPERTIES_FLAGS)
         m_inputProperties.clearModified(modified);
   }
}

/**********************************************************************/

// Public slots
void VWData::sourceIndexChanged(int index)
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   sVWAOI aoi;

   vw->stopVideo();

   m_inputProperties.setCurrentIndex(index);
   if(setRequestedIndexDeviceInput(index))
   {
      qDebug() << "VWData::sourceIndexChanged: Failed to refresh index " << index;
      return;
   }
   updateVideoNode();

   openVideoDevice();

   // Read the new device properties
   m_inputProperties.getLatestValues();
   vw->getCropping(&aoi);
   this->setCropping(aoi, VW_CROPPING_UNSET);

   emit inputValuesChanged();
   emit newSourceIndex();

   vw->restartVideo();
}

void VWData::sourceInputChanged( )
{
   VisionWindow* vw = static_cast<VisionWindow*>(p);
   sVWAOI aoi;

   vw->stopVideo();

   if(setRequestedIndexDeviceInput(m_inputProperties.getCurrentIndex()))
   {
      qDebug() << "VWData::sourceInputChanged: Failed to refresh index " << 
            m_inputProperties.getCurrentIndex();
      return;
   }
   updateVideoNode();

   openVideoDevice();

   // Read the new device properties
   m_inputProperties.getLatestValues();
   vw->getCropping(&aoi);
   setCropping(aoi, VW_CROPPING_CURRENT);

   emit inputValuesChanged();
   emit newSourceIndex();

   // Must emit signal bcause of multi-threading
   vw->setRestartVideo();
   emit restartVideo();

   // Wait for restart to complete in another thread
   vw->waitForVideoRestart();
}

void VWData::reset( )
{
   emit inputPropertiesReset();
}

/**********************************************************************/

// Overloaded operators
VWData& VWData::operator=(VWData& data)
{
   m_windowProperties = data.getWindowProperties();
   m_inputProperties = data.getInputProperties();

   m_video_handle = data.getVideoDeviceHandle();
   m_node = data.getVideoDeviceNode();

   emit windowPropertiesRestore();
   emit inputPropertiesRestore();

   return *this;
}

/**********************************************************************/
