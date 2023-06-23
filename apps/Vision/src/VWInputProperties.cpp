
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/videodev2.h>

#include "VWConfig.h"
#include "VWInputProperties.h"
#include "VWData.h"

#include "rgb133v4l2.h"
#include "rgb133capparms.h"

typedef std::vector<std::string> device_vec;

/**********************************************************************/

// Input Properties
VWInputProperties::VWInputProperties( )
{
   m_ctrl_handle = -1;
   m_video_handle = -1;

   m_modified = VW_MOD_NONE;
   m_device = -1;
   m_channel = -1;
   m_index = -1;
   m_client = -1;

   for(int i=0; i<MAX_VIDEO_DEVICES_SYSTEM; i++)
   {
      for(int j=0; j<MAX_CHANNELS_PER_DEVICE; j++)
      {
         for(int k=0; k<MAX_CLIENTS_PER_CHANNEL; k++)
         {
            m_CroppingType[i][j][k] = VW_CROPPING_UNSET;
         }
      }
   }

   memset(&m_SystemInfo, 0, sizeof(sSystemInfo));
   memset(m_Device, 0, (MAX_VIDEO_DEVICES_SYSTEM * sizeof(sDeviceInfo)));
}

void VWInputProperties::initCtrl( )
{
   // Open up the control device
   m_ctrl_handle = openCtrlDevice();
}

VWInputProperties::~VWInputProperties( )
{
}

void VWInputProperties::uninitCtrl( )
{
}

int VWInputProperties::select( )
{
   fd_set rfds;
   struct timeval tv;

   FD_ZERO(&rfds);
   FD_SET(m_ctrl_handle, &rfds);

   tv.tv_sec = 0;
   tv.tv_usec = 50000;
   return ::select(m_ctrl_handle+1, &rfds, 0, 0, &tv);
}

// Public Getter methods
void VWInputProperties::getLatestValues()
{
   // Read the device input information
   getLatestValues(m_device, m_channel, m_client);
}

void VWInputProperties::getLatestValues(int device, int channel, int client)
{
   // Read the device input information
   getDevicePropertiesAll(device, channel, client);
}

QString VWInputProperties::getVersion( )
{
   unsigned int ver_uint = m_SystemInfo.version;
   return QString("%1.%2.%3")
            .arg((ver_uint >> 16) & 0xFF)
            .arg((ver_uint >> 8) & 0xFF)
            .arg(ver_uint & 0xFF);
}

QString VWInputProperties::getCurrentIndexNode()
{
   if(m_device >= 0)
      return QString(m_Device[m_device].node);
   else
      return QString("Error");
}

enum _eVWSignalType VWInputProperties::getSignalType() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.type;
}

enum _eVWVidStd VWInputProperties::getVideoStd() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoStandard;
}

int VWInputProperties::getVidTimingWidth() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorAddrTime;
}

int VWInputProperties::getVidTimingWidthMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.VideoTimings.HorAddrTime;
}

int VWInputProperties::getVidTimingWidthMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.VideoTimings.HorAddrTime;
}

int VWInputProperties::getVidTimingWidthDef() const
{
   return m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.HorAddrTime;
}

int VWInputProperties::getVidTimingHeight() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerAddrTime;
}

int VWInputProperties::getVidTimingHeightMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.VideoTimings.VerAddrTime;
}

int VWInputProperties::getVidTimingHeightMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.VideoTimings.VerAddrTime;
}

int VWInputProperties::getVidTimingHeightDef() const
{
   return m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.VerAddrTime;
}

double VWInputProperties::getVidTimingRefresh() const
{
   return (double)m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerFrequency/(double)1000;
}

int VWInputProperties::getVidAdjHorPos() const
{
   return ( m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorBackPorch +
            m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorSyncTime );
}

int VWInputProperties::getVidAdjHorPosMin() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorSyncTime;
}

int VWInputProperties::getVidAdjHorPosMax() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.PixelClock;
}

int VWInputProperties::getVidAdjHorPosDef() const
{
   return ( m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.HorBackPorch +
            m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.HorSyncTime );
}

int VWInputProperties::getVidAdjHorSize() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.PixelClock;
}

int VWInputProperties::getVidAdjHorSizeMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.VideoTimings.PixelClock;
}

int VWInputProperties::getVidAdjHorSizeMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.VideoTimings.PixelClock;
}

int VWInputProperties::getVidAdjHorSizeDef() const
{
   return m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.PixelClock;
}

int VWInputProperties::getVidAdjPhase() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.Phase;
}

int VWInputProperties::getVidAdjPhaseMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Phase;
}

int VWInputProperties::getVidAdjPhaseMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.Phase;
}

int VWInputProperties::getVidAdjPhaseDef() const
{
   return m_Device[m_device].Channel[m_channel].defDeviceParms.Phase;
}

int VWInputProperties::getVidAdjVertPos() const
{
   return ( m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerBackPorch +
            m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerSyncTime );
}

int VWInputProperties::getVidAdjVertPosMin() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerSyncTime;
}

int VWInputProperties::getVidAdjVertPosMax() const
{
   return ( m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerBackPorch +
            m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerFrontPorch +
            m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerSyncTime );
}

int VWInputProperties::getVidAdjVertPosDef() const
{
   return ( m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.VerBackPorch +
            m_Device[m_device].Channel[m_channel].detDeviceParms.VideoTimings.VerSyncTime );
}

int VWInputProperties::getVidAdjBlackLevel() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.Blacklevel;
}

int VWInputProperties::getVidAdjBlackLevelMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Blacklevel;
}

int VWInputProperties::getVidAdjBlackLevelMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.Blacklevel;
}

int VWInputProperties::getVidAdjBlackLevelDef() const
{
   return m_Device[m_device].Channel[m_channel].defDeviceParms.Blacklevel;
}

int VWInputProperties::getColAdjBrightness() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.Brightness;
}

int VWInputProperties::getColAdjBrightnessMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Brightness;
}

int VWInputProperties::getColAdjBrightnessMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.Brightness;
}

int VWInputProperties::getColAdjBrightnessDef() const
{
   return m_Device[m_device].Channel[m_channel].defDeviceParms.Brightness;
}

int VWInputProperties::getColAdjContrast() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.Contrast;
}

int VWInputProperties::getColAdjContrastMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Contrast;
}

int VWInputProperties::getColAdjContrastMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.Contrast;
}

int VWInputProperties::getColAdjContrastDef() const
{
   return m_Device[m_device].Channel[m_channel].defDeviceParms.Contrast;
}

int VWInputProperties::getColAdjSaturation() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.Saturation;
}

int VWInputProperties::getColAdjSaturationMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Saturation;
}

int VWInputProperties::getColAdjSaturationMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.Saturation;
}

int VWInputProperties::getColAdjSaturationDef() const
{
   return m_Device[m_device].Channel[m_channel].defDeviceParms.Saturation;
}

int VWInputProperties::getColAdjHue() const
{
   return m_Device[m_device].Channel[m_channel].curDeviceParms.Hue;
}

int VWInputProperties::getColAdjHueMin() const
{
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Hue;
}

int VWInputProperties::getColAdjHueMax() const
{
   return m_Device[m_device].Channel[m_channel].maxDeviceParms.Hue;
}

int VWInputProperties::getColAdjHueDef() const
{
   return m_Device[m_device].Channel[m_channel].defDeviceParms.Hue;
}

int VWInputProperties::getColBalBrightness(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.RedOffset;
      case VW_COL_BAL_GREEN_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.GreenOffset;
      case VW_COL_BAL_BLUE_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.BlueOffset;
      default:
         return -1;
   }
}

int VWInputProperties::getColBalBrightnessMin(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].minDeviceParms.Colour.RedOffset;
      case VW_COL_BAL_GREEN_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].minDeviceParms.Colour.GreenOffset;
      case VW_COL_BAL_BLUE_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].minDeviceParms.Colour.BlueOffset;
      default:
         return -1;
   }
   return m_Device[m_device].Channel[m_channel].minDeviceParms.Brightness;
}

int VWInputProperties::getColBalBrightnessMax(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].maxDeviceParms.Colour.RedOffset;
      case VW_COL_BAL_GREEN_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].maxDeviceParms.Colour.GreenOffset;
      case VW_COL_BAL_BLUE_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].maxDeviceParms.Colour.BlueOffset;
      default:
         return -1;
   }
}

int VWInputProperties::getColBalBrightnessDef(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].defDeviceParms.Colour.RedOffset;
      case VW_COL_BAL_GREEN_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].defDeviceParms.Colour.GreenOffset;
      case VW_COL_BAL_BLUE_BRIGHTNESS:
         return m_Device[m_device].Channel[m_channel].defDeviceParms.Colour.BlueOffset;
      default:
         return -1;
   }
}

int VWInputProperties::getColBalContrast(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_CONTRAST:
         return m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.RedGain;
      case VW_COL_BAL_GREEN_CONTRAST:
         return m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.GreenGain;
      case VW_COL_BAL_BLUE_CONTRAST:
         return m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.BlueGain;
      default:
         return -1;
   }
}

int VWInputProperties::getColBalContrastMin(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_CONTRAST:
         return m_Device[m_device].Channel[m_channel].minDeviceParms.Colour.RedGain;
      case VW_COL_BAL_GREEN_CONTRAST:
         return m_Device[m_device].Channel[m_channel].minDeviceParms.Colour.GreenGain;
      case VW_COL_BAL_BLUE_CONTRAST:
         return m_Device[m_device].Channel[m_channel].minDeviceParms.Colour.BlueGain;
      default:
         return -1;
   }
}

int VWInputProperties::getColBalContrastMax(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_CONTRAST:
         return m_Device[m_device].Channel[m_channel].maxDeviceParms.Colour.RedGain;
      case VW_COL_BAL_GREEN_CONTRAST:
         return m_Device[m_device].Channel[m_channel].maxDeviceParms.Colour.GreenGain;
      case VW_COL_BAL_BLUE_CONTRAST:
         return m_Device[m_device].Channel[m_channel].maxDeviceParms.Colour.BlueGain;
      default:
         return -1;
   }
}

int VWInputProperties::getColBalContrastDef(eVWColBalType type) const
{
   switch(type)
   {
      case VW_COL_BAL_RED_CONTRAST:
         return m_Device[m_device].Channel[m_channel].defDeviceParms.Colour.RedGain;
      case VW_COL_BAL_GREEN_CONTRAST:
         return m_Device[m_device].Channel[m_channel].defDeviceParms.Colour.GreenGain;
      case VW_COL_BAL_BLUE_CONTRAST:
         return m_Device[m_device].Channel[m_channel].defDeviceParms.Colour.BlueGain;
      default:
         return -1;
   }
}

float VWInputProperties::getSourceAR( ) const
{
   float ret = (float)((float)m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorAddrTime /
         (float)m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerAddrTime);
   return ret;
}

void VWInputProperties::getCropping(sVWAOI* pAOI) const
{
   pAOI->top = m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].top;
   pAOI->left = m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].left;
   pAOI->right = m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].right;
   pAOI->bottom = m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].bottom;
}

// Setter methods

// Input Properties Setter methods

void VWInputProperties::setVidTimingWidth(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setVidTimingWidth: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(RGB133_V4L2_CID_HOR_TIME) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorAddrTime != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_HOR_TIME;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setVidTimingWidth");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorAddrTime = value;
   }
}

void VWInputProperties::setVidTimingHeight(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setVidTimingHeight: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(RGB133_V4L2_CID_VER_TIME) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerAddrTime != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_VER_TIME;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setVidTimingHeight");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerAddrTime = value;
   }
}

void VWInputProperties::setBrightness(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setBrightness: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(V4L2_CID_BRIGHTNESS) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.Brightness != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = V4L2_CID_BRIGHTNESS;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setBrightness");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.Brightness = value;
   }
}

void VWInputProperties::setContrast(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setContrast: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(V4L2_CID_CONTRAST) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.Contrast != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = V4L2_CID_CONTRAST;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setContrast");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.Contrast = value;
   }
}

void VWInputProperties::setSaturation(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setSaturation: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(V4L2_CID_SATURATION) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.Saturation != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = V4L2_CID_SATURATION;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setSaturation");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.Saturation = value;
   }
}

void VWInputProperties::setHue(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setHue: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(V4L2_CID_HUE) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.Hue != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = V4L2_CID_HUE;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setHue");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.Hue = value;
   }
}

void VWInputProperties::setHorPos(unsigned long value, bool force)
{
   unsigned long current_value =
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorBackPorch +
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorSyncTime;
   long diff = 0;

   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setHorPos: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(RGB133_V4L2_CID_HOR_POS) &&
      (current_value != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_HOR_POS;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setHorPos");
      else
      {
         diff = value - current_value;
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorBackPorch += diff;
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorFrontPorch -= diff;
      }
   }
}

void VWInputProperties::setHorSize(unsigned long value, bool force)
{
   unsigned long current_value =
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.PixelClock;
   long diff = 0;

   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setHorSize: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(RGB133_V4L2_CID_HOR_SIZE) &&
      (current_value != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_HOR_SIZE;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setHorSize");
      else
      {
         diff = value - current_value;
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.HorFrontPorch += diff;
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.PixelClock = value;
      }
   }
}

void VWInputProperties::setPhase(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setPhase: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(RGB133_V4L2_CID_PHASE) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.Phase != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_PHASE;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setPhase");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.Phase = value;
   }
}

void VWInputProperties::setVertPos(unsigned long value, bool force)
{
   unsigned long current_value =
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerBackPorch +
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerSyncTime;
   long diff = 0;

   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setVertPos: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(RGB133_V4L2_CID_VERT_POS) &&
      (current_value != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = RGB133_V4L2_CID_VERT_POS;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setVertPos");
      else
      {
         diff = value - current_value;
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerBackPorch += diff;
         m_Device[m_device].Channel[m_channel].curDeviceParms.VideoTimings.VerFrontPorch -= diff;
      }
   }
}

void VWInputProperties::setBlackLevel(unsigned long value, bool force)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setBlackLevel: invalid video device handle: " << m_video_handle;
      return;
   }

   if(isCtrlEnabled(V4L2_CID_BLACK_LEVEL) &&
      (m_Device[m_device].Channel[m_channel].curDeviceParms.Blacklevel != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = V4L2_CID_BLACK_LEVEL;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0)
         perror("VWInputProperties::setBlackLevel");
      else
         m_Device[m_device].Channel[m_channel].curDeviceParms.Blacklevel = value;
   }
}

void VWInputProperties::setColBalBrightness(unsigned long value, eVWColBalType type, bool force)
{
   unsigned long* current;
   unsigned int ctrl_id;

   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setColBalBrightness: invalid video device handle: " << m_video_handle;
      return;
   }

   switch(type)
   {
      case VW_COL_BAL_RED_BRIGHTNESS:
         current = &m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.RedOffset;
         ctrl_id = RGB133_V4L2_CID_R_COL_BRIGHTNESS;
         break;
      case VW_COL_BAL_GREEN_BRIGHTNESS:
         current = &m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.GreenOffset;
         ctrl_id = RGB133_V4L2_CID_G_COL_BRIGHTNESS;
         break;
      case VW_COL_BAL_BLUE_BRIGHTNESS:
         current = &m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.BlueOffset;
         ctrl_id = RGB133_V4L2_CID_B_COL_BRIGHTNESS;
         break;
      default:
         return;
   }

   if(isCtrlEnabled(ctrl_id) &&
      (*current != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = ctrl_id;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0) {
         perror("VWInputProperties::setColBalBrightness");
         printf("VWInputProperties::setColBalBrightness: Failed to set %lu for ctrl id %u\n", value, ctrl_id);
      }

      else
         *current = value;
   }
}

void VWInputProperties::setColBalContrast(unsigned long value, eVWColBalType type, bool force)
{
   unsigned long* current;
   unsigned int ctrl_id;

   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::setColBalContrast: invalid video device handle: " << m_video_handle;
      return;
   }

   switch(type)
   {
      case VW_COL_BAL_RED_CONTRAST:
         current = &m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.RedGain;
         ctrl_id = RGB133_V4L2_CID_R_COL_CONTRAST;
         break;
      case VW_COL_BAL_GREEN_CONTRAST:
         current = &m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.GreenGain;
         ctrl_id = RGB133_V4L2_CID_G_COL_CONTRAST;
         break;
      case VW_COL_BAL_BLUE_CONTRAST:
         current = &m_Device[m_device].Channel[m_channel].curDeviceParms.Colour.BlueGain;
         ctrl_id = RGB133_V4L2_CID_B_COL_CONTRAST;
         break;
      default:
         return;
   }

   if(isCtrlEnabled(ctrl_id) &&
      (*current != value || force))
   {
      struct v4l2_control control;
      memset(&control, 0, sizeof (control));
      control.id = ctrl_id;
      control.value = value;
      if (ioctl(m_video_handle, VIDIOC_S_CTRL, &control) < 0) {
         perror("VWInputProperties::setColBalContrast");
         printf("VWInputProperties::setColBalContrast: Failed to set %lu for ctrl id %u\n", value, ctrl_id);         
      }
      else
         *current = value;
   }
}

void VWInputProperties::setCropping(sVWAOI AOI, eVWCroppingType type)
{
   if(type == VW_CROPPING_CURRENT)
   {
      if(m_CroppingType[m_device][m_channel][m_client] == VW_CROPPING_OFF ||
         m_CroppingType[m_device][m_channel][m_client] == VW_CROPPING_UNSET)
      {
         m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].top = AOI.top;
         m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].left = AOI.left;
         m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].right = AOI.right;
         m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].bottom = AOI.bottom;
         m_CroppingType[m_device][m_channel][m_client] = VW_CROPPING_OFF;
      }
   }
   else
   {
      m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].top = AOI.top;
      m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].left = AOI.left;
      m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].right = AOI.right;
      m_Device[m_device].Channel[m_channel].curDeviceParms.aoi[m_client].bottom = AOI.bottom;
      m_CroppingType[m_device][m_channel][m_client] = type;
   }
}

void VWInputProperties::setModified(unsigned int modified)
{
   if(modified)
   {
      m_modified |= modified;
   }
   else
   {
      m_modified = 0;
   }
}

void VWInputProperties::clearModified(unsigned int modified)
{
   if(modified)
   {
      m_modified ^= modified;
   }
}

// Overloaded operators
VWInputProperties& VWInputProperties::operator=(VWInputProperties& properties)
{
   sVWAOI aoi;

   memcpy(&m_SystemInfo, properties.getSystemInfo(), sizeof(sSystemInfo));
   memcpy(&m_Device, properties.getDeviceData(), (MAX_VIDEO_DEVICES_SYSTEM * sizeof(sDeviceInfo)));

   this->setCurrentDevice(properties.getCurrentDevice());
   this->setCurrentInput(properties.getCurrentInput());
   this->setCurrentClient(properties.getCurrentClient());
   this->setCurrentIndex(properties.getCurrentIndex());

   this->setControlDeviceHandle(properties.getHandle());
   this->setVideoDeviceHandle(properties.getVideoDeviceHandle());

   this->setVidTimingWidth(properties.getVidTimingWidth(), true);
   this->setVidTimingHeight(properties.getVidTimingHeight(), true);

   this->setBrightness(properties.getColAdjBrightness(), true);
   this->setContrast(properties.getColAdjContrast(), true);
   this->setSaturation(properties.getColAdjSaturation(), true);
   this->setHue(properties.getColAdjHue(), true);

   this->setHorPos(properties.getVidAdjHorPos(), true);
   this->setHorSize(properties.getVidAdjHorSize(), true);
   this->setPhase(properties.getVidAdjPhase(), true);
   this->setVertPos(properties.getVidAdjVertPos(), true);
   this->setBlackLevel(properties.getVidAdjBlackLevel(), true);

   properties.getCropping(&aoi);
   this->setCropping(aoi, properties.getCroppingType());

   return *this;
}

// Private Control device info methods
int VWInputProperties::openCtrlDevice()
{
   int fd = -1;
   QString device = "/dev/video" + QString::number( RGB133_CONTROL_DEVICE_NUM );

   fd = ::open(device.toLocal8Bit().constData(), O_RDWR | O_NONBLOCK);
   if (fd <= 0)
   {
      printf("VWData::openCtrlDevice: failed to open %s\n", device.toLocal8Bit().constData());
      fd = -1;
   }

   return fd;
}

void VWInputProperties::closeCtrlDevice()
{
   closeCtrlDevice(m_ctrl_handle);
}

void VWInputProperties::closeCtrlDevice(int fd)
{
   if(fd >= 0)
   {
      ::close(fd);
   }
}

static const char *video_dev_prefixes[] = {
   "video",
   NULL
};

static bool is_v4l2_video_dev(const char *name)
{
   for (unsigned i = 0; video_dev_prefixes[i]; i++) {
      unsigned l = strlen(video_dev_prefixes[i]);

      if (!memcmp(name, video_dev_prefixes[i], l)) {
         if (isdigit(name[l]))
            return true;
      }
   }
   return false;
}

static int node_to_num(const char *s)
{
   int n = 0;

   s = strrchr(s, '/') + 1;

   for (unsigned i = 0; video_dev_prefixes[i]; i++) {
      unsigned l = strlen(video_dev_prefixes[i]);

      if (!memcmp(s, video_dev_prefixes[i], l)) {
         n = i << 8;
         n += atol(s + l);
         return n;
      }
   }
   return 0;
}

static bool sort_by_name(const std::string &s1, const std::string &s2)
{
   int n1 = node_to_num(s1.c_str());
   int n2 = node_to_num(s2.c_str());

   return n1 < n2;
}

int VWInputProperties::enumerateVideoDevices()
{
   DIR *pdir;
   struct dirent *pentry;
   device_vec files;
   struct v4l2_capability caps;
   int i = 0;

   memset(&m_SystemInfo, 0, sizeof(sSystemInfo));

   pdir = opendir("/dev");
   if (pdir == NULL) {
      perror ("VWInputProperties::enumerateVideoDevices: Couldn't open /dev directory");
      return -1;
   }

   while ((pentry = readdir(pdir)))
      if (is_v4l2_video_dev(pentry->d_name))
         files.push_back(std::string("/dev/") + pentry->d_name);
   closedir(pdir);

   std::sort(files.begin(), files.end(), sort_by_name);

   for (device_vec::iterator nd = files.begin(); nd != files.end(); ++nd)
   {
      struct v4l2_input input;
      int fd, err;

      if ((fd = open(nd->c_str(), O_RDWR)) < 0)
         continue;
      if ((err = ioctl(fd, VIDIOC_QUERYCAP, &caps)) < 0) {
         close(fd);
         continue;
      }
      if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
          strncmp((const char*)caps.driver, DRIVER_TAG, strlen((const char*)caps.driver))) {
         close(fd);
         continue;
      }

      /* Find how many channels on this device */
      memset(&input, 0, sizeof(input));
      while (ioctl(fd, VIDIOC_ENUMINPUT, &input) >= 0)
      {
         m_Device[i].num_channels++;
         input.index = m_Device[i].num_channels;
      }
      close(fd);

      strncpy(m_Device[i].node, nd->c_str(), sizeof(m_Device[i].node));
      m_SystemInfo.num_devices++;
      m_SystemInfo.num_inputs += m_Device[i].num_channels;
      if (i == 0)
         m_SystemInfo.version = caps.version;

      i++;
   }

   if (m_SystemInfo.num_devices <= 0) {
      qDebug() << "VWInputProperties::enumerateVideoDevices: No video devices found in the system";
      return -1;
   }

   return 0;
}

bool VWInputProperties::isCtrlEnabled(unsigned int id)
{
   struct v4l2_queryctrl query_ctrl;
   int rc;

   memset(&query_ctrl, 0, sizeof(query_ctrl));
   query_ctrl.id = id;
   rc = ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl);
   if (rc == 0 && !(query_ctrl.flags & V4L2_CTRL_FLAG_DISABLED))
      return true;

   return false;
}

// Input Properties Update methods

/* Video Timings */

int VWInputProperties::updateVideoTimings(int device, int channel, eParmsType type)
{
   srgb133VideoTimings video_timings;
   unsigned long bytes;
   
   if (type != RGB133_PARMS_CURRENT &&
       type != RGB133_PARMS_DETECTED) {
      qDebug() << "VWInputProperties::updateVideoTimings: invalid type: " << type;
   }

   bytes = sizeof(srgb133VideoTimings);
   memset(&video_timings, 0, bytes);
   video_timings.type = type;
   video_timings.size = bytes;
   if (ioctl(m_video_handle, RGB133_VIDIOC_G_VIDEO_TIMINGS, &video_timings) < 0) {
      perror("VWInputProperties::updateVideoTimings");
      return -1;
   }
   if (video_timings.size != bytes) {
      qDebug() << 
         "VWInputProperties::updateVideoTimings: different size was filled than requested:" << 
         video_timings.size <<
         "vs." << bytes;
   }

   if (type == RGB133_PARMS_CURRENT)
      memcpy(&m_Device[device].Channel[channel].curDeviceParms.VideoTimings,
             &video_timings.VideoTimings, 
             std::min(bytes, video_timings.size));
   else if (type == RGB133_PARMS_DETECTED)
      memcpy(&m_Device[device].Channel[channel].detDeviceParms.VideoTimings,
             &video_timings.VideoTimings,
             std::min(bytes, video_timings.size));  
   return 0;
}

int VWInputProperties::updateVidTimingWidthMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_HOR_TIME;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateVidTimingWidthMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.VideoTimings.HorAddrTime = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.VideoTimings.HorAddrTime = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.VideoTimings.HorAddrTime = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateVidTimingHeightMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_VER_TIME;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateVidTimingHeightMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.VideoTimings.VerAddrTime = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.VideoTimings.VerAddrTime = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.VideoTimings.VerAddrTime = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateHorSizeMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_HOR_SIZE;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateHorSizeMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.VideoTimings.PixelClock = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.VideoTimings.PixelClock = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.VideoTimings.PixelClock = query_ctrl.default_value;

   return 0;
}

/* Video Characteristics (ALL) */

int VWInputProperties::updateSignalType(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_SIGNAL_TYPE))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_SIGNAL_TYPE;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateSignalType");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.type = (eVWSignalType)ctrl.value;

   return 0;
}

int VWInputProperties::updateBrightness(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(V4L2_CID_BRIGHTNESS))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = V4L2_CID_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateBrightness");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Brightness = ctrl.value;

   return 0;
}

int VWInputProperties::updateBrightnessMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = V4L2_CID_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateBrightnessMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Brightness = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Brightness = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Brightness = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateContrast(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(V4L2_CID_CONTRAST))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = V4L2_CID_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateContrast");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Contrast = ctrl.value;

   return 0;
}

int VWInputProperties::updateContrastMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = V4L2_CID_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateContrastMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Contrast = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Contrast = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Contrast = query_ctrl.default_value;

   return 0;
}

/* Video Characteristics (ANALOG) */

int VWInputProperties::updateBlackLevel(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(V4L2_CID_BLACK_LEVEL))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = V4L2_CID_BLACK_LEVEL;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateBlackLevel");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Blacklevel = ctrl.value;

   return 0;
}

int VWInputProperties::updateBlackLevelMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = V4L2_CID_BLACK_LEVEL;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateBlackLevelMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Blacklevel = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Blacklevel = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Blacklevel = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateSaturation(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(V4L2_CID_SATURATION))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = V4L2_CID_SATURATION;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateSaturation");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Saturation = ctrl.value;

   return 0;
}

int VWInputProperties::updateSaturationMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = V4L2_CID_SATURATION;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateSaturationMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Saturation = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Saturation = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Saturation = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updatePhase(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_PHASE))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_PHASE;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updatePhase");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Phase = ctrl.value;

   return 0;
}

int VWInputProperties::updatePhaseMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_PHASE;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updatePhaseMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Phase = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Phase = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Phase = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateHue(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(V4L2_CID_HUE))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = V4L2_CID_HUE;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateHue");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Hue = ctrl.value;

   return 0;
}

int VWInputProperties::updateHueMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = V4L2_CID_HUE;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateHueMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Hue = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Hue = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Hue = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateVideoStandard(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_VIDEO_STANDARD))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_VIDEO_STANDARD;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateVideoStandard");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.VideoStandard = (eVWVidStd)ctrl.value;

   return 0;
}

/* Colour Balance */

int VWInputProperties::updateRedBrightness(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_R_COL_BRIGHTNESS))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_R_COL_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateRedBrightness");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Colour.RedOffset = ctrl.value;

   return 0;
}

int VWInputProperties::updateRedBrightnessMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_R_COL_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateRedBrightnessMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Colour.RedOffset = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Colour.RedOffset = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Colour.RedOffset = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateRedContrast(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_R_COL_CONTRAST))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_R_COL_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateRedContrast");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Colour.RedGain = ctrl.value;

   return 0;
}

int VWInputProperties::updateRedContrastMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_R_COL_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateRedContrastMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Colour.RedGain = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Colour.RedGain = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Colour.RedGain = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateGreenBrightness(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_G_COL_BRIGHTNESS))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_G_COL_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateGreenBrightness");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Colour.GreenOffset = ctrl.value;

   return 0;
}

int VWInputProperties::updateGreenBrightnessMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_G_COL_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateGreenBrightnessMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Colour.GreenOffset = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Colour.GreenOffset = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Colour.GreenOffset = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateGreenContrast(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_G_COL_CONTRAST))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_G_COL_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateGreenContrast");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Colour.GreenGain = ctrl.value;

   return 0;
}

int VWInputProperties::updateGreenContrastMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_G_COL_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateGreenContrastMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Colour.GreenGain = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Colour.GreenGain = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Colour.GreenGain = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateBlueBrightness(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_B_COL_BRIGHTNESS))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_B_COL_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateBlueBrightness");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Colour.BlueOffset = ctrl.value;

   return 0;
}

int VWInputProperties::updateBlueBrightnessMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_B_COL_BRIGHTNESS;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateBlueBrightnessMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Colour.BlueOffset = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Colour.BlueOffset = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Colour.BlueOffset = query_ctrl.default_value;

   return 0;
}

int VWInputProperties::updateBlueContrast(int device, int channel)
{
   struct v4l2_control ctrl;

   if (!isCtrlEnabled(RGB133_V4L2_CID_B_COL_CONTRAST))
      return -2;

   memset(&ctrl, 0, (sizeof(struct v4l2_control)));
   ctrl.id = RGB133_V4L2_CID_B_COL_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_G_CTRL, &ctrl) < 0) {
      perror("VWInputProperties::updateBlueContrast");
      return -1;
   }

   m_Device[device].Channel[channel].curDeviceParms.Colour.BlueGain = ctrl.value;

   return 0;
}

int VWInputProperties::updateBlueContrastMinMaxDef(int device, int channel)
{
   struct v4l2_queryctrl query_ctrl;

   memset(&query_ctrl, 0, sizeof(struct v4l2_queryctrl));
   query_ctrl.id = RGB133_V4L2_CID_B_COL_CONTRAST;
   if (ioctl(m_video_handle, VIDIOC_QUERYCTRL, &query_ctrl) < 0) {
      perror("VWInputProperties::updateBlueContrastMinMaxDef");
      return -1;
   }

   m_Device[device].Channel[channel].minDeviceParms.Colour.BlueGain = query_ctrl.minimum;
   m_Device[device].Channel[channel].maxDeviceParms.Colour.BlueGain = query_ctrl.maximum;
   m_Device[device].Channel[channel].defDeviceParms.Colour.BlueGain = query_ctrl.default_value;

   return 0;
}

/* Cropping */

int VWInputProperties::updateCropping(int device, int channel, int client)
{
   struct v4l2_rect rect;

#ifdef VW_CONFIG_HAVE_SELECTION_API
   struct v4l2_selection selection;

   memset(&selection, 0, sizeof(selection));
   selection.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   selection.target = V4L2_SEL_TGT_CROP;
   ioctl(m_video_handle, VIDIOC_G_SELECTION, &selection);
   rect = selection.r;
#else /* !VW_CONFIG_HAVE_SELECTION_API */
   struct v4l2_crop crop;

   memset(&crop, 0, sizeof(crop));
   crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   ioctl(m_video_handle, VIDIOC_G_CROP, &crop);
   rect = crop.c;
#endif /* VW_CONFIG_HAVE_SELECTION_API */

   m_Device[device].Channel[channel].curDeviceParms.aoi[client].top = rect.top;
   m_Device[device].Channel[channel].curDeviceParms.aoi[client].left = rect.left;
   m_Device[device].Channel[channel].curDeviceParms.aoi[client].right = rect.width;
   m_Device[device].Channel[channel].curDeviceParms.aoi[client].bottom = rect.height;

   return 0;
}

/* Current Device Properties */
int VWInputProperties::getDevicePropertiesCurrent(int device, int channel, int client)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::getDevicePropertiesCurrent: invalid video dev handle: " << m_video_handle;
      return -1;
   }

   /* Video Timings */
   updateVideoTimings(device, channel, RGB133_PARMS_CURRENT);

   /* Video Characteristics (ALL) */
   updateSignalType(device, channel);
   updateBrightness(device, channel);
   updateContrast(device, channel);

   /* Video Characteristics (ANALOG) */
   updateBlackLevel(device, channel);
   updateSaturation(device, channel);
   updatePhase(device, channel);
   updateHue(device, channel);
   updateVideoStandard(device, channel);

   /* Colour Balance */
   updateRedBrightness(device, channel);
   updateRedContrast(device, channel);
   updateGreenBrightness(device, channel);
   updateGreenContrast(device, channel);
   updateBlueBrightness(device, channel);
   updateBlueContrast(device, channel);
   
   /* Cropping */
   updateCropping(device, channel, client);

   return 0;
}

int VWInputProperties::getDevicePropertiesDetected(int device, int channel)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::getDevicePropertiesDetected: invalid video dev handle: " << m_video_handle;
      return -1;
   }

   /* Video Timings */
   updateVideoTimings(device, channel, RGB133_PARMS_DETECTED);

   return 0;
}

int VWInputProperties::getDevicePropertiesMinMaxDef(int device, int channel)
{
   if (m_video_handle < 0)
   {
      qDebug() << "VWInputProperties::getDevicePropertiesMinMaxDef: invalid video device handle: " << m_video_handle;
      return -1;
   }

   updateVidTimingWidthMinMaxDef(device, channel);
   updateVidTimingHeightMinMaxDef(device, channel);
   updateHorSizeMinMaxDef(device, channel);

   updateBrightnessMinMaxDef(device, channel);
   updateContrastMinMaxDef(device, channel);

   updateBlackLevelMinMaxDef(device, channel);
   updateSaturationMinMaxDef(device, channel);
   updatePhaseMinMaxDef(device, channel);
   updateHueMinMaxDef(device, channel);

   updateRedBrightnessMinMaxDef(device, channel);
   updateRedContrastMinMaxDef(device, channel);
   updateGreenBrightnessMinMaxDef(device, channel);
   updateGreenContrastMinMaxDef(device, channel);
   updateBlueBrightnessMinMaxDef(device, channel);
   updateBlueContrastMinMaxDef(device, channel);

   return 0;
}

int VWInputProperties::getDevicePropertiesAll(int device, int channel, int client)
{
   int err = -1;

   err = getDevicePropertiesMinMaxDef(device, channel);
   if (err < 0)
   {
      qDebug() <<
         "VWInputProperties::getDevicePropertiesAll: Failed to get min, max, def device properties:" << err;
      return err;
   }
   err = getDevicePropertiesCurrent(device, channel, client);
   if (err < 0)
   {
      qDebug() <<
         "VWInputProperties::getDevicePropertiesAll: Failed to get current device properties:" << err;
      return err;
   }
   err = getDevicePropertiesDetected(device, channel);
   if (err < 0)
   {
      qDebug() <<
         "VWInputProperties::getDevicePropertiesAll: Failed to get detected device properties:" << err;
      return err;
   }

   return 0;
}

int VWInputProperties::readCtrlDeviceEvent(int *pDevice, int *pChannel)
{
   sVWSignalEvent event;
   int read = 0;

   event.magic = VW_MAGIC_SIGNAL_EVENT;
   read = ::read(m_ctrl_handle, &event, sizeof(sVWSignalEvent));
   if (read <= 0)
   {
      printf("VWInputProperties::readCtrlDeviceEvent: Failed to read %zu bytes of signal event data info into buffer(%p)\n",
            sizeof(sVWSignalEvent), &event);
      return -1;
   }

   *pDevice = event.device;
   *pChannel = event.channel;

   return 0;
}

int VWInputProperties::setRequestedIndexDeviceInput(int index)
{
   int idx = index;
   int i = 0;

   if (idx < 0 || idx >= m_SystemInfo.num_inputs) {
      qDebug() <<
         "VWInputProperties::setRequestedIndexDeviceInput: Incorrect input index:" << idx;
      return -1;
   }

   for (i = 0; i < m_SystemInfo.num_devices; i++) {
      if (idx < m_Device[i].num_channels)
         break; /* Found the matching device! */
      
      idx -= m_Device[i].num_channels;
   }

   m_device = i;
   m_channel = idx;

   return 0;
}
