
//          Copyright (c) Datapath Limited 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef VWINPUTPROPERTIES_H_
#define VWINPUTPROPERTIES_H_

#include <QtWidgets>

#include "rgb133control.h"
#include "rgb133v4l2.h"

#include "VWProperties.h"
#include "VWColBal.h"

#define MAX_VIDEO_DEVICES_SYSTEM   RGB133_MAX_VIDEO_DEVICES
#define MAX_CLIENTS_PER_CHANNEL    RGB133_MAX_CAP_PER_CHANNEL
#define MAX_CHANNELS_PER_DEVICE    RGB133_MAX_CHANNEL_PER_CARD

enum eVWCroppingType {
   VW_CROPPING_UNSET = 0,
   VW_CROPPING_OFF,
   VW_CROPPING_ON,
   VW_CROPPING_OVERSCAN,
   VW_CROPPING_CURRENT,
};

typedef struct _sDeviceInfo {
   int           num_channels;
   char          node[MAX_NODE_LEN];
   sVWInput      Channel[MAX_CHANNELS_PER_DEVICE];
} sDeviceInfo, *psDeviceInfo;

typedef struct _sSystemInfo {
   int           num_devices;
   int           num_inputs;
   unsigned int  version;
} sSystemInfo;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class VWInputProperties : public QThread
{
public:
   VWInputProperties( );
   ~VWInputProperties( );

   void initCtrl( );
   void uninitCtrl( );

   int enumerateVideoDevices();

   int select( );
   int readCtrlDeviceEvent(int *pDevice, int *pChannel);

   // Update value methods
   void getLatestValues();
   void getLatestValues(int device, int channel, int client);

   int  getVideoDeviceHandle( ) { return m_video_handle; }

   // Device getter methods
   QString        getVersion( );
   int            getNumDevices() const { return m_SystemInfo.num_devices; };
   int            getNumInputs() const { return m_SystemInfo.num_inputs; };
   int            getCurrentDevice() const { return m_device; };
   int            getCurrentInput() const { return m_channel; };
   QString        getCurrentIndexNode();
   int            getCurrentIndex() const { return m_index; };
   int            getCurrentClient() const { return m_client; };
   int            setRequestedIndexDeviceInput(int index);
   sSystemInfo*   getSystemInfo( ) { return &m_SystemInfo; };

   sDeviceInfo*   getDeviceData( ) { return &m_Device[0]; };
   int            getHandle( ) { return m_ctrl_handle; };

   // Input Properties Getter methods
   enum _eVWSignalType getSignalType() const;
   enum _eVWVidStd getVideoStd() const;

   int getVidTimingWidth() const;
   int getVidTimingWidthMin() const;
   int getVidTimingWidthMax() const;
   int getVidTimingWidthDef() const;
   int getVidTimingHeight() const;
   int getVidTimingHeightMin() const;
   int getVidTimingHeightMax() const;
   int getVidTimingHeightDef() const;
   double getVidTimingRefresh() const;

   int getColAdjBrightness() const;
   int getColAdjBrightnessMin() const;
   int getColAdjBrightnessMax() const;
   int getColAdjBrightnessDef() const;
   int getColAdjContrast() const;
   int getColAdjContrastMin() const;
   int getColAdjContrastMax() const;
   int getColAdjContrastDef() const;
   int getColAdjSaturation() const;
   int getColAdjSaturationMin() const;
   int getColAdjSaturationMax() const;
   int getColAdjSaturationDef() const;
   int getColAdjHue() const;
   int getColAdjHueMin() const;
   int getColAdjHueMax() const;
   int getColAdjHueDef() const;

   int getVidAdjHorPos() const;
   int getVidAdjHorPosMin() const;
   int getVidAdjHorPosMax() const;
   int getVidAdjHorPosDef() const;
   int getVidAdjHorSize() const;
   int getVidAdjHorSizeMin() const;
   int getVidAdjHorSizeMax() const;
   int getVidAdjHorSizeDef() const;
   int getVidAdjPhase() const;
   int getVidAdjPhaseMin() const;
   int getVidAdjPhaseMax() const;
   int getVidAdjPhaseDef() const;
   int getVidAdjVertPos() const;
   int getVidAdjVertPosMin() const;
   int getVidAdjVertPosMax() const;
   int getVidAdjVertPosDef() const;
   int getVidAdjBlackLevel() const;
   int getVidAdjBlackLevelMin() const;
   int getVidAdjBlackLevelMax() const;
   int getVidAdjBlackLevelDef() const;

   int getColBalBrightness(eVWColBalType type) const;
   int getColBalBrightnessMin(eVWColBalType type) const;
   int getColBalBrightnessMax(eVWColBalType type) const;
   int getColBalBrightnessDef(eVWColBalType type) const;
   int getColBalContrast(eVWColBalType type) const;
   int getColBalContrastMin(eVWColBalType type) const;
   int getColBalContrastMax(eVWColBalType type) const;
   int getColBalContrastDef(eVWColBalType type) const;

   float getSourceAR( ) const;

   void getCropping(sVWAOI* pAOI) const;
   eVWCroppingType getCroppingType() const \
         { return m_CroppingType[m_device][m_channel][m_client]; };

   // Input Properties Update methods
   int updateVideoTimings(int device, int channel, eParmsType type);
   int updateVidTimingWidthMinMaxDef(int device, int channel);
   int updateVidTimingHeightMinMaxDef(int device, int channel);
   int updateHorSizeMinMaxDef(int device, int channel);

   int updateSignalType(int device, int channel);
   int updateBrightness(int device, int channel);
   int updateBrightnessMinMaxDef(int device, int channel);
   int updateContrast(int device, int channel);
   int updateContrastMinMaxDef(int device, int channel);

   int updateBlackLevel(int device, int channel);
   int updateBlackLevelMinMaxDef(int device, int channel);
   int updateSaturation(int device, int channel);
   int updateSaturationMinMaxDef(int device, int channel);
   int updatePhase(int device, int channel);
   int updatePhaseMinMaxDef(int device, int channel);
   int updateHue(int device, int channel);
   int updateHueMinMaxDef(int device, int channel);
   int updateVideoStandard(int device, int channel);

   int updateRedBrightness(int device, int channel);
   int updateRedBrightnessMinMaxDef(int device, int channel);
   int updateRedContrast(int device, int channel);
   int updateRedContrastMinMaxDef(int device, int channel);
   int updateGreenBrightness(int device, int channel);
   int updateGreenBrightnessMinMaxDef(int device, int channel);
   int updateGreenContrast(int device, int channel);
   int updateGreenContrastMinMaxDef(int device, int channel);
   int updateBlueBrightness(int device, int channel);
   int updateBlueBrightnessMinMaxDef(int device, int channel);
   int updateBlueContrast(int device, int channel);
   int updateBlueContrastMinMaxDef(int device, int channel);

   int updateCropping(int device, int channel, int client);

   // Setter methods
   void setCurrentDevice(int device) { m_device = device; };
   void setCurrentInput(int input) { m_channel = input; };
   void setCurrentIndex(int index) { m_index = index; };
   void setCurrentClient(int id) { m_client = id; };
   void setControlDeviceHandle(int handle) { m_ctrl_handle = handle; };
   void setVideoDeviceHandle(int h) { m_video_handle = h; }

   // Input Properties Setter methods
   void setVidTimingWidth(unsigned long value, bool force);
   void setVidTimingHeight(unsigned long value, bool force);

   void setBrightness(unsigned long value, bool force);
   void setContrast(unsigned long value, bool force);
   void setSaturation(unsigned long value, bool force);
   void setHue(unsigned long value, bool force);

   void setHorPos(unsigned long value, bool force);
   void setHorSize(unsigned long value, bool force);
   void setPhase(unsigned long value, bool force);
   void setVertPos(unsigned long value, bool force);
   void setBlackLevel(unsigned long value, bool force);

   void setColBalBrightness(unsigned long value, eVWColBalType type, bool force);
   void setColBalContrast(unsigned long value, eVWColBalType type, bool force);

   void setCropping(sVWAOI AOI, eVWCroppingType type);

   // Modification operators
   unsigned int getModified( ) { return m_modified; };
   void setModified(unsigned int modified);
   void clearModified(unsigned int modified);

   // Overloaded operators
   VWInputProperties& operator=(VWInputProperties& properties);

private:
   // Control device data and methods
   int openCtrlDevice();
   void closeCtrlDevice();
   void closeCtrlDevice(int fd);

   // Helper methods
   bool isCtrlEnabled(unsigned int id);

   // Device Properties Getter methods
   int getDevicePropertiesCurrent(int device, int channel, int client);
   int getDevicePropertiesDetected(int device, int channel);
   int getDevicePropertiesMinMaxDef(int device, int channel);
   int getDevicePropertiesAll(int device, int channel, int client);

   int             m_ctrl_handle;    // Control device handle
   int             m_video_handle;   // Current video device handle
   int             m_device;         // Current device number
   int             m_channel;        // Current channel number
   int             m_index;          // Current input index (drop-down menu)
   int             m_client;         // Current client

   sSystemInfo     m_SystemInfo;
   sDeviceInfo     m_Device[MAX_VIDEO_DEVICES_SYSTEM];
   eVWCroppingType m_CroppingType[MAX_VIDEO_DEVICES_SYSTEM][MAX_CHANNELS_PER_DEVICE][MAX_CLIENTS_PER_CHANNEL];

   unsigned int    m_modified;
};
#endif /* VWINPUTPROPERTIES_H_ */
